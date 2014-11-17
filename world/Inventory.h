#pragma once

#include "Constants.h"
#include "Bonuses.h"

class Character;
class Item;
class Inventoryy : public Bonuses { // We get an extra y for now.

public:
	Inventoryy(Character* pCharacter);

	// Items

	// Puts pItem at pSlot. Excluding the cursor, pSlot is expected to be empty (nullptr).
	const bool put(Item* pItem, const uint32 pSlot);
	const bool putContainer(Item* pItem, const uint32 pSlot);

	// Returns the Item at pSlot or nullptr.
	Item* getItem(const uint32 pSlot) const;

	Item* find(const uint32 pItemID, const uint32 pInstanceID) const;

	const unsigned char* getData(uint32& pSize); // Caller responsible for delete.
	const bool move(const uint32 pFromSlot, const uint32 pToSlot, const uint32 pStackSize);
	const bool consume(const uint32 pSlot, const uint32 pStacks);
	const bool pushCursor(Item* pItem);
	inline const bool isCursorEmpty() const { return mCursor.empty(); }

	inline Item* peekCursor() const { return _peekCursor(); }

	// Currency

	// Alternate Currency

	// Radiant Crystals.
	inline const uint32 getRadiantCrystals() const { return mRadiantCrystals; }
	inline const uint32 getTotalRadiantCrystals() const { return mTotalRadiantCrystals; }
	inline void addRadiantCrystals(const uint32 pCrystals) { mRadiantCrystals += pCrystals; mTotalRadiantCrystals += pCrystals; }
	const bool removeRadiantCrystals(const uint32 pCrystals);
	
	// Sets the number of Radiant Crystals. Should only be used during initialisation.
	inline void _setRadiantCrystals(const uint32 pCurrent, const uint32 pTotal) { mRadiantCrystals = pCurrent; mTotalRadiantCrystals = pTotal; }

	// Ebon Crystals.
	inline const uint32 getEbonCrystals() const { return mEbonCrystals; }
	inline const uint32 getTotalEbonCrystals() const { return mTotalEbonCrystals; }
	inline void addEbonCrystals(const uint32 pCrystals) { mEbonCrystals += pCrystals; mTotalEbonCrystals += pCrystals; }
	const bool removeEbonCrystals(const uint32 pCrystals);

	// Sets the number of Ebon Crystals. Should only be used during initialisation.
	inline void _setEbonCrystals(const uint32 pCurrent, const uint32 pTotal) { mEbonCrystals = pCurrent; mTotalEbonCrystals = pTotal; }

private:
	Item* _popCursor();
	Item* _peekCursor() const;
	const bool _clear(const uint32 pSlot);
	const bool _clearContainerSlot(const uint32 pSlot);

	void _calculateAdd(Item* pItem);
	void _calculateRemove(Item* pItem);

	Character* mCharacter = nullptr;

	Item* mItems[SlotID::MAIN_SLOTS]; // Slots 0 - 30
	std::queue<Item*> mCursor; // Slot 31
	Item* mBank[SlotID::BANK_SLOTS]; // Slots 2000 - 2023
	Item* mSharedBank[SlotID::SHARED_BANK_SLOTS]; // Slots 2500 - 2501
	Item* mTrade[SlotID::TRADE_SLOTS]; // Slots 3000 - ?

	void updateConsumables();
	Item* findFirst(const uint8 pItemType);
	
	const bool _putDown(const uint32 pToSlot, const uint32 pStackSize);
	const bool _stackMergeCursor(const uint32 pToSlot, const uint32 pStackSize);
	const bool _pickUp(const uint32 pFromSlot, const uint32 pStackSize);
	

	Item* mAutoFood = nullptr;
	Item* mAutoDrink = nullptr;

	// Radiant Crystals
	uint32 mRadiantCrystals = 0;
	uint32 mTotalRadiantCrystals = 0;
	
	// Ebon Crystals
	uint32 mEbonCrystals = 0;
	uint32 mTotalEbonCrystals = 0;
};