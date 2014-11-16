#pragma once

#include "Constants.h"

class Character;
class Item;
class Inventoryy { // We get an extra y for now.

public:
	Inventoryy(Character* pCharacter);

	// Puts pItem at pSlot. Excluding the cursor, pSlot is expected to be empty (nullptr).
	const bool put(Item* pItem, const uint32 pSlot);
	const bool putContainer(Item* pItem, const uint32 pSlot);

	// Returns the Item at pSlot or nullptr.
	Item* getItem(const uint32 pSlot) const;

	const unsigned char* getData(uint32& pSize); // Caller responsible for delete.
	const bool move(const uint32 pFromSlot, const uint32 pToSlot, const uint32 pStackSize);
	const bool consume(const uint32 pSlot, const uint32 pStacks);
	const bool pushCursor(Item* pItem);
	inline const bool isCursorEmpty() const { return mCursor.empty(); }

	void _calculateAdd(Item* pItem);
	void _calculateRemove(Item* pItem);

	inline const int32 getStrength() const { return mStrength; }
	inline const int32 getStamina() const { return mStamina; }
	inline const int32 getIntelligence() const { return mIntelligence; }
	inline const int32 getWisdom() const { return mWisdom; }
	inline const int32 getAgility() const { return mAgility; }
	inline const int32 getDexterity() const { return mDexterity; }
	inline const int32 getCharisma() const { return mCharisma; }

	inline const int32 getHeroicStrength() const { return mHeroicStrength; }
	inline const int32 getHeroicStamina() const { return mHeroicStamina; }
	inline const int32 getHeroicIntelligence() const { return mHeroicIntelligence; }
	inline const int32 getHeroicWisdom() const { return mHeroicWisdom; }
	inline const int32 getHeroicAgility() const { return mHeroicAgility; }
	inline const int32 getHeroicDexterity() const { return mHeroicDexterity; }
	inline const int32 getHeroicCharisma() const { return mHeroicCharisma; }

	inline const int32 getMagicResist() const { return mMagicResist; }
	inline const int32 getFireResist() const { return mFireResist; }
	inline const int32 getColdResist() const { return mColdResist; }
	inline const int32 getDiseaseResist() const { return mDiseaseResist; }
	inline const int32 getPoisonResist() const { return mPoisonResist; }
	inline const int32 getCorruptionResist() const { return mCorruptionResist; }

	inline const int32 getHeroicMagicResist() const { return mHeroicMagicResist; }
	inline const int32 getHeroicFireResist() const { return mHeroicFireResist; }
	inline const int32 getHeroicColdResist() const { return mHeroicColdResist; }
	inline const int32 getHeroicDiseaseResist() const { return mHeroicDiseaseResist; }
	inline const int32 getHeroicPoisonResist() const { return mHeroicPoisonResist; }
	inline const int32 getHeroicCorruptionResist() const { return mHeroicCorruptionResist; }

	inline const int32 getHealth() const { return mHealth; }
	inline const int32 getMana() const { return mMana; }
	inline const int32 getEndurance() const { return mEndurance; }

	inline const int32 getHealthRegen() const { return mHealthRegen; }
	inline const int32 getManaRegen() const { return mManaRegen; }
	inline const int32 getEnduranceRegen() const { return mEnduranceRegen; }

	inline Item* peekCursor() const { return _peekCursor(); }

private:
	Item* _popCursor();
	Item* _peekCursor() const;
	const bool _clear(const uint32 pSlot);
	const bool _clearContainerSlot(const uint32 pSlot);

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

	int32 mStrength = 0;
	int32 mStamina = 0;
	int32 mIntelligence = 0;
	int32 mWisdom = 0;
	int32 mAgility = 0;
	int32 mDexterity = 0;
	int32 mCharisma = 0;

	int32 mHeroicStrength = 0;
	int32 mHeroicStamina = 0;
	int32 mHeroicIntelligence = 0;
	int32 mHeroicWisdom = 0;
	int32 mHeroicAgility = 0;
	int32 mHeroicDexterity = 0;
	int32 mHeroicCharisma = 0;

	int32 mMagicResist = 0;
	int32 mFireResist = 0;
	int32 mColdResist = 0;
	int32 mDiseaseResist = 0;
	int32 mPoisonResist = 0;
	int32 mCorruptionResist = 0;

	int32 mHeroicMagicResist = 0;
	int32 mHeroicFireResist = 0;
	int32 mHeroicColdResist = 0;
	int32 mHeroicDiseaseResist = 0;
	int32 mHeroicPoisonResist = 0;
	int32 mHeroicCorruptionResist = 0;

	int32 mHealth = 0;
	int32 mMana = 0;
	int32 mEndurance = 0;

	int32 mHealthRegen = 0;
	int32 mManaRegen = 0;
	int32 mEnduranceRegen = 0;
};