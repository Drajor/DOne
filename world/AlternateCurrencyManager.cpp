#include "AlternateCurrencyManager.h"
#include "Data.h"
#include "IDataStore.h"
#include "LogSystem.h"

AlternateCurrencyManager::~AlternateCurrencyManager() {
	if (mLog) {
		delete mLog;
		mLog = nullptr;
	}

	for (auto i : mData) {
		delete i;
	}
	mData.clear();
}

const bool AlternateCurrencyManager::initialise(IDataStore* pDataStore, ILogFactory* pLogFactory) {
	if (mInitialised) return false;
	if (!pLogFactory) return false;
	if (!pDataStore) return false;

	mDataStore = pDataStore;
	mLog = pLogFactory->make();

	mLog->setContext("[AlternateCurrencyManager]");
	mLog->status("Initialising.");

	// Load data.
	if (!mDataStore->loadAlternateCurrencies(mData)) {
		mLog->error("Failed to load data.");
		return false;
	}
	mLog->info("Loaded data for " + toString(mData.size()) + " Currencies.");

	mLog->status("Finished initialising.");
	mInitialised = true;
	return true;
}

const u32 AlternateCurrencyManager::getItemID(const u32 pCurrencyID) const {
	for (auto i : mData) {
		if (i->mCurrencyID == pCurrencyID)
			return i->mItemID;
	}

	mLog->error("getItemID");
	return 0;
}

const u32 AlternateCurrencyManager::getCurrencyID(const u32 pItemID) const {
	for (auto i : mData) {
		if (i->mItemID == pItemID)
			return i->mCurrencyID;
	}

	mLog->error("getCurrencyID");
	return 0;
}
