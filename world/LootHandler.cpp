#include "LootHandler.h"
#include "LootController.h"
#include "LogSystem.h"
#include "Character.h"
#include "Inventory.h"
#include "Item.h"
#include "NPC.h"
#include "Zone.h"
#include "ZoneConnection.h"

LootHandler::LootHandler(Zone* pZone, ILogFactory* pLogFactory) : mZone(pZone) {
	mLog = pLogFactory->make();
	mLog->setContext("[LootHandler]");
}

LootHandler::~LootHandler() {
	delete mLog;
	mLog = nullptr;
}

void LootHandler::onRequest(Character* pCharacter, const u32 pSpawnID) {
	using namespace Payload::Zone;
	if (!pCharacter) return;
	if (pCharacter->isLooting()) return;

	auto connection = pCharacter->getConnection();

	// Check: Actor exists.
	auto actor = mZone->getActor(pSpawnID);
	if (!actor) {
		pCharacter->notify("Corpse could not be found.");
		connection->sendLootResponse(LootResponse::DENY);
		return;
	}

	// Check: Actor is a corpse.
	if (!actor->isCorpse()) {
		pCharacter->notify("You can not loot that.");
		connection->sendLootResponse(LootResponse::DENY);
		return;
	}

	// Handle: Looting an NPC corpse.
	if (actor->isNPCCorpse()) {
		auto lootController = actor->getLootController();
		auto npcCorpse = Actor::cast<NPC*>(actor);
		// Check: Is pCharacter close enough to loot.
		if (pCharacter->squareDistanceTo(npcCorpse) > 625) { // TODO: Magic.
			connection->sendLootResponse(LootResponse::TOO_FAR);
			return;
		}
		// Check: Is corpse currently closed?
		if (lootController->isOpen() == false) {
			// Check: Is it time to open the corpse?
			// TODO: This is probably the most efficient place to check this however it is odd!
			if (npcCorpse->getOpenTimer().check()) {
				// Open corpse.
				lootController->setOpen(true);
			}
		}
		// Check: Is Character allowed to loot this corpse?
		if (lootController->canLoot(pCharacter) == false) {
			connection->sendLootResponse(LootResponse::DENY);
			return;
		}

		// Check: Is someone already looting this corpse?
		if (lootController->hasLooter()) {
			connection->sendLootResponse(LootResponse::ALREADY);
			return;
		}

		// Associate Character and Corpse.
		pCharacter->setLootingCorpse(npcCorpse);
		lootController->setLooter(pCharacter);

		i32 platinum = 0;
		i32 gold = 0;
		i32 silver = 0;
		i32 copper = 0;
		bool currencyLooted = false;

		// Check: Does the corpse have currency on it?
		if (npcCorpse->hasCurrency()) {
			currencyLooted = true;

			// Remove currency from corpse.
			npcCorpse->getCurrency(platinum, gold, silver, copper);
			npcCorpse->removeCurrency();

			// Add currency to looter.
			pCharacter->getInventory()->addCurrency(CurrencySlot::Personal, platinum, gold, silver, copper, InventoryReason::Loot);
		}

		pCharacter->getConnection()->sendLootResponse(LootResponse::LOOT, platinum, gold, silver, copper);

		if (currencyLooted) {
			// TODO: Currency save.
		}

		// Send items.
		npcCorpse->onLootBegin();
		int count = 0;
		for (auto i : npcCorpse->getLootItems()) {
			i->setSlot(23 + count);
			u32 payloadSize = 0;
			const unsigned char* data = i->copyData(payloadSize, Payload::ItemPacketLoot, true);

			auto packet = new EQApplicationPacket(OP_ItemPacket, data, payloadSize);
			pCharacter->getConnection()->sendPacket(packet);
			delete packet;
			count++;
		}

		return;
	}

	// Handle: Looting a Character corpse.
	if (actor->isCharacterCorpse()) {
		// TODO:
	}
}

void LootHandler::onFinished(Character* pCharacter) {
	if (!pCharacter) return;
	if (!pCharacter->isLooting()) return;

	auto corpse = pCharacter->getLootingCorpse();
	auto controller = corpse->getLootController();
	
	// Disassociate Character and corpse.
	controller->clearLooter();
	pCharacter->setLootingCorpse(nullptr);

	pCharacter->getConnection()->sendLootComplete();

	// Finished looting an NPC corpse.
	if (corpse->isNPCCorpse()) {
		NPC* npcCorpse = Actor::cast<NPC*>(corpse);

		// Check: Empty corpse
		if (!npcCorpse->hasCurrency() && !npcCorpse->hasItems()) {
			npcCorpse->destroy();
			return;
		}
	}
	// Finished looting a Character corpse.
	else if (corpse->isCharacterCorpse()) {
		// TODO:
	}
}

void LootHandler::onLootItem(Character* pCharacter, const u32 pSlotID) {
	if (!pCharacter) return;
	if (!pCharacter->isLooting()) return;

	auto corpse = pCharacter->getLootingCorpse();
	auto controller = corpse->getLootController();

	// TODO: Check out Slot IDs.

	// Handle: Looting from NPC corpse.
	if (corpse->isNPCCorpse()) {
		NPC* npcCorpse = Actor::cast<NPC*>(corpse);
		Item* item = npcCorpse->getLootItem(pSlotID - 23);

		// TODO: Cursor empty check.

		// Update Character Inventory.
		pCharacter->getInventory()->pushCursor(item);

		// Update NPC loot items.
		npcCorpse->removeLootItem(pSlotID - 23);

		pCharacter->getConnection()->sendItemTrade(item);
	}
	// Handle: Looting from Character corpse.
}
