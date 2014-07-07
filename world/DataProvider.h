#pragma once

class DataProvider {
public:
	virtual ~DataProvider() { };
	virtual bool initialise() = 0;
private:
};