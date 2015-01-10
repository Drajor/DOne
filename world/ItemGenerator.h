#pragma once

#include "Constants.h"
#include "Singleton.h"

class ServiceLocator;
class ItemFactory;
class Item;
struct ItemData;

class ItemGenerator {
public:

	ItemGenerator(ServiceLocator* pServiceLocator);
	const bool initialise();

	Item* makeRandom(const u8 pLevel, const Rarity pRarity);
	Item* makeRandomContainer(const Rarity pRarity);
	Item* makeFood();
	Item* makeDrink();
	Item* makePowerSource(const u8 pLevel, const Rarity pRarity);

	Item* makeDice(String pCharacterName);

	// Weapons
	Item* makeOneHandSlash(const u8 pLevel, const Rarity pRarity);
	Item* makeTwoHandSlash(const u8 pLevel, const Rarity pRarity);
	Item* makeOneHandPierce(const u8 pLevel, const Rarity pRarity);
	Item* makeTwoHandPierce(const u8 pLevel, const Rarity pRarity);
	Item* makeOneHandBlunt(const u8 pLevel, const Rarity pRarity);
	Item* makeTwoHandBlunt(const u8 pLevel, const Rarity pRarity);
	Item* makeHandtoHand(const u8 pLevel, const Rarity pRarity);
	Item* makeBow(const u8 pLevel, const Rarity pRarity);

	Item* makeShield(const u8 pLevel, const Rarity pRarity);
	Item* makeCharm(const u8 pLevel, const Rarity pRarity);
	Item* makeRange(const u8 pLevel, const Rarity pRarity);

	// Visible Armor
	Item* makeHead(const u8 pLevel, const Rarity pRarity);
	Item* makeChest(const u8 pLevel, const Rarity pRarity);
	Item* makeArms(const u8 pLevel, const Rarity pRarity);
	Item* makeWrists(const u8 pLevel, const Rarity pRarity);
	Item* makeLegs(const u8 pLevel, const Rarity pRarity);
	Item* makeHands(const u8 pLevel, const Rarity pRarity);
	Item* makeFeet(const u8 pLevel, const Rarity pRarity);

	Item* makeEars(const u8 pLevel, const Rarity pRarity);
	Item* makeRing(const u8 pLevel, const Rarity pRarity);
	Item* makeNeck(const u8 pLevel, const Rarity pRarity);
	Item* makeFace(const u8 pLevel, const Rarity pRarity);
	Item* makeShoulder(const u8 pLevel, const Rarity pRarity);
	Item* makeBack(const u8 pLevel, const Rarity pRarity);
	Item* makeWaist(const u8 pLevel, const Rarity pRarity);

	// Augmentations
	Item* makeAugmentation(const u8 pLevel, const Rarity pRarity);

protected:

	ServiceLocator* mServiceLocator = nullptr;
	ItemFactory* mItemFactory = nullptr;

private:

	bool mInitialised = false;

	Item* _makeBaseArmorItem(const u8 pLevel);
	Item* _makeBaseOneHandWeapon();
	Item* _makeRandom(const u8 pLevel);
	Item* _makeBaseItem();

	void _addAffixes(Item* pItem, const u8 pLevel, const Rarity pRarity);

	const String _getContainerName();
	const u8 _getContainerSlots(const Rarity pRarity);
	const uint32 _getContainerIcon(const Rarity pRarity);
};