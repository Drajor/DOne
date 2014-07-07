#pragma once

#include "DataProvider.h"

class MySQLDataProvider : public DataProvider {
public:
	MySQLDataProvider();
	~MySQLDataProvider();
	bool initialise();
private:
};