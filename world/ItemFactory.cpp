#include "ItemFactory.h"
#include "ServiceLocator.h"
#include "Utility.h"
#include "Item.h"
#include "ItemDataStore.h"

const bool ItemFactory::initialise(ItemDataStore* pItemDataStore) {
	EXPECTED_BOOL(mInitialised == false);
	EXPECTED_BOOL(pItemDataStore);

	mItemDataStore = pItemDataStore;
	mInitialised = true;
	return true;
}

Item* ItemFactory::make(const u32 pItemID, const u32 pStacks) {
	auto data = mItemDataStore->get(pItemID);
	EXPECTED_PTR(data);
	auto item = new Item(data);
	item->setInstanceID(mItemDataStore->getNextSerial());
	item->setStacks(pStacks);
	item->setCharmFile(std::to_string(item->getID()) + "|" + std::to_string(item->getInstanceID()));

	item->setPrice(1234); // TEMP TESTING

	// Instance ID.

	return item;
}

Item* ItemFactory::make(){
	auto data = mItemDataStore->getNew();
	EXPECTED_PTR(data);
	auto item = new Item(data);
	item->setCharmFile(std::to_string(item->getID()) + "|" + std::to_string(item->getInstanceID()));

	item->setPrice(1234); // TEMP TESTING

	return item;
}

Item* ItemFactory::makeAugment() {
	Item* augment = make();
	EXPECTED_PTR(augment);

	augment->setName("Unnamed Augmentation");
	augment->setIcon(2876);
	augment->setAugmentationType(AugmentationSlotType::MultipleStat);
	augment->setAugmentationRestriction(AugmentationRestriction::None);
	augment->setItemType(ItemType::Augmentation);
	augment->setSlots(EquipSlots::VisibleArmor);
	augment->setAugmentationDistiller(ItemID::UniversalDistiller);

	return augment;
}

Item* ItemFactory::copy(Item* pItem) {
	auto item = make(pItem->getID(), pItem->getStacks());

	return item;
}
