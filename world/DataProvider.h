#pragma once

#include <list>

struct AccountData;

class DataProvider {
public:
	virtual ~DataProvider() { };
	virtual bool initialise() = 0;
	virtual void update() { };
	virtual bool getAccounts(std::list<AccountData*>& pAccounts) = 0;
private:
};