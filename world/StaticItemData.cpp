//#include "StaticItemData.h"
//#include "Utility.h"
//#include "Profile.h"
//#include "DataStore.h"
//
//StaticItemData::StaticItemData() { }
//
//StaticItemData::~StaticItemData() { }
//
//const bool StaticItemData::initialise() {
//	EXPECTED_BOOL(mInitialised == false);
//
//	Profile p("StaticItemData::initialise");
//	Log::status("[Static Item Data] Initialising.");
//
//	// Allocate block of memory for items.
//	mItemData = new ItemData[Limits::StaticItems::MAX_ITEM_ID];
//	memset(mItemData, 0, sizeof(mItemData));
//
//	// DataStore to load and populate.
//	uint32 numItemsLoaded = 0;
//	EXPECTED_BOOL(DataStore::getInstance().loadItems(mItemData, numItemsLoaded));
//
//	Log::info("[Static Item Data] Loaded data for " + std::to_string(numItemsLoaded) + " Items.");
//	mInitialised = true;
//	return true;
//}
