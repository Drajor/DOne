#include "ItemDataStore.h"
#include "Utility.h"

ItemDataStore::ItemDataStore() {
	mItemData.resize(1000000);
}

ItemData* ItemDataStore::getNew() {
	auto itemData = _getNext();
	itemData->mID = getNextID();
	itemData->mInstanceID = getNextSerial();

	return itemData;
}

const uint32 ItemDataStore::getNextID() {
	auto nextID = mNextID;
	mNextID++;
	return nextID;
}

const uint32 ItemDataStore::getNextSerial() {
	mNextSerial++;
	return mNextSerial;
}

ItemData* ItemDataStore::_getNext() {
	// Check whether we need to allocate more memory.
	if (mItemData.size() == mNextID) {
		mItemData.resize(mItemData.size() + 1000);
	}

	EXPECTED_PTR(mItemData[mNextID].mID == 0); // Sanity Check: Not already 'in use'.
	return &mItemData[mNextID];
}

ItemData* ItemDataStore::get(const uint32 pID) {
	EXPECTED_PTR(pID < mItemData.size());
	ItemData* data = &mItemData[pID];
	EXPECTED_PTR(data->mID != 0); // Sanity Check: Returning an 'in use' ItemData.

	return data;
}
