#pragma once

#include "Constants.h"

class NPC;

class LootAllocator {
public:
	void allocate(NPC* pNPC);

	inline const float getCurrencyModifier() const { return mCurrencyModifier; }
	inline const uint32 getDropRate() const { return mDropRate; }
	inline const uint32 getMinimumItems() const { return mMinimumItems; }
	inline const uint32 getMaximumItems() const { return mMaximumItems; }

	inline void setCurrencyModifier(const float pValue) { mCurrencyModifier = pValue; }
	inline void setDropRate(const uint32 pValue) { mDropRate = pValue; }
	inline void setMinimumItems(const uint32 pValue) { mMinimumItems = pValue; }
	inline void setMaximumItems(const uint32 pValue) { mMaximumItems = pValue; }

private:

	void _allocateItems(NPC* pNPC);
	void _allocateCurrency(NPC* pNPC);

	float mCurrencyModifier = 1.0f;
	uint32 mDropRate = 50; // 0-100 chance of an item dropping.
	uint32 mMinimumItems = 2; // Minimum number of random items that can be allocated to an NPC.
	uint32 mMaximumItems = 4; // Maximum number of random items that can be allocated to an NPC.
};