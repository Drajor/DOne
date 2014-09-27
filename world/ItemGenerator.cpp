#include "ItemGenerator.h"
#include "ItemData.h"
#include "Item.h"
#include "Random.h"

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

Item* ItemGenerator::makeRandomContainer(const ContainerRarity pRarity) {
	Item* item = ItemGenerator::getInstance()._makeBaseItem();

	item->setID(ItemGenerator::getInstance().getNextItemID());
	item->setName(_getContainerName());
	item->setContainerType(ContainerType::NORMAL);
	item->setContainerSize(ContainerSize::GIANT);
	item->setContainerWR(100);
	item->setContainerSlots(_getContainerSlots(pRarity));
	item->setIcon(_getContainerIcon(pRarity));
	item->setItemClass(ItemClass::CONTAINER);
	item->setItemType(ItemType::MISC);

	return item;
}

const uint8 ItemGenerator::_getContainerSlots(const ContainerRarity pRarity) {
	switch (pRarity) {
	case COMMON:
		return 4;
	case MAGIC:
		return 6;
	case RARE:
		return 8;
	case ARTIFACT:
		return 10;
	default:
		Log::error("Unknown ContainerRarity in makeRandomContainer");
		return 4;
	}
}

const uint32 ItemGenerator::_getContainerIcon(const ContainerRarity pRarity) {
	//std::discrete_distribution<uint32> icons = {
	std::vector<uint32> icons = {
		539,
		557,
		561,
		565,
		582,
		608,
		609,
		667,
		689,
		690,
		691,
		722,
		723,
		724,
		727,
		730,
		739,
		836,
		837,
		883,
		892
	};
	return icons[Random::make<uint32>(0, icons.size())];
	//return Random::make(icons);
	//return MakeRandomInt(0, icons.size());
	/*
	539 (bag)
	557 (bag)
	561 (belt bag)
	565 (bag)
	582 (bag)
	608 (box)
	609 (box)
	667 (bag)
	689 (bag)
	690 (bag)
	691 (bag)
	722 (belt bag)
	723 (box)
	724 (box)
	727 (bag)
	730 (toolbox)
	739 (bag)
	836 (box)
	837 (box)
	883 (kit)
	892 (kit)
	*/
	//return 557; // TODO
}

const String ItemGenerator::_getContainerName() {
	return "Some Big Bag";
}

const uint32 ItemGenerator::getNextItemID() {
	mNextItemID++;
	return mNextItemID;
}
