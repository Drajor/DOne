#include "Item.h"
#include "ItemData.h"
#include <iostream>

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
	result -= sizeof(mItemData->mName);
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
	result += strlen(mItemData->mName) + 1;
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
	// Update ItemData with anything specific from this Item
	_onCopy();

	// Chunk Zero.
	std::size_t chunk0 = (unsigned int)&(mItemData->mName) - (unsigned int)&(mItemData->mStacks);
	pStructure.writeChunk((void*)&(mItemData->mStacks), chunk0);

	// Variable.
	pStructure.writeString(String(mItemData->mName));
	pStructure.writeString(String(mItemData->mLore));
	pStructure.writeString(String(mItemData->mIDFile));

	// Chunk One.
	std::size_t chunk1 = (unsigned int)&(mItemData->mCharmFile) - (unsigned int)&(mItemData->mID);
	pStructure.writeChunk((void*)&(mItemData->mID), chunk1);

	// Variable.
	pStructure.writeString(String(mItemData->mCharmFile));
	
	// Chunk Two.
	std::size_t chunk2 = (unsigned int)&(mItemData->mFileName) - (unsigned int)&(mItemData->mAugType);
	pStructure.writeChunk((void*)&(mItemData->mAugType), chunk2);
	
	// Variable.
	pStructure.writeString(String(mItemData->mFileName));

	// Chunk Three.
	std::size_t chunk3 = (unsigned int)&(mItemData->mClickEffect) - (unsigned int)&(mItemData->mLoreGroup);
	pStructure.writeChunk((void*)&(mItemData->mLoreGroup), chunk3);
	
	// Click Effect.
	pStructure.write<ItemData::ClickEffect>(mItemData->mClickEffect);
	pStructure.writeString(String(mItemData->mClickName));
	pStructure.write<int32>(mItemData->mClickUnknown);
	
	// Proc Effect.
	pStructure.write<ItemData::ProcEffectStruct>(mItemData->mProcEffect);
	pStructure.writeString(String(mItemData->mProcName));
	pStructure.write<int32>(mItemData->mProcUnknown);

	// Worn Effect.
	pStructure.write<ItemData::Effect>(mItemData->mWornEffect);
	pStructure.writeString(String(mItemData->mWornName));
	pStructure.write<int32>(mItemData->mWornUnknown);

	// Focus Effect.
	pStructure.write<ItemData::Effect>(mItemData->mFocusEffect);
	pStructure.writeString(String(mItemData->mFocusName));
	pStructure.write<int32>(mItemData->mWornUnknown);

	// Scroll Effect.
	pStructure.write<ItemData::Effect>(mItemData->mScrollEffect);
	pStructure.writeString(String(mItemData->mScrollName));
	pStructure.write<int32>(mItemData->mScrollUnknown);

	// Bard Effect.
	pStructure.write<ItemData::Effect>(mItemData->mBardEffect);
	pStructure.writeString(String(mItemData->mBardName));
	pStructure.write<int32>(mItemData->mBardUnknown);

	// Chunk Four.
	std::size_t chunk4 = (unsigned int)&(mItemData->subitem_count) - (unsigned int)&(mItemData->mScriptFileID);
	pStructure.writeChunk((void*)&(mItemData->mScriptFileID), chunk4);

	// Child Items.
	pStructure.write<uint32>(getSubItems());

	// Write augments
	for (auto i : mAugments)
		if (i) EXPECTED_BOOL(i->copyData(pStructure));		

	// Write contents
	for (auto i : mContents)
		if (i) EXPECTED_BOOL(i->copyData(pStructure));

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

void Item::_onCopy() {
	mItemData->mSlot = mSlot;
	mItemData->mStacks = mStacks;
	mItemData->mAttuned = mAttuned ? 1 : 0;
	// NOTE: When an Item has augments, those augments are sent with the same slot ID as the parent item.
}

Item* Item::findFirst(const uint8 pItemType) {
	for (auto i : mContents) {
		if (i && i->getItemType() == pItemType)
			return i;
	}

	return nullptr;
}
