#include "ShopHandler.h"
#include "LogSystem.h"
#include "Character.h"
#include "Inventory.h"
#include "Item.h"
#include "ItemFactory.h"
#include "NPC.h"
#include "Zone.h"
#include "ZoneConnection.h"

ShopHandler::ShopHandler(Zone* pZone, ItemFactory* pItemFactory, ILogFactory* pLogFactory) : mZone(pZone) {
	mLog = pLogFactory->make();
	mLog->setContext("[ShopHandler]");

	mItemFactory = pItemFactory;
}

ShopHandler::~ShopHandler() {
	delete mLog;
	mLog = nullptr;
}

void ShopHandler::onRequest(Character* pCharacter, const u32 pSpawnID) {
	if (!pCharacter) return;

	auto connection = pCharacter->getConnection();

	// Check: Character is in a state that allows for shopping.
	if (!pCharacter->canShop()){
		return;
	}

	// Find Actor by pSpawnID.
	auto actor = mZone->getActor(pSpawnID);
	if (!actor) {
		return;
	}

	if (!actor->isNPC()) {
		return;
	}

	auto npc = Actor::cast<NPC*>(actor);

	// Check: NPC is a merchant.
	if (!npc->isMerchant()) {
		return;
	}

	// Check: Character is close enough to NPC.
	if (!canShop(pCharacter, npc)){
		return;
	}

	// Check: Character has Items on cursor.
	// NOTE: UF does not prevent this, this is just to keep things more simple.
	if (pCharacter->getInventory()->isCursorEmpty() == false) {
		pCharacter->notify("Please clear your cursor and try again.");
		connection->sendShopRequestReply(pSpawnID, 0);
		return;
	}

	// Merchant is open for business.
	if (npc->isShopOpen()) {
		// Associate Character and NPC.
		pCharacter->setShoppingWith(npc);
		npc->addShopper(pCharacter);

		connection->sendShopRequestReply(pSpawnID, 1, npc->_getSellRate());

		// Send shop Items.
		std::list<Item*> shopItems = npc->getShopItems();
		for (auto i : shopItems) {
			connection->sendItemShop(i);
		}
	}
	// Merchant is busy.
	else {
		connection->sendSimpleMessage(MessageType::Yellow, StringID::MERCHANT_BUSY);
		connection->sendShopRequestReply(pSpawnID, 0);
	}
}

void ShopHandler::onFinished(Character* pCharacter) {
	if (!pCharacter) return;
	if (!pCharacter->isShopping()) return;

	// Dissociate Character and NPC.
	auto npc = pCharacter->getShoppingWith();
	npc->removeShopper(pCharacter);
	pCharacter->setShoppingWith(nullptr);

	pCharacter->getConnection()->sendShopEndReply();
}


const bool ShopHandler::canShop(Character* pCharacter, NPC* pMerchant) const {
	if (!pCharacter) return false;
	if (!pMerchant) return false;

	static const float MaxShoppingDistance = 405.0f; // This is fairly close. Have seen 401.

	return pCharacter->squareDistanceTo(pMerchant) <= MaxShoppingDistance;
}

void ShopHandler::onSell(Character* pCharacter, const u32 pSlotID, const u32 pStacks) {
	if (!pCharacter) return;
	if (!pCharacter->isShopping()) return;

	auto inventory = pCharacter->getInventory();
	auto shop = pCharacter->getShoppingWith();

	// Check: Distance to merchant.
	if (!canShop(pCharacter, shop)) {
		// TODO: Log.
		return;
	}

	// Check: Slot ID is valid.
	if (!SlotID::isValidShopSellSlot(pSlotID)) {
		// TODO: Log.
		return;
	}

	auto item = inventory->get(pSlotID);

	// Check: Item is valid.
	if (!item) {
		// TODO: Log.
		return;
	}

	// Check: Item can be sold.
	if (!item->isSellable()) {
		// TODO: Log.
		return;
	}

	// Check: Item has enough stacks.
	if (item->getStacks() >= pStacks) {
		// TODO: Log.
		return;
	}

	// Calculate sale price.
	const u32 price = item->getSellPrice(pStacks, shop->getSellRate());

	const i32 copper = price % 10;
	const i32 silver = (price % 100) / 10;
	const i32 gold = (price % 1000) / 100;
	const i32 platinum = (price / 1000);

	// Add currency to Character.
	inventory->addCurrency(platinum, gold, silver, copper, CurrencyReason::ShopSell);

	// Detect when the Character's auto food / drink is being sold.
	const bool updateConsumables = pCharacter->getInventory()->isAutoFood(item) || pCharacter->getInventory()->isAutoDrink(item);

	// Consume Item/stacks sold.
	pCharacter->getInventory()->consume(pSlotID, pStacks);
	item = nullptr;

	// Update consumables.
	if (updateConsumables)
		inventory->updateConsumables();

	// Item being sold from a worn slot.
	if (SlotID::isWorn(pSlotID)) {
		// TODO:
	}

	// Send reply.
	pCharacter->getConnection()->sendShopSellReply(shop->getSpawnID(), pSlotID, pStacks, price);

	// TODO: Dynamic merchant Items.
}

void ShopHandler::onBuy(Character* pCharacter, const u32 pInstanceID, const u32 pStacks) {
	if (!pCharacter) return;
	if (!pCharacter->isShopping()) return;

	auto inventory = pCharacter->getInventory();
	auto connection = pCharacter->getConnection();
	auto shop = pCharacter->getShoppingWith();

	// Check: Cursor is empty. Underfoot checks this.
	if (!inventory->isCursorEmpty()) {
		// TODO: Log.
		return;
	}

	// Check: Distance to merchant.
	if (!canShop(pCharacter, shop)) {
		// TODO: Log.
		return;
	}

	// Find Item.
	auto item = shop->getShopItem(pInstanceID);
	if (!item) {
		pCharacter->notify("I seem to have misplaced that. Sorry!");
		// Send failure reply to prevent UI locking up.
		connection->sendShopBuyReply(shop->getSpawnID(), pInstanceID, pStacks, 0, -1);
		return;
	}

	// Try to buy.
	if (!buy(pCharacter, shop, item, pStacks)) {
		connection->sendShopBuyReply(shop->getSpawnID(), pInstanceID, pStacks, 0, -1);
		return;
	}

	// Calculate cost.
	const u32 price = item->getShopPrice() * pStacks;

	// Convert currency from single number to EQ currency.
	i32 platinum = 0, gold = 0, silver = 0, copper = 0;
	Utility::convertFromCopper(price, platinum, gold, silver, copper);

	// Remove currency from Character.
	inventory->removeCurrency(platinum, gold, silver, copper, CurrencyReason::ShopBuy);

	// Update client.
	connection->sendCurrencyUpdate();
	connection->sendShopBuyReply(shop->getSpawnID(), pInstanceID, pStacks, price);
}

const bool ShopHandler::buy(Character* pCharacter, NPC* pNPC, Item* pItem, const u32 pStacks) {
	if (!pCharacter) return false;
	if (!pNPC) return false;
	if (!pItem) return false;

	// Unlimited Quantity.
	if (pItem->getShopQuantity() == -1) {
		// Non-stackable
		if (pItem->isStackable() == false) {
			EXPECTED_BOOL(pStacks == 1);

			// Try to find an empty slot for the Item.
			const u32 slotID = pCharacter->getInventory()->findSlotFor(pItem->isContainer(), pItem->getSize());

			// No empty slot found.
			if (SlotID::isNone(slotID)) {
				// NOTE: UF still sends the packet when it knows the Inventory is full. Go figure.
				// X tells you, 'Your inventory appears full! How can you buy more?'
				return false;
			}

			// Make a copy of the shop Item.
			Item* purchasedItem = mItemFactory->copy(pItem);

			// Put Item into Inventory.
			EXPECTED_BOOL(pCharacter->getInventory()->put(purchasedItem, slotID));

			// Update client.
			pCharacter->getConnection()->sendItemTrade(purchasedItem);

			// Update currency.
			//purchasedItem->getB

			return true;
		}
		// Stackable
		else {

			// Try to find an existing stack.
			//pCharacter->getInventory()->findStackable(pItem->getID());
		}
	}
	// Limited Quantity.
	else {
		// TODO: Dynamic shop Items.
		// ItemPacketMerchant
	}

	return false;
}
