#pragma once

#include "DataProvider.h"
#include <list>

struct AccountData;
struct CharacterSelect_Struct;
class DatabaseConnection;
class Timer;

class MySQLDataProvider : public DataProvider {
public:
	MySQLDataProvider();
	~MySQLDataProvider();
	bool initialise();
	void update();

	bool getAccounts(std::list<AccountData*>& pAccounts);
	bool getCharacterSelectInfo(uint32 pWorldAccountID, CharacterSelect_Struct* pCharacterSelectData);
private:
	Timer* mKeepAliveTimer;
	DatabaseConnection* mDatabaseConnection;
};