#pragma once

#include "Types.h"

class ILog;
class ILogFactory;
class ItemDataStore;
class Item;

class ItemFactory {
public:

	~ItemFactory();

	const bool initialise(ItemDataStore* pItemDataStore, ILogFactory* pLogFactory);

	Item* make(const u32 pItemID, const u32 pStacks = 1);
	Item* make();
	Item* makeAugment();
	Item* copy(Item* pItem);
	
private:

	bool mInitialised = false;
	ILog* mLog = nullptr;
	ItemDataStore* mItemDataStore = nullptr;
};