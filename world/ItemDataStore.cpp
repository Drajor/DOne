#include "ItemDataStore.h"
#include "IDataStore.h"

#include "Utility.h"
#include "Item.h"

ItemDataStore::ItemDataStore() {
	mItemData.resize(1000000);
}

ItemDataStore::~ItemDataStore() {
	mDataStore = nullptr;

	if (mLog) {
		delete mLog;
		mLog = nullptr;
	}
	
	mItemData.clear();
}

const bool ItemDataStore::initialise(IDataStore* pDataStore, ILogFactory* pLogFactory) {
	if (mInitialised) return false;
	if (!pDataStore) return false;
	if (!pLogFactory) return false;

	mDataStore = pDataStore;
	mLog = pLogFactory->make();

	mLog->setContext("[ItemDataStore]");
	mLog->status("Initialising.");

	_bootstrap();

	mLog->status("Finished initialising.");
	mInitialised = true;
	return true;
}

void ItemDataStore::_bootstrap() {
	// Radiant Crystal.
	{
		auto data = &mItemData[ItemID::RadiantCrystal];
		data->mID = ItemID::RadiantCrystal;
		auto item = new Item(data);

		item->setName("Radiant Crystal");
		item->setIcon(1536);
		item->setItemType(ItemType::AlternateCurrency);
		item->setMaxStacks(MaxRadiantCrystalsStacks);

		delete item;
	}
	// Ebon Crystal.
	{
		auto data = &mItemData[ItemID::EbonCrystal];
		data->mID = ItemID::EbonCrystal;
		auto item = new Item(data);

		item->setName("Ebon Crystal");
		item->setIcon(1535);
		item->setItemType(ItemType::AlternateCurrency);
		item->setMaxStacks(MaxEbonCrystalsStacks);

		delete item;
	}
	// Augmentation Sealer.
	{
		auto data = &mItemData[ItemID::AugmentationSealer];
		data->mID = ItemID::AugmentationSealer;
		auto item = new Item(data);

		item->setName("Augmentation Sealer");
		item->setIcon(1142);
		item->setItemClass(ItemClass::Container);
		item->setContainerType(ContainerType::AugmentationSealer);
		item->setContainerSlots(2);
		item->setContainerSize(ContainerSize::Giant);

		delete item;
	}
	// Universal Distiller.
	{
		auto data = &mItemData[ItemID::UniversalDistiller];
		data->mID = ItemID::UniversalDistiller;
		auto item = new Item(data);

		item->setName("Universal Distiller");
		item->setIcon(2267);
		item->setItemType(ItemType::AugmentationDistiller);
		item->setItemClass(ItemClass::Common);
		item->setMaxStacks(10000);

		delete item;
	}
	// Universal Solvent.
	{
		auto data = &mItemData[ItemID::UniversalSolvent];
		data->mID = ItemID::UniversalSolvent;
		auto item = new Item(data);

		item->setName("Universal Solvent");
		item->setIcon(2268);
		item->setItemType(ItemType::AugmentationSolvent);
		item->setItemClass(ItemClass::Common);
		item->setMaxStacks(10000);

		delete item;
	}
	// Start Container
	{
		auto data = &mItemData[ItemID::StartContainer];
		data->mID = ItemID::StartContainer;
		auto item = new Item(data);

		item->setName("Backpack");
		item->setIcon(884);
		item->setItemClass(ItemClass::Container);
		item->setContainerType(ContainerType::Normal);
		item->setContainerSlots(10);
		item->setContainerSize(ContainerSize::Giant);
		item->setIsNoDrop(true);

		delete item;
	}
	// Start Food.
	{
		auto data = &mItemData[ItemID::StartFood];
		data->mID = ItemID::StartFood;
		auto item = new Item(data);

		item->setName("Bread");
		item->setIcon(537);
		item->setItemType(ItemType::Food);
		item->setMaxStacks(10000);
		item->setFoodSize(10);
		item->setIsNoDrop(true);

		delete item;
	}
	// Start Drink.
	{
		auto data = &mItemData[ItemID::StartDrink];
		data->mID = ItemID::StartDrink;
		auto item = new Item(data);

		item->setName("Water");
		item->setIcon(584);
		item->setItemType(ItemType::Drink);
		item->setMaxStacks(10000);
		item->setDrinkSize(10);
		item->setIsNoDrop(true);

		delete item;
	}
	// Transmuter.
	{
		auto data = &mItemData[ItemID::TransmuterTen];
		data->mID = ItemID::TransmuterTen;
		auto item = new Item(data);

		item->setName("Transmuter");
		item->setIcon(2304);
		item->setItemClass(ItemClass::Container);
		item->setContainerType(ContainerType::Quest);
		item->setContainerSlots(10);
		item->setContainerSize(ContainerSize::Giant);
		item->setIsNoDrop(true);

		delete item;
	}
	// Test Container
	{
		auto data = &mItemData[ItemID::TestContainer];
		data->mID = ItemID::TestContainer;
		auto item = new Item(data);

		item->setName("Test Container");
		item->setIcon(2304);
		item->setItemClass(ItemClass::Container);
		item->setContainerType(ContainerType::Normal);
		item->setContainerSlots(10);
		item->setContainerSize(ContainerSize::Giant);
	}
	// Token
	{
		auto data = &mItemData[ItemID::Token];
		data->mID = ItemID::Token;
		auto item = new Item(data);

		item->setName("Token");
		item->setIcon(3307);
		item->setItemType(ItemType::AlternateCurrency);
		item->setMaxStacks(MaxTokensStacks);
		item->setIsNoDrop(true);

		delete item;
	}
	// Adhesive
	{
		auto data = &mItemData[ItemID::Adhesive];
		data->mID = ItemID::Adhesive;
		auto item = new Item(data);

		item->setName("Adhesive");
		item->setIcon(2768);
		item->setItemType(ItemType::AlternateCurrency);
		item->setMaxStacks(MaxAdhesiveStacks);
		item->setIsTradeskillsItem(true);

		delete item;
	}

	EXPECTED(_TransmutationComponentTest(ItemID::TCStrength, "TC Strength"));
	EXPECTED(_TransmutationComponentTest(ItemID::TCStamina, "TC Stamina"));
	EXPECTED(_TransmutationComponentTest(ItemID::TCAgility, "TC Agility"));
	EXPECTED(_TransmutationComponentTest(ItemID::TCDexterity, "TC Dexterity"));
	EXPECTED(_TransmutationComponentTest(ItemID::TCWisdom, "TC Wisdom"));
	EXPECTED(_TransmutationComponentTest(ItemID::TCIntelligence, "TC Intelligence"));
	EXPECTED(_TransmutationComponentTest(ItemID::TCCharisma, "TC Charisma"));

	EXPECTED(_TransmutationComponentTest(ItemID::TCPoisonResist, "TC Poison Resist"));
	EXPECTED(_TransmutationComponentTest(ItemID::TCMagicResist, "TC Magic Resist"));
	EXPECTED(_TransmutationComponentTest(ItemID::TCDiseaseResist, "TC Disease Resist"));
	EXPECTED(_TransmutationComponentTest(ItemID::TCFireResist, "TC Fire Resist"));
	EXPECTED(_TransmutationComponentTest(ItemID::TCColdResist, "TC Cold Resist"));
	EXPECTED(_TransmutationComponentTest(ItemID::TCCorruptionResist, "TC Corruption Resist"));
	//// Transmutation Test Zero
	//{
	//	auto data = &mItemData[ItemID::TestComponentZero];
	//	data->mID = ItemID::TestComponentZero;
	//	auto item = new Item(data);

	//	item->setName("TC Strength");
	//	item->setIcon(666);
	//	item->setItemType(ItemType::Miscellaneous);
	//	item->setMaxStacks(100);

	//	delete item;
	//}
	//// Transmutation Test One
	//{
	//	auto data = &mItemData[ItemID::TestComponentOne];
	//	data->mID = ItemID::TestComponentOne;
	//	auto item = new Item(data);

	//	item->setName("Sea Shells");
	//	item->setIcon(998);
	//	item->setItemType(ItemType::Miscellaneous);
	//	item->setMaxStacks(100);

	//	delete item;
	//}


	//// Test - Elemental Damage Augmentations
	//int id = 4;
	//for (auto i = 1; i < 20; i++) {
	//	auto data = &mItemData[id];
	//	data->mID = id;
	//	auto item = new Item(data);

	//	item->setName("Elemental Damage T(" + std::to_string(i)+")");
	//	item->setIDFile("IT63");
	//	item->setIcon(2187);
	//	item->setItemType(ItemType::Augmentation);
	//	item->setItemClass(ItemClass::Common);
	//	item->setElementalDamageType(i);
	//	item->setElementalDamage(7);
	//	item->setSlots(EquipSlots::PrimarySecondary);

	//	item->setAugmentationType(AugmentationSlotType::MultipleStat);
	//	item->setAugmentationRestriction(AugmentationRestriction::None);
	//	item->setAugmentationDistiller(2);

	//	delete item;
	//	id++;
	//}
	
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

const bool ItemDataStore::_TransmutationComponentTest(const uint32 pItemID, const String& pName) {
	auto data = &mItemData[pItemID];
	data->mID = pItemID;
	auto item = new Item(data);

	item->setName(pName);
	item->setIcon(666);
	item->setItemType(ItemType::Miscellaneous);
	item->setMaxStacks(100);
	item->setPrice(1000);

	delete item;

	return true;
}
