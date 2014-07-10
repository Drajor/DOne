#pragma once

#include "../common/types.h"
#include <string>
#include <list>

struct AccountData;
class DataStore;

class AccountManager {
public:
	AccountManager(DataStore* pDataStore);
	~AccountManager();
	bool initialise();

	// Lookup World Account ID associated with Login Server Account ID. 0 = Not found.
	uint32 getWorldAccountID(uint32 pLoginServerAccountID);
	uint32 getStatusFromLoginServerID(uint32 pLoginServerAccountID);
private:

	std::list<AccountData*> mAccounts;
	DataStore* mDataStore;
};