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
	static Item* makeRandom(const uint8 pLevel);
	static Item* makeRandomContainer(const Rarity pRarity);
	static Item* makeFood();
	static Item* makeDrink();
	static Item* makePowerSource(const uint8 pLevel, const Rarity pRarity);
	static Item* makeOneHandBlunt(const uint8 pLevel, const Rarity pRarity);
	static Item* makeShield(const uint8 pLevel, const Rarity pRarity);

	static Item* makeHead(const uint8 pLevel, const Rarity pRarity);
	//static Item* makeChest();
private:
	Item* _makeRandom(const uint8 pLevel);
	Item* _makeBaseItem();

	static const String _getContainerName();
	static const uint8 _getContainerSlots(const Rarity pRarity);
	static const uint32 _getContainerIcon(const Rarity pRarity);
	
	uint32 mNextID = 10000;
	const uint32 getNextID();
	uint32 mNextSerial = 1;
	const uint32 getNextSerial();
};