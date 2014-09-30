#include "ItemGenerator.h"
#include "ItemData.h"
#include "Item.h"
#include "Random.h"

Item* ItemGenerator::_makeBaseItem() {
	auto data = new ItemData();
	auto item = new Item(data);
	item->setID(getInstance().getNextID());
	item->setSerial(getInstance().getNextSerial());

	return item;
}

Item* ItemGenerator::makeRandom(const uint8 pLevel) {
	return ItemGenerator::getInstance()._makeRandom(pLevel);
}

Item* ItemGenerator::_makeRandom(const uint8 pLevel) {
	Item* item = _makeBaseItem();
	item->setReqLevel(pLevel);

	return item;
}

Item* ItemGenerator::makeFood() {
	Item* item = ItemGenerator::getInstance()._makeBaseItem();

	item->setName("Food");
	item->setIcon(537);
	item->setMaxStacks(20);
	item->setStacks(20);
	item->setFoodSize(1);
	item->setSize(ItemSize::Small);
	item->setItemClass(ItemClass::Common);
	item->setItemType(ItemType::Food);
	item->setStrength(15);

	return item;
}

Item* ItemGenerator::makeDrink() {
	Item* item = ItemGenerator::getInstance()._makeBaseItem();

	item->setName("Drink");
	item->setIcon(856);
	item->setMaxStacks(20);
	item->setStacks(20);
	item->setDrinkSize(1);
	item->setSize(ItemSize::Small);
	item->setItemClass(ItemClass::Common);
	item->setItemType(ItemType::Drink);

	return item;
}

Item* ItemGenerator::makePowerSource(const uint8 pLevel, const Rarity pRarity) {
	Item* item = ItemGenerator::getInstance()._makeBaseItem();

	item->setName("PowerSource");
	item->setCharmFile("PS-Air");
	item->setIcon(2095);
	item->setIDFile("IT7");
	item->setItemType(ItemType::Miscellaneous);
	item->setIsMagic(true);
	item->setSlots(EquipSlots::PowerSource);
	item->setMaxPower(100);
	item->setPower(Random::make<uint32>(0, 100));

	return item;
}


Item* ItemGenerator::makeOneHandBlunt(const uint8 pLevel, const Rarity pRarity) {
	Item* item = ItemGenerator::getInstance()._makeBaseItem();

	item->setName("One Hand Blunt");
	item->setIcon(578);
	item->setIDFile("IT7");
	item->setItemType(ItemType::OneHandBlunt);
	item->setDelay(Random::make<uint32>(25, 45));
	item->setDamage(Random::make<uint32>(5, 15));
	item->setIsMagic(true);
	item->setSlots(EquipSlots::Waist);

	return item;
}

Item* ItemGenerator::makeShield(const uint8 pLevel, const Rarity pRarity) {
	Item* item = ItemGenerator::getInstance()._makeBaseItem();

	item->setName("Shield");
	item->setIcon(676);
	item->setIDFile("IT221");
	item->setItemType(ItemType::Shield);
	item->setIsMagic(true);
	item->setSlots(EquipSlots::Secondary);
	item->setSkillMod(1);
	item->setSkillModAmount(1);
	//item->setClickName("Test");
	//item->setMaxPower(100);
	//item->setPower(Random::make<uint32>(0, 100));

	return item;
}

Item* ItemGenerator::makeRandomContainer(const Rarity pRarity) {
	Item* item = ItemGenerator::getInstance()._makeBaseItem();

	item->setName(_getContainerName());
	item->setContainerType(ContainerType::NORMAL);
	item->setContainerSize(ContainerSize::Giant);
	item->setContainerWR(100);
	item->setContainerSlots(_getContainerSlots(pRarity));
	item->setIcon(_getContainerIcon(pRarity));
	item->setItemClass(ItemClass::Container);
	item->setItemType(ItemType::Miscellaneous);

	return item;
}

const uint8 ItemGenerator::_getContainerSlots(const Rarity pRarity) {
	switch (pRarity) {
	case Common:
		return 4;
	case Magic:
		return 6;
	case Rare:
		return 8;
	case Artifact:
		return 10;
	default:
		Log::error("Unknown ContainerRarity in makeRandomContainer");
		return 4;
	}
}

const uint32 ItemGenerator::_getContainerIcon(const Rarity pRarity) {
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

const uint32 ItemGenerator::getNextID() {
	mNextID++;
	return mNextID;
}

const uint32 ItemGenerator::getNextSerial() {
	mNextSerial++;
	return mNextSerial;
}