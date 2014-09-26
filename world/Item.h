#pragma once

#include "Constants.h"
#include "Utility.h"

struct ItemData;
class DynamicStructure;
class Item {
public:
	Item(ItemData* pItemData);
	inline const String& getName() const { return mName; }
	Item* getAugment(const uint8 pSlot) const;
	Item* getContents(const uint8 pSlot) const;

	const uint32 getDataSize() const;
	const bool copyData(Utility::DynamicStructure& pStructure);
	uint32 getSubItems() const;
private:
	const uint32 _getDataSize() const;
	String mName;
	Item* mAugments[5];
	Item* mContents[10];
	ItemData* mItemData = nullptr;
};