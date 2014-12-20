#pragma once

#include "Constants.h"
#include "Singleton.h"

class Item;
struct ItemData;
class ItemGenerator : public Singleton<ItemGenerator> {
private:
	friend class Singleton<ItemGenerator>;
	ItemGenerator() {};
	~ItemGenerator() {};
	ItemGenerator(ItemGenerator const&); // Do not implement.
	void operator=(ItemGenerator const&); // Do not implement.
public:
	static Item* makeRandom(const u8 pLevel, const Rarity pRarity);
	static Item* makeRandomContainer(const Rarity pRarity);
	static Item* makeFood();
	static Item* makeDrink();
	static Item* makePowerSource(const u8 pLevel, const Rarity pRarity);

	static Item* makeDice(String pCharacterName);

	// Weapons
	static Item* makeOneHandSlash(const u8 pLevel, const Rarity pRarity);
	static Item* makeTwoHandSlash(const u8 pLevel, const Rarity pRarity);
	static Item* makeOneHandPierce(const u8 pLevel, const Rarity pRarity);
	static Item* makeTwoHandPierce(const u8 pLevel, const Rarity pRarity);
	static Item* makeOneHandBlunt(const u8 pLevel, const Rarity pRarity);
	static Item* makeTwoHandBlunt(const u8 pLevel, const Rarity pRarity);
	static Item* makeHandtoHand(const u8 pLevel, const Rarity pRarity);
	static Item* makeBow(const u8 pLevel, const Rarity pRarity);

	static Item* makeShield(const u8 pLevel, const Rarity pRarity);
	static Item* makeCharm(const u8 pLevel, const Rarity pRarity);
	static Item* makeRange(const u8 pLevel, const Rarity pRarity);

	// Visible Armor
	static Item* makeHead(const u8 pLevel, const Rarity pRarity);
	static Item* makeChest(const u8 pLevel, const Rarity pRarity);
	static Item* makeArms(const u8 pLevel, const Rarity pRarity);
	static Item* makeWrists(const u8 pLevel, const Rarity pRarity);
	static Item* makeLegs(const u8 pLevel, const Rarity pRarity);
	static Item* makeHands(const u8 pLevel, const Rarity pRarity);
	static Item* makeFeet(const u8 pLevel, const Rarity pRarity);

	static Item* makeEars(const u8 pLevel, const Rarity pRarity);
	static Item* makeRing(const u8 pLevel, const Rarity pRarity);
	static Item* makeNeck(const u8 pLevel, const Rarity pRarity);
	static Item* makeFace(const u8 pLevel, const Rarity pRarity);
	static Item* makeShoulder(const u8 pLevel, const Rarity pRarity);
	static Item* makeBack(const u8 pLevel, const Rarity pRarity);
	static Item* makeWaist(const u8 pLevel, const Rarity pRarity);

	// Augmentations
	static Item* makeAugmentation(const u8 pLevel, const Rarity pRarity);
private:
	Item* _makeBaseArmorItem(const u8 pLevel);
	Item* _makeBaseOneHandWeapon();
	Item* _makeRandom(const u8 pLevel);
	Item* _makeBaseItem();

	void _addAffixes(Item* pItem, const u8 pLevel, const Rarity pRarity);

	static const String _getContainerName();
	static const u8 _getContainerSlots(const Rarity pRarity);
	static const uint32 _getContainerIcon(const Rarity pRarity);
	
	//uint32 mNextID = 10000;
	//const uint32 getNextID();
	//uint32 mNextSerial = 1;
	//const uint32 getNextSerial();
};