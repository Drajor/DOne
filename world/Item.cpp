#include "Item.h"
#include "ItemData.h"

Item::Item(ItemData* pItemData) : mItemData(pItemData) {
	for (auto& i : mAugments) i = nullptr;
	for (auto& i : mContents) i = nullptr;
}

Item* Item::getAugment(const uint8 pSlot) const {
	return mAugments[pSlot];
}

Item* Item::getContents(const uint8 pSlot) const {
	return mContents[pSlot];
}

const uint32 Item::getDataSize() const {
	uint32 result = 0;

	// Add augments.
	for (auto i : mAugments)
		if (i) result += i->getDataSize();

	// Add contents (container)
	for (auto i : mContents)
		if(i) result += i->getDataSize();

	return _getDataSize() + result;
}

const uint32 Item::_getDataSize() const {
	uint32 result = sizeof(ItemData);

	// Remove the maximum size of variable sized attributes.
	result -= sizeof(mItemData->mItemName);
	result -= sizeof(mItemData->mLore);
	result -= sizeof(mItemData->mIDFile);
	result -= sizeof(mItemData->mCharmFile);
	result -= sizeof(mItemData->mFileName);
	result -= sizeof(mItemData->mClickName);
	result -= sizeof(mItemData->mProcName);
	result -= sizeof(mItemData->mWornName);
	result -= sizeof(mItemData->mFocusName);
	result -= sizeof(mItemData->mScrollName);
	result -= sizeof(mItemData->mBardName);

	// Add the real size
	result += strlen(mItemData->mItemName) + 1;
	result += strlen(mItemData->mLore) + 1;
	result += strlen(mItemData->mIDFile) + 1;
	result += strlen(mItemData->mCharmFile) + 1;
	result += strlen(mItemData->mFileName) + 1;
	result += strlen(mItemData->mClickName) + 1;
	result += strlen(mItemData->mProcName) + 1;
	result += strlen(mItemData->mWornName) + 1;
	result += strlen(mItemData->mFocusName) + 1;
	result += strlen(mItemData->mScrollName) + 1;
	result += strlen(mItemData->mBardName) + 1;

	return result;
}

const bool Item::copyData(Utility::DynamicStructure& pStructure) {
	// Write our own data
	pStructure.write<ItemData::P0>(mItemData->mP0);
	pStructure.writeString(String(mItemData->mItemName));
	//pStructure.writeString(String("Test Item"));
	pStructure.writeString(String(mItemData->mLore));
	pStructure.writeString(String(mItemData->mIDFile));
	//pStructure.writeString(String("IT63"));
	pStructure.write<ItemData::P1>(mItemData->mP1);
	pStructure.writeString(String(mItemData->mCharmFile));
	pStructure.write<ItemData::P2>(mItemData->mP2);
	pStructure.writeString(String(mItemData->mFileName));
	pStructure.write<ItemData::P3>(mItemData->mP3);
	pStructure.write<ItemData::ClickEffectStruct>(mItemData->mClickEffectStruct);
	pStructure.writeString(String(mItemData->mClickName));
	pStructure.write<int32>(0); // UNK
	pStructure.write<ItemData::ProcEffectStruct>(mItemData->mProcEffectStruct);
	pStructure.writeString(String(mItemData->mProcName));
	pStructure.write<int32>(0); // UNK

	pStructure.write<ItemData::WornEffectStruct>(mItemData->mWornEffectStruct);
	pStructure.writeString(String(mItemData->mWornName));
	pStructure.write<int32>(0); // UNK

	pStructure.write<ItemData::WornEffectStruct>(mItemData->mFocusEffect);
	pStructure.writeString(String(mItemData->mFocusName));
	pStructure.write<int32>(0); // UNK

	pStructure.write<ItemData::WornEffectStruct>(mItemData->mScrollEffect);
	pStructure.writeString(String(mItemData->mScrollName));
	pStructure.write<int32>(0); // UNK

	pStructure.write<ItemData::WornEffectStruct>(mItemData->mBardEffect);
	pStructure.writeString(String(mItemData->mBardName));
	pStructure.write<int32>(0); // UNK

	pStructure.write<ItemData::ItemQuaternaryBodyStruct>(mItemData->mItemQuaternaryBodyStruct);
	// Write augments

	// Write contents

	return true;
}

uint32 Item::getSubItems() const {
	uint32 count = 0;

	for (auto i : mAugments)
		if(i) count++;
	for (auto i : mContents)
		if(i) count++;

	return count;
}
