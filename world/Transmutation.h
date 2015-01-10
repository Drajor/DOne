#pragma once

#include "Types.h"
#include <list>

class IDataStore;
class ItemFactory;
class Item;
struct TransmutationComponent;

class Transmutation {
public:

	const bool initialise();
	Item* transmute(std::list<Item*> pItems);

private:

	bool mInitialised = false;
	IDataStore* mDataStore = nullptr;
	ItemFactory* mItemFactory = nullptr;

	const int32 _roll(Item* pItem, TransmutationComponent* pComponent);
	TransmutationComponent* _get(const uint32 pItemID) const;
	void _bootstrap();
	std::list<TransmutationComponent*> mComponents;
};