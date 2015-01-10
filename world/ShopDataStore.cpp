#include "ShopDataStore.h"
#include "ServiceLocator.h"
#include "Data.h"
#include "DataStore.h"
#include "Utility.h"

const bool ShopDataStore::initialise(DataStore* pDataStore) {
	EXPECTED_BOOL(mInitialised == false);
	EXPECTED_BOOL(pDataStore);
	mDataStore = pDataStore;

	Log::status("[ShopDataStore] Initialising.");
	EXPECTED_BOOL(mDataStore->loadShops(mShopData));
	Log::info("[ShopDataStore] Loaded data for " + std::to_string(mShopData.size()) + " Shops.");

	mInitialised = true;
	return true;
}

Data::Shop* ShopDataStore::getShopData(const u32 pID) const {
	for (auto i : mShopData) {
		if (i->mID == pID)
			return i;
	}

	return nullptr;
}
