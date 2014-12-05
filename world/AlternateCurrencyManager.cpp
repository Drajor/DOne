#include "AlternateCurrencyManager.h"
#include "Utility.h"
#include "Data.h"
#include "DataStore.h"

const bool AlternateCurrencyManager::initialise() {
	EXPECTED_BOOL(mInitialised == false);

	Log::status("[AlternateCurrencyManager] Initialising.");
	EXPECTED_BOOL(DataStore::getInstance().loadAlternateCurrencies(mCurrencies));
	Log::info("[AlternateCurrencyManager] Loaded data for " + std::to_string(mCurrencies.size()) + " Currencies.");

	mInitialised = true;
	return true;
}

const u32 AlternateCurrencyManager::getItemID(const u32 pCurrencyID) const {
	for (auto i : mCurrencies) {
		if (i->mCurrencyID == pCurrencyID)
			return i->mItemID;
	}
	return 0;
}

const u32 AlternateCurrencyManager::getCurrencyID(const u32 pItemID) const {
	for (auto i : mCurrencies) {
		if (i->mItemID == pItemID)
			return i->mCurrencyID;
	}
	return 0;
}
