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
	// Universal Distiller.
	{
		auto data = &mItemData[2];
		data->mID = 2;
		auto item = new Item(data);

		item->setName("Universal Distiller");
		item->setIDFile("IT63");
		item->setIcon(2267);
		item->setItemType(ItemType::AugmentationDistiller);
		item->setItemClass(ItemClass::Common);
		item->setMaxStacks(10000);

		delete item;
	}
	// Universal Solvent.
	{
		auto data = &mItemData[3];
		data->mID = 3;
		auto item = new Item(data);

		item->setName("Universal Solvent");
		item->setIDFile("IT63");
		item->setIcon(2268);
		item->setItemType(ItemType::AugmentationSolvent);
		item->setItemClass(ItemClass::Common);
		item->setMaxStacks(10000);

		delete item;
	}

	// Test - Elemental Damage Augmentations
	int id = 4;
	for (auto i = 1; i < 20; i++) {
		auto data = &mItemData[id];
		data->mID = id;
		auto item = new Item(data);

		item->setName("Elemental Damage T(" + std::to_string(i)+")");
		item->setIDFile("IT63");
		item->setIcon(2187);
		item->setItemType(ItemType::Augmentation);
		item->setItemClass(ItemClass::Common);
		item->setElementalDamageType(i);
		item->setElementalDamage(7);
		item->setSlots(EquipSlots::PrimarySecondary);

		item->setAugmentationType(AugmentationSlotType::MultipleStat);
		item->setAugmentationRestriction(AugmentationRestriction::None);
		item->setAugmentationDistiller(2);

		delete item;
		id++;
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
