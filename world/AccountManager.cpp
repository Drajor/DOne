#include "AccountManager.h"
#include "DataStore.h"
#include "Data.h"
#include "LogSystem.h"

AccountManager::AccountManager(DataStore* pDataStore) : mDataStore(pDataStore) { }
static bool deleteAll(AccountData* pValue) { delete pValue; return true; };
AccountManager::~AccountManager() {
	_clearAccounts();
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

bool AccountManager::accountExists(uint32 pLoginServerAccountID) {
	for (auto i : mAccounts) {
		if (i->mLoginServerAccountID == pLoginServerAccountID)
			return true;
	}
	return false;
}

bool AccountManager::createAccount(uint32 pLoginServerAccountID, std::string pLoginServerAccountName) {
	if (mDataStore->createAccount(pLoginServerAccountID, pLoginServerAccountName)) {
		// AccountManagers internal accounts are out of date so lets reload them.
		_clearAccounts();
		if (!mDataStore->getAccounts(mAccounts)) {
			// Everything has gone to shit and the world should really just shut down.
			Log::error("[Account Manager] getAccounts failed after a new account creation.");
			return false; // Does not matter what is returned we have no account data now.
		}
		return true;
	}
	return false;
}

void AccountManager::_clearAccounts() {
	mAccounts.remove_if(deleteAll);
	mAccounts.clear();
}
