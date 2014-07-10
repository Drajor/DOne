#pragma once

#include "../common/types.h"
#include "Data.h"
#include <list>

class DataProvider;

class DataStore {
public:
	DataStore();
	~DataStore();
	void setProvider(DataProvider* pDataProvider);
	void update();

	// Account Data
	bool getAccounts(std::list<AccountData*>& pAccounts);

private:
	DataProvider* mDataProvider; // DataStore is not responsible for deleting.
};