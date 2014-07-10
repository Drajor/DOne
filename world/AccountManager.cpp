#include "AccountManager.h"
#include "DataStore.h"
#include "Data.h"

AccountManager::AccountManager(DataStore* pDataStore) : mDataStore(pDataStore) { }

AccountManager::~AccountManager() {
	for (auto i : mAccounts)
		delete i;

	mAccounts.clear();
}


bool AccountManager::initialise() { return mDataStore->getAccounts(mAccounts); }

uint32 AccountManager::getWorldAccountID(uint32 pLoginServerAccountID) {
	for (auto i : mAccounts) {
		if (i->mLoginServerAccountID == pLoginServerAccountID)
			return i->mWorldAccountID;
	}
	return 0;
}

uint32 AccountManager::getStatusFromLoginServerID(uint32 pLoginServerAccountID) {
	for (auto i : mAccounts) {
		if (i->mLoginServerAccountID == pLoginServerAccountID)
			return i->mStatus;
	}
	return 0;
}
