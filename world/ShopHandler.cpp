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

	// Every sell attempt is logged.
	StringStream ss;
	ss << "(SELL) " << pCharacter->getName() << " Slot ID: " << pSlotID << " | Stacks: " << pStacks;
	mLog->info(ss.str());

	// Check: Character is shopping.
	if (!pCharacter->isShopping()) {
		mLog->error(pCharacter->getName() + " attempted to sell item but they are not shopping.");
		return;
	}

	auto inventory = pCharacter->getInventory();
	auto shop = pCharacter->getShoppingWith();
	auto item = inventory->get(pSlotID);
	auto updateConsumables = inventory->isAutoFood(item) || inventory->isAutoDrink(item);

	// Check: Distance to merchant.
	if (!canShop(pCharacter, shop)) {
		mLog->error(pCharacter->getName() + " attempted to sell item but they are not close enough.");
		return;
	}

	// Check: Slot ID is valid.
	if (!SlotID::isValidShopSellSlot(pSlotID)) {
		mLog->error(pCharacter->getName() + " attempted to sell item from invalid slot.");
		return;
	}

	// Check: Item is valid.
	if (!item) {
		mLog->error(pCharacter->getName() + " attempted to sell null item.");
		return;
	}

	// Check: Item can be sold.
	if (!item->isSellable()) {
		mLog->error(pCharacter->getName() + " attempted to sell invalid item.");
		return;
	}

	// Check: Item has enough stacks.
	if (item->getStacks() < pStacks) {
		mLog->error(pCharacter->getName() + " attempted to sell more stacks than they own.");
		return;
	}

	// Calculate sale price.
	const u32 price = item->getSellPrice(pStacks, shop->getSellRate());

	const i32 copper = price % 10;
	const i32 silver = (price % 100) / 10;
	const i32 gold = (price % 1000) / 100;
	const i32 platinum = (price / 1000);

	// Add currency to Character.
	if (!inventory->addCurrency(platinum, gold, silver, copper, InventoryReason::ShopSell)){
		mLog->error("Failed to add currency from sale.");
		return;
	}

	// Consume Item/stacks sold.
	if (!inventory->consume(pSlotID, pStacks))
		mLog->error("Failed to remove sold Item from Inventory.");

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

	// Every buy attempt is logged.
	StringStream ss;
	ss << "(BUY) " << pCharacter->getName() << " Instance: " << pInstanceID << " | Stacks: " << pStacks;
	mLog->info(ss.str());

	// Check: Character is shopping.
	if (!pCharacter->isShopping()) {
		mLog->error(pCharacter->getName() + " attempted to buy item but they are not shopping.");
		return;
	}

	auto inventory = pCharacter->getInventory();
	auto connection = pCharacter->getConnection();
	auto shop = pCharacter->getShoppingWith();
	auto item = shop->getShopItem(pInstanceID);

	// Check: Cursor is empty. Underfoot checks this.
	if (!inventory->isCursorEmpty()) {
		mLog->error(pCharacter->getName() + " attempted to buy item but their cursor is not empty.");
		return;
	}

	// Check: Distance to merchant.
	if (!canShop(pCharacter, shop)) {
		mLog->error(pCharacter->getName() + " attempted to buy item but they are not close enough.");
		return;
	}

	// Check: Item is valid.
	if (!item) {
		pCharacter->notify("I seem to have misplaced that. Sorry!");
		mLog->error(pCharacter->getName() + " attempted to sell null item.");
		connection->sendShopBuyReply(shop->getSpawnID(), pInstanceID, pStacks, 0, -1);
		return;
	}

	// Try to buy.
	if (!buy(pCharacter, shop, item, pStacks)) {
		mLog->error("Failed to buy Item.");
		connection->sendShopBuyReply(shop->getSpawnID(), pInstanceID, pStacks, 0, -1);
		return;
	}

	mLog->info("(BUY) Success.");
}

const bool ShopHandler::buy(Character* pCharacter, NPC* pNPC, Item* pItem, const u32 pStacks) {
	if (!pCharacter) return false;
	if (!pNPC) return false;
	if (!pItem) return false;

	auto connection = pCharacter->getConnection();
	auto inventory = pCharacter->getInventory();
	const bool unlimited = pItem->getShopQuantity() == -1;

	// Check: Stacks are valid.
	if (!pItem->isStackable() && pStacks != 1) {
		mLog->error(pCharacter->getName() + " attempted to buy non stackable item with !1 stacks.");
		return false;
	}

	// Check: Enough stacks available.
	if (!unlimited && pItem->getShopQuantity() < pStacks) {
		mLog->error(pCharacter->getName() + " attempted to buy more stacks than are available.");
		return false;
	}
	Item* purchasedItem = nullptr;

	if (unlimited) {
		purchasedItem = mItemFactory->copy(pItem);

		if (pItem->isStackable()) {
			purchasedItem->setStacks(pStacks);
		}
	}
	else {
		// TODO: Remove Item from shop inventory.
		return false;
	}


	// Give Character their purchased Item.
	if (!mZone->giveItem(pCharacter, purchasedItem, InventoryReason::ShopBuy)) {
		mLog->error("Failed to give Item to Character.");
		return false;
	}
	// Calculate cost.
	const u32 price = pItem->getShopPrice() * pStacks;

	// Convert currency from single number to EQ currency.
	i32 platinum = 0, gold = 0, silver = 0, copper = 0;
	Utility::convertFromCopper(price, platinum, gold, silver, copper);

	// Remove currency from Character.
	inventory->removeCurrency(platinum, gold, silver, copper, InventoryReason::ShopBuy);

	// Update client.
	connection->sendCurrencyUpdate();
	connection->sendShopBuyReply(pNPC->getSpawnID(), pItem->getInstanceID(), pStacks, price);

	return true;
}