#pragma once

#include "Types.h"

class ItemDataStore;
class Item;

class ItemFactory {
public:
	const bool initialise(ItemDataStore* pItemDataStore);

	Item* make(const u32 pItemID, const u32 pStacks = 1);
	Item* make();
	Item* makeAugment();
	Item* copy(Item* pItem);
	
private:

	bool mInitialised = false;
	ItemDataStore* mItemDataStore = nullptr;
};