#include "ItemFactory.h"
#include "Item.h"
#include "ItemDataStore.h"

Item* ItemFactory::make(const uint32 pItemID, const uint32 pStacks) {
	return getInstance()._make(pItemID, pStacks);
}

Item* ItemFactory::make(){
	return getInstance()._make();
}

Item* ItemFactory::_make(const uint32 pItemID, const uint32 pStacks) {
	auto data = ItemDataStore::getInstance().get(pItemID);
	EXPECTED_PTR(data);
	auto item = new Item(data);
	item->setInstanceID(ItemDataStore::getInstance().getNextSerial());
	item->setStacks(pStacks);
	item->setCharmFile(std::to_string(item->getID()) + "|" + std::to_string(item->getInstanceID()));

	// Instance ID.

	return item;
}

Item* ItemFactory::_make() {
	auto data =ItemDataStore::getInstance().getNew();
	EXPECTED_PTR(data);
	auto item = new Item(data);
	item->setCharmFile(std::to_string(item->getID()) + "|" + std::to_string(item->getInstanceID()));

	return item;
}
