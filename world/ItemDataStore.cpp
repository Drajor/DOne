#include "ItemDataStore.h"
#include "Utility.h"
#include "Item.h"

ItemDataStore::ItemDataStore() {
	mItemData.resize(1000000);
}

const bool ItemDataStore::initialise() {
	EXPECTED_BOOL(mInitialised == false);

	_bootstrap();

	return (mInitialised = true);
}

void ItemDataStore::_bootstrap() {

	// Augmentation Sealer.
	{
		auto data = &mItemData[1];
		data->mID = 1;
		auto item = new Item(data);
		
		item->setName("Augmentation Sealer");
		item->setIDFile("IT63");
		item->setIcon(1142);
		item->setItemClass(ItemClass::Container);
		item->setContainerType(ContainerType::AugmentationSealer);
		item->setContainerSlots(2);
		item->setContainerSize(ContainerSize::Giant);

		delete item;
	}
	
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
