#include "Inventory.h"
#include "Item.h"
#include "ItemData.h"
#include "Character.h"
#include "Utility.h"

Inventoryy::Inventoryy(Character* pCharacter) : mCharacter(pCharacter) {
	for (auto& i : mItems) i = nullptr;

	auto itemData = new ItemData();
	itemData->mP1.mID = 2;
	itemData->mP0.mInstanceID = 2;
	itemData->mP0.mSlot = 23;
	itemData->mP1.ItemType = 17;
	itemData->mP1.mClasses = 65535;
	itemData->mP1.mIcon = 639;
	strcpy(itemData->mItemName, String("Test Item").c_str());
	strcpy(itemData->mIDFile, String("IT63").c_str());

	auto item = new Item(itemData);
	put(item, 23);
	
}

const unsigned char* Inventoryy::getData(uint32& pSize) {
	unsigned char * data = nullptr;
	uint32 numItems = 0;

	// First we need to calculate how much memory will be required to store the inventory data.

	// Inventory
	for (auto i = 0; i < 32; i++) {
		auto item = getItem(i);
		if (item) {
			numItems++;
			pSize += item->getDataSize();
			numItems += item->getSubItems();
		}
			
	}

	// Bank Items

	// Shared Bank Items.

	// Allocate the required memory.
	pSize += sizeof(uint32); // Item Count
	data = new unsigned char[pSize];

	Utility::DynamicStructure ds(data, pSize);
	ds.write<uint32>(numItems);

	// Second we need to copy

	// Inventory.
	for (auto i = 0; i < 32; i++) {
		auto item = getItem(i);
		if (item)
			item->copyData(ds);
	}

	// Bank Items

	// Shared Bank Items.

	if (ds.check() == false) Log::info("badcheck");

	return data;
}

Item* Inventoryy::getItem(const uint32 pSlot) const {
	return mItems[pSlot];
}

const bool Inventoryy::put(Item* pItem, const uint32 pSlot) {
	mItems[pSlot] = pItem;
	return true;
}
