#include "ShopDataStore.h"
#include "Data.h"
#include "IDataStore.h"
#include "LogSystem.h"

ShopDataStore::~ShopDataStore() {
	if (mLog) {
		delete mLog;
		mLog = nullptr;
	}

	for (auto i : mData) {
		delete i;
	}
	mData.clear();
}

const bool ShopDataStore::initialise(IDataStore* pDataStore, ILogFactory* pLogFactory) {
	if (mInitialised) return false;
	if (!pDataStore) return false;
	if (!pLogFactory) return false;

	mDataStore = pDataStore;
	mLog = pLogFactory->make();

	mLog->setContext("[ShopDataStore]");
	mLog->status("Initialising.");

	// Load data.
	if (!mDataStore->loadShops(mData)) {
		mLog->error("Failed to load data.");
		return false;
	}
	mLog->info("[ShopDataStore] Loaded data for " + toString(mData.size()) + " Shops.");

	mLog->status("Finished initialising.");
	mInitialised = true;
	return true;
}

Data::Shop* ShopDataStore::getShopData(const u32 pID) const {
	for (auto i : mData) {
		if (i->mID == pID)
			return i;
	}

	return nullptr;
}
