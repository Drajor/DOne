#include "ItemGenerator.h"
#include "ServiceLocator.h"
#include "ItemData.h"
#include "Item.h"
#include "Random.h"
#include "ItemDataStore.h"
#include "ItemFactory.h"

namespace RandomItemType {
	enum : u32 {
		OneHandSlash,
		TwoHandSlash,
		OneHandPierce,
		TwoHandPierce,
		OneHandBlunt,
		TwoHandBlunt,
		HandtoHand,
		Bow,
		Shield,
		Charm,
		
		Range,
		
		Head,
		Arms,
		Wrists,
		Hands,
		Chest,
		Legs,
		Feet,

		Ears,
		Rings,
		Neck,
		Face,
		Shoulders,
		Back,
		Waist,
		Max
	};
};

const u32 getColour() {
	static std::vector<u32> colours = {
		// Classic
		4285761480, // Crafted (Warrior)
		4287401060, // Totemic (Shaman)
		4278256072, // Lambent (Bard)

		// Planar
		4290027570, // Imbrued (Bard)

		// Kunark
		4281479770, // Cobalt (Warrior)
		4293322400, // Jaundiced Bone (Shaman)
		4282035835, // Singing Steel (Bard)
		4281466880, // Blood Ember (Shadowknight)
		4284769400, // Donal's (Cleric)
		4291338260, // Deepwater (Paladin)
		4283443220, // Tolan's (Ranger)
		4278858250, // Elder Spiritist's (Druid)
		4280819230, // Mrylokar's (Rogue)
	};

	return colours[Random::make<u32>(0, colours.size() - 1)];
}

ItemGenerator::ItemGenerator(ServiceLocator* pServiceLocator) : mServiceLocator(pServiceLocator) { }

const bool ItemGenerator::initialise() {
	EXPECTED_BOOL(mInitialised == false);
	EXPECTED_BOOL(mServiceLocator);
	mItemFactory = mServiceLocator->getItemFactory();
	EXPECTED_BOOL(mItemFactory);

	return true;
}

Item* ItemGenerator::_makeBaseItem() {
	return mItemFactory->make();
}

Item* ItemGenerator::_makeBaseArmorItem(const u8 pLevel) {
	auto item = _makeBaseItem();
	auto data = item->getData();
	data->mAugmentationSlots[0].mType = AugmentationSlotType::MultipleStat;
	data->mAugmentationSlots[1].mType = AugmentationSlotType::MultipleStat;
	data->mAugmentationSlots[2].mType = AugmentationSlotType::MultipleStat;
	data->mAugmentationSlots[3].mType = AugmentationSlotType::MultipleStat;
	data->mAugmentationSlots[4].mType = AugmentationSlotType::MultipleStat;

	item->setRequiredLevel(pLevel);
	item->setColour(getColour());

	item->setHealth(Random::make<i32>(pLevel * 3));
	item->setMana(Random::make<i32>(pLevel * 3));
	item->setEndurance(Random::make<i32>(pLevel * 3));

	// Armor always gets some AC.

	// Determine number of affixes.

	// Apply affixes

	return item;
}

Item* ItemGenerator::_makeBaseOneHandWeapon() {
	auto item = _makeBaseItem();
	auto data = item->getData();
	data->mAugmentationSlots[0].mType = AugmentationSlotType::MultipleStat;
	data->mAugmentationSlots[1].mType = AugmentationSlotType::MultipleStat;
	data->mAugmentationSlots[2].mType = AugmentationSlotType::MultipleStat;
	data->mAugmentationSlots[3].mType = AugmentationSlotType::MultipleStat;
	data->mAugmentationSlots[4].mType = AugmentationSlotType::MultipleStat;

	return item;
}

Item* ItemGenerator::makeRandom(const u8 pLevel, const Rarity pRarity) {
	const u32 itemType = Random::make<u32>(RandomItemType::Max);

	switch (itemType) {
	case RandomItemType::OneHandSlash: return makeOneHandSlash(pLevel, pRarity);
	case RandomItemType::TwoHandSlash: return makeTwoHandSlash(pLevel, pRarity);
	case RandomItemType::OneHandPierce: return makeOneHandPierce(pLevel, pRarity);
	case RandomItemType::TwoHandPierce: return makeTwoHandPierce(pLevel, pRarity);
	case RandomItemType::OneHandBlunt: return makeOneHandBlunt(pLevel, pRarity);
	case RandomItemType::TwoHandBlunt: return makeTwoHandBlunt(pLevel, pRarity);
	case RandomItemType::HandtoHand: return makeHandtoHand(pLevel, pRarity);
	case RandomItemType::Bow: return makeBow(pLevel, pRarity);
	case RandomItemType::Shield: return makeShield(pLevel, pRarity);
	case RandomItemType::Charm: return makeCharm(pLevel, pRarity);
	case RandomItemType::Range: return makeRange(pLevel, pRarity);
	// Armor
	case RandomItemType::Head: return makeHead(pLevel, pRarity);
	case RandomItemType::Arms: return makeArms(pLevel, pRarity);
	case RandomItemType::Wrists: return makeWrists(pLevel, pRarity);
	case RandomItemType::Hands: return makeHands(pLevel, pRarity);
	case RandomItemType::Chest: return makeChest(pLevel, pRarity);
	case RandomItemType::Legs: return makeLegs(pLevel, pRarity);
	case RandomItemType::Feet: return makeFeet(pLevel, pRarity);
	// Misc
	case RandomItemType::Ears: return makeEars(pLevel, pRarity);
	case RandomItemType::Rings: return makeRing(pLevel, pRarity);
	case RandomItemType::Neck: return makeNeck(pLevel, pRarity);
	case RandomItemType::Face: return makeFace(pLevel, pRarity);
	case RandomItemType::Shoulders: return makeShoulder(pLevel, pRarity);
	case RandomItemType::Back: return makeBack(pLevel, pRarity);
	case RandomItemType::Waist: return makeWaist(pLevel, pRarity);

	default:
		return makeFood();
	}
}

Item* ItemGenerator::_makeRandom(const u8 pLevel) {
	Item* item = _makeBaseItem();
	item->setRequiredLevel(pLevel);

	return item;
}

Item* ItemGenerator::makeFood() {
	Item* item = _makeBaseItem();

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
	Item* item = _makeBaseItem();

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

Item* ItemGenerator::makePowerSource(const u8 pLevel, const Rarity pRarity) {
	Item* item = _makeBaseItem();

	item->setName("PowerSource");
	item->setCharmFile("PS-Air");
	item->setIcon(2095);
	item->setIDFile("IT7");
	item->setItemType(ItemType::Miscellaneous);
	item->setIsMagic(true);
	item->setSlots(EquipSlots::PowerSource);
	item->setMaxPower(100);
	//item->setPower(Random::make<u32>(0, 100));
	item->setPower(100);
	item->setIsHeirloom(true);

	return item;
}

Item* ItemGenerator::makeShield(const u8 pLevel, const Rarity pRarity) {
	Item* item = _makeBaseItem();

	item->setName("Shield");
	item->setIcon(676);
	item->setIDFile("IT221");
	item->setItemType(ItemType::Shield);
	item->setIsMagic(true);
	item->setSlots(EquipSlots::Secondary);
	item->setSkillMod(1);
	item->setSkillModAmount(1);
	item->setPurity(100);
	item->setStrength(30);
	//item->setClickName("Test");
	//item->setMaxPower(100);
	//item->setPower(Random::make<u32>(0, 100));

	return item;
}

Item* ItemGenerator::makeRandomContainer(const Rarity pRarity) {
	Item* item = _makeBaseItem();

	item->setName(_getContainerName());
	item->setContainerType(ContainerType::Normal);
	item->setContainerSize(ContainerSize::Giant);
	item->setContainerWR(100);
	item->setContainerSlots(_getContainerSlots(pRarity));
	item->setIcon(_getContainerIcon(pRarity));
	item->setItemClass(ItemClass::Container);
	item->setItemType(ItemType::Miscellaneous);

	return item;
}

const u8 ItemGenerator::_getContainerSlots(const Rarity pRarity) {
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

const u32 ItemGenerator::_getContainerIcon(const Rarity pRarity) {
	//std::discrete_distribution<u32> icons = {
	std::vector<u32> icons = {
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
	return icons[Random::make<u32>(0, icons.size()-1)];
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

//const u32 ItemGenerator::getNextID() {
//	mNextID++;
//	return mNextID;
//}
//
//const u32 ItemGenerator::getNextSerial() {
//	mNextSerial++;
//	return mNextSerial;
//}

Item* ItemGenerator::makeOneHandSlash(const u8 pLevel, const Rarity pRarity) {
	Item* item = _makeBaseItem();

	item->setName("One Hand Slash");
	item->setIcon(580);
	item->setIDFile("IT10653");
	item->setItemType(ItemType::OneHandSlash);
	item->setDelay(Random::make<u32>(25, 45));
	item->setDamage(Random::make<u32>(5, 15));
	item->setIsMagic(true);
	item->setSlots(EquipSlots::Primary + EquipSlots::Secondary);

	return item;
}

Item* ItemGenerator::makeTwoHandSlash(const u8 pLevel, const Rarity pRarity) {
	Item* item = _makeBaseItem();

	item->setName("Two Hand Slash");
	item->setIcon(519);
	item->setIDFile("IT10648");
	item->setItemType(ItemType::TwoHandSlash);
	item->setDelay(Random::make<u32>(25, 45));
	item->setDamage(Random::make<u32>(5, 15));
	item->setIsMagic(true);
	item->setSlots(EquipSlots::Primary);

	return item;
}

Item* ItemGenerator::makeOneHandPierce(const u8 pLevel, const Rarity pRarity) {
	Item* item = _makeBaseItem();

	item->setName("One Hand Pierce");
	item->setIcon(592);
	item->setIDFile("IT10650");
	item->setItemType(ItemType::OneHandPierce);
	item->setDelay(Random::make<u32>(25, 45));
	item->setDamage(Random::make<u32>(5, 15));
	item->setIsMagic(true);
	item->setSlots(EquipSlots::Primary + EquipSlots::Secondary);

	return item;
}

Item* ItemGenerator::makeTwoHandPierce(const u8 pLevel, const Rarity pRarity) {
	Item* item = _makeBaseItem();

	item->setName("Two Hand Pierce");
	item->setIcon(776);
	item->setIDFile("IT172");
	item->setItemType(ItemType::TwoHandPierce);
	item->setDelay(Random::make<u32>(25, 45));
	item->setDamage(Random::make<u32>(5, 15));
	item->setIsMagic(true);
	item->setSlots(EquipSlots::Primary);

	return item;
}

Item* ItemGenerator::makeOneHandBlunt(const u8 pLevel, const Rarity pRarity) {
	Item* item = _makeBaseOneHandWeapon();

	item->setName("One Hand Blunt");
	item->setIcon(578);
	item->setIDFile("IT7");
	item->setItemType(ItemType::OneHandBlunt);
	item->setDelay(Random::make<u32>(25, 45));
	item->setDamage(Random::make<u32>(5, 15));
	item->setIsMagic(true);
	item->setSlots(EquipSlots::PrimarySecondary);
	//item->setIsCopied(8);

	item->setIsEvolvingItem(true);
	item->setEvolvingLevel(3);
	item->setEvolvingMaximumLevel(4);
	item->setEvolvingProgress(55.4);
	item->setLoreGroup(1502);
	//item

	//item->setOrnamentationIcon(1346);
	//item->setOrnamentationIDFile("IT10634");
	Log::error("ID = " + std::to_string(item->getID()));
	return item;
}


Item* ItemGenerator::makeTwoHandBlunt(const u8 pLevel, const Rarity pRarity) {
	Item* item = _makeBaseItem();

	item->setName("Two Hand Blunt");
	item->setIcon(741);
	item->setIDFile("IT10608");
	item->setItemType(ItemType::TwoHandBlunt);
	item->setDelay(Random::make<u32>(25, 45));
	item->setDamage(Random::make<u32>(5, 15));
	item->setIsMagic(true);
	item->setSlots(EquipSlots::Primary);

	//item->setIsHeirloom(true);
	//item->setIsNoTransfer(true);
	//item->setIsFVNoDrop(true);

	//item->setIsEvolvingItem(1501);
	//item->setIsEvolvingItem(true);
	//item->setCurrentEvolvingLevel(3);
	//item->setMaximumEvolvingLevel(4);
	//item->setEvolvingProgress(55.4);
	//item->setLoreGroup(1501);

	//item->setOrnamentationIcon(2174);
	//item->setOrnamentationIDFile("IT11107");

	return item;
}

Item* ItemGenerator::makeHandtoHand(const u8 pLevel, const Rarity pRarity) {
	Item* item = _makeBaseItem();

	item->setSlots(EquipSlots::PrimarySecondary);
	item->setIsMagic(true);

	item->setName("Hand to Hand");
	// TODO

	return item;
}


Item* ItemGenerator::makeBow(const u8 pLevel, const Rarity pRarity) {
	Item* item = _makeBaseItem();

	item->setSlots(EquipSlots::Range);
	item->setIsMagic(true);

	item->setName("Bow");
	item->setIcon(597);
	item->setIDFile("IT10614");
	item->setItemType(ItemType::Bow);
	item->setDelay(Random::make<u32>(25, 45));
	item->setDamage(Random::make<u32>(5, 15));
	
	

	return item;
}

Item* ItemGenerator::makeHead(const u8 pLevel, const Rarity pRarity) {
	Item* item = _makeBaseArmorItem(pLevel);

	item->setName("Head Item");
	item->setIcon(746);
	item->setIDFile("IT63");
	item->setMaterial(ItemMaterial::Plate);
	item->setItemType(ItemType::Armor);
	item->setIsMagic(true);
	item->setSlots(EquipSlots::Head);
	//item->setIsEvolvingItem(1500);
	item->setIsEvolvingItem(true);
	item->setEvolvingLevel(3);
	item->setEvolvingMaximumLevel(4);
	item->setEvolvingProgress(55.4);
	item->setLoreGroup(item->getID()); // http://lucy.allakhazam.com/itemraw.html?id=85612 LoreGroup(loreItem) == ItemID
	item->setWeight(150);
	item->setIsNoDrop(false);
	item->setIsAttunable(true);
	item->setLore("This is a Head Slot item.");

	//item->setOrnamentationIcon(1000);
	//item->setOrnamentationIDFile("IT148");

	return item;
}

Item* ItemGenerator::makeChest(const u8 pLevel, const Rarity pRarity) {
	Item* item = _makeBaseArmorItem(pLevel);

	item->setSlots(EquipSlots::Chest);
	item->setIsMagic(true);

	item->setName("Chest Item");
	item->setIcon(624);
	item->setIDFile("IT63");
	item->setMaterial(ItemMaterial::Plate);
	item->setItemType(ItemType::Armor);

	return item;
}

Item* ItemGenerator::makeArms(const u8 pLevel, const Rarity pRarity) {
	Item* item = _makeBaseArmorItem(pLevel);

	item->setSlots(EquipSlots::Arms);
	item->setIsMagic(true);

	item->setName("Arms Item");
	item->setIcon(622);
	item->setIDFile("IT63");
	item->setMaterial(ItemMaterial::Plate);
	item->setItemType(ItemType::Armor);
	

	return item;
}

Item* ItemGenerator::makeWrists(const u8 pLevel, const Rarity pRarity) {
	Item* item = _makeBaseArmorItem(pLevel);

	item->setSlots(EquipSlots::Wrists);
	item->setIsMagic(true);

	item->setName("Bracers Item");
	item->setIcon(516);
	item->setIDFile("IT63");
	item->setMaterial(ItemMaterial::Plate);
	item->setItemType(ItemType::Armor);

	return item;
}

Item* ItemGenerator::makeLegs(const u8 pLevel, const Rarity pRarity) {
	Item* item = _makeBaseArmorItem(pLevel);

	item->setSlots(EquipSlots::Legs);
	item->setIsMagic(true);

	item->setName("Legs Item");
	item->setIcon(540);
	item->setIDFile("IT63");
	item->setMaterial(ItemMaterial::Plate);
	item->setItemType(ItemType::Armor);
	
	return item;
}

Item* ItemGenerator::makeHands(const u8 pLevel, const Rarity pRarity) {
	Item* item = _makeBaseArmorItem(pLevel);
	
	item->setSlots(EquipSlots::Hands);
	item->setIsMagic(true);
	
	item->setName("Hands Item");
	item->setIcon(531);
	item->setIDFile("IT63");
	item->setMaterial(ItemMaterial::Plate);
	item->setItemType(ItemType::Armor);

	return item;
}

Item* ItemGenerator::makeFeet(const u8 pLevel, const Rarity pRarity) {
	Item* item = _makeBaseArmorItem(pLevel);
	
	item->setSlots(EquipSlots::Feet);
	item->setIsMagic(true);
	
	item->setName("Feet Item");
	item->setIcon(524);
	item->setIDFile("IT63");
	item->setMaterial(ItemMaterial::Plate);
	item->setItemType(ItemType::Armor);
	
	return item;
}

Item* ItemGenerator::makeCharm(const u8 pLevel, const Rarity pRarity) {
	Item* item = _makeBaseItem();
	
	item->setSlots(EquipSlots::Charm);
	item->setIsMagic(true);
	
	item->setName("Charm");

	return item;
}

Item* ItemGenerator::makeRange(const u8 pLevel, const Rarity pRarity) {
	Item* item = _makeBaseItem();
	
	item->setSlots(EquipSlots::Range);
	item->setIsMagic(true);
	
	item->setName("Range");

	return item;
}

Item* ItemGenerator::makeEars(const u8 pLevel, const Rarity pRarity) {
	Item* item = _makeBaseItem();
	
	item->setSlots(EquipSlots::Ears);
	item->setIsMagic(true);
	
	item->setName("Ear");

	return item;
}

Item* ItemGenerator::makeRing(const u8 pLevel, const Rarity pRarity) {
	Item* item = _makeBaseItem();
	
	item->setSlots(EquipSlots::Fingers);
	item->setIsMagic(true);
	
	item->setName("Ring");
	
	return item;
}

Item* ItemGenerator::makeNeck(const u8 pLevel, const Rarity pRarity){
	Item* item = _makeBaseItem();
	
	item->setSlots(EquipSlots::Neck);
	item->setIsMagic(true);
	
	item->setName("Neck");

	return item;
}

Item* ItemGenerator::makeFace(const u8 pLevel, const Rarity pRarity) {
	Item* item = _makeBaseItem();
	
	item->setSlots(EquipSlots::Face);
	item->setIsMagic(true);
	
	item->setName("Face");
	
	return item;
}

Item* ItemGenerator::makeShoulder(const u8 pLevel, const Rarity pRarity) {
	Item* item = _makeBaseItem();
	
	item->setSlots(EquipSlots::Shoulders);
	item->setIsMagic(true);
	
	item->setName("Shoulder");

	return item;
}

Item* ItemGenerator::makeBack(const u8 pLevel, const Rarity pRarity) {
	Item* item = _makeBaseItem();
	
	item->setSlots(EquipSlots::Back);
	item->setIsMagic(true);
	
	item->setName("Back");

	return item;
}

Item* ItemGenerator::makeWaist(const u8 pLevel, const Rarity pRarity) {
	Item* item = _makeBaseItem();
	
	item->setSlots(EquipSlots::Waist);
	item->setIsMagic(true);
	
	item->setName("Waist");

	_addAffixes(item, pLevel, pRarity);
	return item;
}

Item* ItemGenerator::makeAugmentation(const u8 pLevel, const Rarity pRarity) {
	Item* item = _makeBaseItem();

	item->setName("Augmentation");
	item->setIcon(2185);
	item->setAugmentationType(AugmentationSlotType::MultipleStat);
	item->setAugmentationRestriction(AugmentationRestriction::None);
	item->setHealth(100);
	item->setItemType(ItemType::Augmentation);
	item->setSlots(EquipSlots::VisibleArmor);
	item->setAugmentationDistiller(ItemID::UniversalDistiller);

	return item;
}

Item* ItemGenerator::makeDice(String pCharacterName) {
	Item* item = _makeBaseItem();

	item->setName(pCharacterName + "'s Lucky Dice");
	item->setIcon(2889);
	item->setItemType(ItemType::Miscellaneous);
	item->setSlots(EquipSlots::PowerSource);
	item->setMaxPower(100);
	item->setPower(100);
	item->setLore("\"God does not play dice.\" - Albert Einstein");
	item->setIsNoDrop(true);

	return item;
}

void ItemGenerator::_addAffixes(Item* pItem, const u8 pLevel, const Rarity pRarity) {
	// TODO: Data-driven.
	static std::vector<std::pair<u8, u8>> affixRange = {
		{ 1, 2 }, // Common
		{ 2, 4 }, // Magic
		{ 4, 7}, // Rare
	};

	// Determine number of affixes to roll.
	auto affixes = Random::make<u32>(affixRange[pRarity].first, affixRange[pRarity].second);


}
