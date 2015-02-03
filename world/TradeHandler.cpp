#include "TradeHandler.h"
#include "LogSystem.h"
#include "Character.h"
#include "Item.h"
#include "Inventory.h"
#include "NPC.h"
#include "Zone.h"
#include "ZoneConnection.h"

TradeHandler::TradeHandler(Zone* pZone, ILogFactory* pLogFactory) : mZone(pZone) {
	mLog = pLogFactory->make();
	mLog->setContext("[TradeHandler]");
}

TradeHandler::~TradeHandler() {
	delete mLog;
	mLog = nullptr;
}

void TradeHandler::onRequest(Character* pCharacter, const u32 pSpawnID) {
	if (!pCharacter) return;

	// Check: Character can trade.
	if (!pCharacter->canRequestTrade()) {
		mLog->error("Got trade request from " + pCharacter->getName() + " but they are not allowed to trade right now.");
		return;
	}

	// Find Actor.
	auto actor = mZone->getActor(pSpawnID);
	if (!actor) {
		return;
	}

	// Check: Self trade.
	if (pCharacter == actor) {
		mLog->error("Got self trade request from " + pCharacter->getName());
		return;
	}

	// TODO: Check distance for trading.

	// Requesting trade with an NPC.
	if (actor->isNPC()) {
		auto npc = Actor::cast<NPC*>(actor);
		// Check: Is this NPC accepting trading requests?
		if (npc->willTrade()) {
			// Notify: NPC will accept trading.
			pCharacter->getConnection()->sendTradeRequestAcknowledge(npc->getSpawnID());

			// Associate Character and NPC.
			npc->addTrader(pCharacter);
			pCharacter->setTradingWith(npc);
			return;
		}
		else {
			pCharacter->message(MessageType::White, "This NPC will not trade with you.");
			return;
		}
	}
	// Requesting trade with a Character.
	else if (actor->isCharacter()) {
		auto character = Actor::cast<Character*>(actor);

		// Prevent trade with camping Character.
		if (character->isCamping())
			return;

		// Prevent trade with LD Character.
		if (character->isLinkDead())
			return;

		// Check: Does Character have a pending trade request.
		if (character->hasTradeRequest()) {

			return;
		}

		character->getConnection()->sendTradeRequest(pCharacter->getSpawnID());

		// Record request.
		auto& request = character->getTradeRequest();
		request.mSpawnID = pCharacter->getSpawnID();
		request.mTimeRequested = Utility::Time::now();

		return;
	}
}

void TradeHandler::onResponse(Character* pCharacter, const u32 pSpawnID) {
	if (!pCharacter) return;

	// Check: Character has a pending trade request.
	if (!pCharacter->hasTradeRequest()) {
		mLog->error(pCharacter->getName() + " responded to trade request they do not have one pending.");
		return;
	}

	auto& request = pCharacter->getTradeRequest();

	// Check: Trade request spawn ID matches.
	if (request.mSpawnID != pSpawnID) {
		mLog->error(pCharacter->getName() + " responded to trade request with spawn ID: " + toString(pSpawnID) + " but the pending trade request has spawn ID: " + toString(request.mSpawnID));
		pCharacter->clearTradeRequest();
		return;
	}

	// Find Actor.
	auto actor = mZone->getActor(pSpawnID);
	if (!actor) {
		// NOTE: This is probably not an error. A Character could send a request then zone out.
		mLog->error(pCharacter->getName() + " responded to trade request with spawn ID: " + toString(pSpawnID) + " but the Actor could not be found.");
		pCharacter->clearTradeRequest();
		return;
	}

	// Check: Is visible to
	// Check: Distance is valid.

	auto character = Actor::cast<Character*>(actor);

	// Associate Characters as trading with each other.
	character->setTradingWith(pCharacter);
	pCharacter->setTradingWith(actor);

	character->getConnection()->sendTradeRequestAcknowledge(pCharacter->getSpawnID());

	pCharacter->clearTradeRequest();
}

void TradeHandler::onBusy(Character* pCharacter, const u32 pSpawnID) {
	if (!pCharacter) return;


}

void TradeHandler::onAccept(Character* pCharacter, const u32 pSpawnID) {
	using namespace Payload::Zone;
	if (!pCharacter) return;

	// Check: Character is trading.
	if (!pCharacter->isTrading()) {
		mLog->error(pCharacter->getName() + " accepted trade but they are not trading.");
		return;
	}

	// Check: Character has already accepted trade.
	if (pCharacter->isTradeAccepted()) {
		mLog->error(pCharacter->getName() + " accepted trade but they have already accepted.");
		return;
	}

	auto tradingWithActor = pCharacter->getTradingWith();

	// Check: Character is accepting trade with correct Actor.
	if (tradingWithActor->getSpawnID() != pSpawnID) {
		mLog->error(pCharacter->getName() + " accepted trade with incorrect Actor.");
		return;
	}

	// Handle: Accepted trade with another Character.
	if (pCharacter->isTradingWithCharacter()) {
		pCharacter->setTradeAccepted(true);

		auto tradingWith = Actor::cast<Character*>(tradingWithActor);

		// Update other Character.
		auto packet = TradeAccept::construct(pCharacter->getSpawnID());
		tradingWith->getConnection()->sendPacket(packet);
		delete packet;

		// Check: Have both Characters accepted trade.
		if (tradingWith->isTradeAccepted()) {

			if (!trade(pCharacter, tradingWith))
				mLog->error("Trade failed");

			pCharacter->setTradingWith(nullptr);
			pCharacter->setTradeAccepted(false);
			pCharacter->getConnection()->sendTradeFinished();
			mZone->saveCharacter(pCharacter);

			tradingWith->setTradingWith(nullptr);
			tradingWith->setTradeAccepted(false);
			tradingWith->getConnection()->sendTradeFinished();
			mZone->saveCharacter(tradingWith);

			return;
		}
	}
	// Handle: Accepted trade with an NPC.
	else if (pCharacter->isTradingWithNPC()) {
		auto tradingWith = Actor::cast<NPC*>(tradingWithActor);

		pCharacter->getConnection()->sendTradeFinished();
		pCharacter->setTradingWith(nullptr);

		//const int64 tradeCurrency = pCharacter->getInventory()->getTotalTradeCurrency();
		//pCharacter->getInventory()->onTradeAccept();
	}
	// Handle: Unknown.
	else {
		mLog->error(pCharacter->getName() + " trading with unknown type.");
	}
}

const bool TradeHandler::trade(Character* pCharacterA, Character* pCharacterB) {
	if (!pCharacterA) return false;
	auto inventoryA = pCharacterA->getInventory();

	if (!pCharacterB) return false;
	auto inventoryB = pCharacterB->getInventory();

	const auto preTradeCurrency = inventoryA->getTotalCurrency() + inventoryB->getTotalCurrency();

	std::list<Item*> unorderedA;
	std::list<Item*> orderedA;
	inventoryA->getTradeItems(unorderedA);
	inventoryA->clearTradeItems();
	auto copperA = inventoryA->getTradeCopper();
	auto silverA = inventoryA->getTradeSilver();
	auto goldA = inventoryA->getTradeGold();
	auto platinumA = inventoryA->getTradePlatinum();
	inventoryA->clearTradeCurrency();

	std::list<Item*> unorderedB;
	std::list<Item*> orderedB;
	inventoryB->getTradeItems(unorderedB);
	inventoryB->clearTradeItems();
	auto copperB = inventoryB->getTradeCopper();
	auto silverB = inventoryB->getTradeSilver();
	auto goldB = inventoryB->getTradeGold();
	auto platinumB = inventoryB->getTradePlatinum();
	inventoryB->clearTradeCurrency();

	// TODO:
	StringStream tradeSummary;
	tradeSummary << "[Trade Summary] " << pCharacterA->getName() << " trading with " << pCharacterB->getName();

	// Give A to B.
	if (!unorderedA.empty()) {
		orderItems(unorderedA, orderedA);
		if (!mZone->giveItems(pCharacterB, orderedA, InventoryReason::Trade))
			mLog->error("Failed to give items A to B");
	}
	inventoryB->addCurrency(CurrencySlot::Personal, platinumA, goldA, silverA, copperA, InventoryReason::Trade);
	pCharacterB->getConnection()->sendCurrencyUpdate();

	// Give B to A.
	if (!unorderedB.empty()) {
		orderItems(unorderedB, orderedB);
		if (!mZone->giveItems(pCharacterA, orderedB, InventoryReason::Trade))
			mLog->error("Failed to give items B to A");
	}
	inventoryA->addCurrency(CurrencySlot::Personal, platinumB, goldB, silverB, copperB, InventoryReason::Trade);
	pCharacterA->getConnection()->sendCurrencyUpdate();

	// Check: The total number of copper pieces between the two Characters matches the pre-trade total.
	// Note: This /should/ catch 99.9% of errors / duping that involves trading.
	const auto postTradeCurrency = inventoryA->getTotalCurrency() + inventoryB->getTotalCurrency();
	if (preTradeCurrency != postTradeCurrency) {
		mLog->error("Post trade currency total did not match! pre= " + toString(preTradeCurrency) + " post=" + toString(postTradeCurrency));
		return false;
	}

	return true;
}

void TradeHandler::onCancel(Character* pCharacter, const u32 pSpawnID) {
	using namespace Payload::Zone;
	if (!pCharacter) return;

	// Check: Character is trading.
	if (!pCharacter->isTrading()) {
		mLog->error(pCharacter->getName() + " canceled trade but they are not trading.");
		return;
	}

	// Handle: Character has canceled while trading with a Character.
	if (pCharacter->isTradingWithCharacter()) {
		auto tradingWith = Actor::cast<Character*>(pCharacter->getTradingWith());

		pCharacter->getConnection()->sendTradeCancel(pCharacter->getSpawnID(), tradingWith->getSpawnID());
		pCharacter->getConnection()->sendCharacterTradeClose();
		pCharacter->setTradingWith(nullptr);
		pCharacter->setTradeAccepted(false);

		// Return trade currency.
		if (!returnTradeCurrency(pCharacter))
			mLog->error("Failed to return trade currency to " + pCharacter->getName());

		// Return trade Items.
		if (!returnTradeItems(pCharacter))
			mLog->error("Failed to return trade items to " + pCharacter->getName());

		tradingWith->getConnection()->sendTradeCancel(tradingWith->getSpawnID(), pCharacter->getSpawnID());
		tradingWith->getConnection()->sendCharacterTradeClose();
		tradingWith->setTradingWith(nullptr);
		tradingWith->setTradeAccepted(false);

		// Return trade currency.
		if (!returnTradeCurrency(tradingWith))
			mLog->error("Failed to return trade currency to " + tradingWith->getName());

		// Return trade Items.
		if (!returnTradeItems(tradingWith))
			mLog->error("Failed to return trade items to " + tradingWith->getName());
	}
	// Handle: Character has canceled while trading with an NPC.
	else if (pCharacter->isTradingWithNPC()) {
		auto connection = pCharacter->getConnection();
		auto tradingWith = Actor::cast<NPC*>(pCharacter->getTradingWith());

		connection->sendTradeCancel(pCharacter->getSpawnID(), tradingWith->getSpawnID());
		connection->sendCharacterTradeClose();
		connection->sendFinishWindow2();
		pCharacter->setTradingWith(nullptr);

		// Return trade currency.
		if (!returnTradeCurrency(pCharacter))
			mLog->error("Failed to return trade currency to " + pCharacter->getName());

		// Return trade Items.
		if (!returnTradeItems(pCharacter))
			mLog->error("Failed to return trade items to " + pCharacter->getName());
	}
}

const bool TradeHandler::returnTradeItems(Character* pCharacter)
{
	if (!pCharacter) return false;

	// Note: Ideally returning trade items would be a simple as generating 0-8 OP_MoveItem packets but that does not seem to work.

	auto inventory = pCharacter->getInventory();

	// Make a list of Items that are in the trade window.
	std::list<Item*> unordered;
	std::list<Item*> ordered;
	inventory->getTradeItems(unordered);

	// Check: Nothing to return.
	if (unordered.empty()) return true;

	orderItems(unordered, ordered);

	// Clear all trade Items.
	pCharacter->getInventory()->clearTradeItems();

	// Return all trade Items back into Inventory.
	if (!mZone->giveItems(pCharacter, ordered, InventoryReason::TradeReturn)){
		mLog->error("Failed to return trade Item to Character.");
	}

	return true;
}

const bool TradeHandler::returnTradeCurrency(Character* pCharacter)
{
	if (!pCharacter) return false;

	auto inventory = pCharacter->getInventory();

	auto copper = inventory->getTradeCopper();
	auto silver = inventory->getTradeSilver();
	auto gold = inventory->getTradeGold();
	auto platinum = inventory->getTradePlatinum();

	inventory->clearTradeCurrency();

	// Add currency that was in trade back into personal.
	if (!inventory->addCurrency(CurrencySlot::Personal, platinum, gold, silver, copper, InventoryReason::TradeReturn))
		mLog->error("Returning trade currency failed.");

	// Update Character.
	pCharacter->getConnection()->sendCurrencyUpdate();

	return true;
}

void TradeHandler::orderItems(std::list<Item*>& pUnordered, std::list<Item*>& pOrdered) {
	// Adjust order of Items so that containers are sent first.
	for (auto returnItem = pUnordered.begin(); returnItem != pUnordered.end();) {
		if ((*returnItem)->isContainer()) {
			pOrdered.push_back(*returnItem);
			returnItem = pUnordered.erase(returnItem);
			continue;
		}
		returnItem++;
	}

	// Add remaining Items.
	for (auto returnItem : pUnordered)
		pOrdered.push_back(returnItem);
}

void TradeHandler::onMoveCurrency(Character* pCharacter, const u32 pType, const i32 pAmount) {
	using namespace Payload::Zone;
	if (!pCharacter) return;
	if (!pCharacter->isTrading()) return;
	if (!pCharacter->isTradingWithCharacter()) return;

	auto tradingWith = Actor::cast<Character*>(pCharacter->getTradingWith());

	// Moving currency into trade resets 'accepted'.
	pCharacter->setTradeAccepted(false);
	tradingWith->setTradeAccepted(false);

	// Update the Character being traded with.
	auto packet = TradeCurrencyUpdate::construct(pCharacter->getSpawnID(), pType, pAmount);
	tradingWith->getConnection()->sendPacket(packet);
	delete packet;
}
