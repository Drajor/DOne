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
	const bool move(const uint32 pFromSlot, const uint32 pToSlot, const uint32 pStackSize);
	const bool consume(uint32 pSlot);
	const bool pushCursor(Item* pItem);
	inline const bool isCursorEmpty() const { return mCursor.empty(); }
private:
	Item* _popCursor();
	void _set(Item* pItem, const uint32 pSlot);
	Character* mCharacter = nullptr;

	Item* mItems[SlotID::MAIN_SLOTS]; // Slots 0 - 30
	std::queue<Item*> mCursor; // Slot 31
	Item* mBank[SlotID::BANK_SLOTS]; // Slots 2000 - 2023
	Item* mSharedBank[SlotID::SHARED_BANK_SLOTS]; // Slots 2500 - 2501
};