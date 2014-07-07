#pragma once

class DataProvider;

class DataStore {
public:
	DataStore();
	~DataStore();
	void setProvider(DataProvider* pDataProvider);

private:
	DataProvider* mDataProvider; // DataStore is not responsible for deleting.
};