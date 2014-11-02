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
	static Item* makeRandom(const uint8 pLevel, const Rarity pRarity);
	static Item* makeRandomContainer(const Rarity pRarity);
	static Item* makeFood();
	static Item* makeDrink();
	static Item* makePowerSource(const uint8 pLevel, const Rarity pRarity);

	// Weapons
	static Item* makeOneHandSlash(const uint8 pLevel, const Rarity pRarity);
	static Item* makeTwoHandSlash(const uint8 pLevel, const Rarity pRarity);
	static Item* makeOneHandPierce(const uint8 pLevel, const Rarity pRarity);
	static Item* makeTwoHandPierce(const uint8 pLevel, const Rarity pRarity);
	static Item* makeOneHandBlunt(const uint8 pLevel, const Rarity pRarity);
	static Item* makeTwoHandBlunt(const uint8 pLevel, const Rarity pRarity);
	static Item* makeHandtoHand(const uint8 pLevel, const Rarity pRarity);
	static Item* makeBow(const uint8 pLevel, const Rarity pRarity);

	static Item* makeShield(const uint8 pLevel, const Rarity pRarity);
	static Item* makeCharm(const uint8 pLevel, const Rarity pRarity);
	static Item* makeRange(const uint8 pLevel, const Rarity pRarity);

	// Visible Armor
	static Item* makeHead(const uint8 pLevel, const Rarity pRarity);
	static Item* makeChest(const uint8 pLevel, const Rarity pRarity);
	static Item* makeArms(const uint8 pLevel, const Rarity pRarity);
	static Item* makeWrists(const uint8 pLevel, const Rarity pRarity);
	static Item* makeLegs(const uint8 pLevel, const Rarity pRarity);
	static Item* makeHands(const uint8 pLevel, const Rarity pRarity);
	static Item* makeFeet(const uint8 pLevel, const Rarity pRarity);

	static Item* makeEars(const uint8 pLevel, const Rarity pRarity);
	static Item* makeRing(const uint8 pLevel, const Rarity pRarity);
	static Item* makeNeck(const uint8 pLevel, const Rarity pRarity);
	static Item* makeFace(const uint8 pLevel, const Rarity pRarity);
	static Item* makeShoulder(const uint8 pLevel, const Rarity pRarity);
	static Item* makeBack(const uint8 pLevel, const Rarity pRarity);
	static Item* makeWaist(const uint8 pLevel, const Rarity pRarity);
private:
	Item* _makeRandom(const uint8 pLevel);
	Item* _makeBaseItem();

	static const String _getContainerName();
	static const uint8 _getContainerSlots(const Rarity pRarity);
	static const uint32 _getContainerIcon(const Rarity pRarity);
	
	//uint32 mNextID = 10000;
	//const uint32 getNextID();
	//uint32 mNextSerial = 1;
	//const uint32 getNextSerial();
};