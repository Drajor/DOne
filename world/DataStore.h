#pragma once

#include "../common/types.h"
#include "Data.h"
#include <list>

class DataProvider;
struct CharacterSelect_Struct;

class DataStore {
public:
	DataStore();
	~DataStore();
	void setProvider(DataProvider* pDataProvider);
	void update();

	// Account Data
	bool getAccounts(std::list<AccountData*>& pAccounts);
	bool getCharacterSelectInfo(uint32 pWorldAccountID, CharacterSelect_Struct* pCharacterSelectData);
private:
	DataProvider* mDataProvider; // DataStore is not responsible for deleting.
};