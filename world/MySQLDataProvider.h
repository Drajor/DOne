#pragma once

#include "DataProvider.h"
#include <list>

struct AccountData;
class DatabaseConnection;
class Timer;

class MySQLDataProvider : public DataProvider {
public:
	MySQLDataProvider();
	~MySQLDataProvider();
	bool initialise();
	void update();

	bool getAccounts(std::list<AccountData*>& pAccounts);
private:
	Timer* mKeepAliveTimer;
	DatabaseConnection* mDatabaseConnection;
};