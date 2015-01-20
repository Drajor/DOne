#pragma once

#include "Types.h"
#include <list>

class ILog;
class ILogFactory;
class IDataStore;
class ItemFactory;
class Item;
struct TransmutationComponent;

class Transmutation {
public:

	const bool initialise(IDataStore* pDataStore, ILogFactory* pLogFactory, ItemFactory* pItemFactory);
	Item* transmute(std::list<Item*> pItems);

private:

	bool mInitialised = false;
	ILog* mLog = nullptr;
	IDataStore* mDataStore = nullptr;
	ItemFactory* mItemFactory = nullptr;

	const i32 _roll(Item* pItem, TransmutationComponent* pComponent);
	TransmutationComponent* _get(const uint32 pItemID) const;
	void _bootstrap();
	std::list<TransmutationComponent*> mComponents;
};