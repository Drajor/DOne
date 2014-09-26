#pragma once

#include "Constants.h"
class Character;
class Item;
class Inventoryy { // We get an extra y for now.
public:
	Inventoryy(Character* pCharacter);
	const bool put(Item* pItem, const uint32 pSlot);
	Item* getItem(const uint32 pSlot) const;
	const unsigned char* getData(uint32& pSize); // Caller responsible for delete.
private:
	Character* mCharacter = nullptr;

	Item* mItems[32];
};