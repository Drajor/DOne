#include "ItemFactory.h"
#include "ItemDataStore.h"
#include "LogSystem.h"
#include "Item.h"
#include "Constants.h"

ItemFactory::~ItemFactory() {
	mItemDataStore = nullptr;

	if (mLog) {
		delete mLog;
		mLog = nullptr;
	}
}

const bool ItemFactory::initialise(ItemDataStore* pItemDataStore, ILogFactory* pLogFactory) {
	if (mInitialised) return false;
	if (!pItemDataStore) return false;
	if (!pLogFactory) return false;

	mItemDataStore = pItemDataStore;
	mLog = pLogFactory->make();

	mLog->setContext("[ItemFactory]");
	mLog->status("Initialising.");

	mLog->status("Finished initialising.");
	mInitialised = true;
	return true;
}

Item* ItemFactory::make(const u32 pItemID, const u32 pStacks) {
	auto data = mItemDataStore->get(pItemID);
	if (!data) return nullptr;

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
	if (!data) return nullptr;

	auto item = new Item(data);
	item->setCharmFile(std::to_string(item->getID()) + "|" + std::to_string(item->getInstanceID()));

	item->setPrice(1234); // TEMP TESTING

	return item;
}

Item* ItemFactory::makeAugment() {
	Item* augment = make();
	if (!augment) return false;

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
