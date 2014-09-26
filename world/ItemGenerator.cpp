#include "ItemGenerator.h"
#include "ItemData.h"
#include "Item.h"

Item* ItemGenerator::_makeBaseItem() {
	ItemData* data = new ItemData();
	return new Item(data);
}

Item* ItemGenerator::makeRandom(const uint8 pLevel) {
	return ItemGenerator::getInstance()._makeRandom(pLevel);
}

Item* ItemGenerator::_makeRandom(const uint8 pLevel) {
	Item* item = _makeBaseItem();
	item->setReqLevel(pLevel);

	return item;
}
