#include "ShopDataStore.h"
#include "Data.h"
#include "DataStore.h"
#include "Utility.h"

const bool ShopDataStore::initialise() {
	EXPECTED_BOOL(mInitialised == false);

	Log::status("[ShopDataStore] Initialising.");
	EXPECTED_BOOL(DataStore::getInstance().loadShops(mShopData));
	Log::info("[ShopDataStore] Loaded data for " + std::to_string(mShopData.size()) + " Shops.");

	mInitialised = true;
	return true;
}

ShopData* ShopDataStore::getShopData(const unsigned int pShopID) const {
	for (auto i : mShopData) {
		if (i->mID == pShopID)
			return i;
	}

	return nullptr;
}