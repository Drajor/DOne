#pragma once

#include "Types.h"

class ILogFactory;
class ILog;
class Character;
class Zone;
class ItemFactory;
class NPC;
class Item;

class ShopHandler {
public:
	ShopHandler(Zone* pZone, ItemFactory* pItemFactory, ILogFactory* pLogFactory);
	~ShopHandler();

	// Character is requesting to shop.
	void onRequest(Character* pCharacter, const u32 pSpawnID);

	// Character has finished shopping.
	void onFinished(Character* pCharacter);

	// Character is selling an Item.
	void onSell(Character* pCharacter, const u32 pSlotID, const u32 pStacks);

	// Character is buying an Item.
	void onBuy(Character* pCharacter, const u32 pInstanceID, const u32 pStacks);

	const bool canShop(Character* pCharacter, NPC* pMerchant) const;

private:

	const bool buy(Character* pCharacter, NPC* pNPC, Item* pItem, const u32 pStacks);

	ILog* mLog = nullptr;
	Zone* mZone = nullptr;
	ItemFactory* mItemFactory = nullptr;
};