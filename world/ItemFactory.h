#pragma once

#include "Constants.h"
#include "Singleton.h"

class Item;

class ItemFactory : public Singleton<ItemFactory> {
private:
	friend class Singleton<ItemFactory>;
	ItemFactory() {};
	~ItemFactory() {};
	ItemFactory(ItemFactory const&); // Do not implement.
	void operator=(ItemFactory const&); // Do not implement.
public:
	static Item* make(const uint32 pItemID, const uint32 pStacks = 1);
	static Item* make();
	static Item* makeAugment();

	static Item* copy(Item* pItem) { return getInstance()._copy(pItem); };
private:
	Item* _make(const uint32 pItemID, const uint32 pStacks);
	Item* _make();

	Item* _copy(Item* pItem);
};