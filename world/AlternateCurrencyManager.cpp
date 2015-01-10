#include "AlternateCurrencyManager.h"
#include "ServiceLocator.h"
#include "IDataStore.h"
#include "Utility.h"
#include "Data.h"

AlternateCurrencyManager::AlternateCurrencyManager() {
	mLog = new LogContext("[AlternateCurrencyManager]");
}

AlternateCurrencyManager::~AlternateCurrencyManager() {
	delete mLog;
	mLog = nullptr;
}

const bool AlternateCurrencyManager::initialise(IDataStore* pDataStore) {
	mLog->status("Initialising.");
	EXPECTED_BOOLX(mInitialised == false, mLog);
	EXPECTED_BOOLX(pDataStore, mLog);

	mDataStore = pDataStore;
	EXPECTED_BOOLX(mDataStore->loadAlternateCurrencies(mCurrencies), mLog);

	mInitialised = true;

	mLog->info("Loaded data for " + std::to_string(mCurrencies.size()) + " Currencies.");
	mLog->status("Finished initialising.");
	return true;
}

const u32 AlternateCurrencyManager::getItemID(const u32 pCurrencyID) const {
	for (auto i : mCurrencies) {
		if (i->mCurrencyID == pCurrencyID)
			return i->mItemID;
	}

	mLog->error("getItemID");
	return 0;
}

const u32 AlternateCurrencyManager::getCurrencyID(const u32 pItemID) const {
	for (auto i : mCurrencies) {
		if (i->mItemID == pItemID)
			return i->mCurrencyID;
	}

	mLog->error("getCurrencyID");
	return 0;
}
