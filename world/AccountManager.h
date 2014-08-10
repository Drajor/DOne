#pragma once

#include "Constants.h"

struct AccountData;
class DataStore;

class AccountManager {
public:
	AccountManager();
	~AccountManager();
	bool initialise();

	// Lookup World Account ID associated with Login Server Account ID. 0 = Not found.
	uint32 getWorldAccountID(uint32 pLoginServerAccountID);
	uint32 getStatusFromLoginServerID(uint32 pLoginServerAccountID);
	bool accountExists(uint32 pLoginServerAccountID);
	bool createAccount(uint32 pLoginServerAccountID, String pLoginServerAccountName);
private:
	void _clearAccounts();

	std::list<AccountData*> mAccounts;
};