#include "DataStore.h"
#include "DataProvider.h"

DataStore::DataStore() : mDataProvider(0) { }
DataStore::~DataStore() { }

void DataStore::setProvider(DataProvider* pDataProvider) { mDataProvider = pDataProvider; }
void DataStore::update() { mDataProvider->update(); }
bool DataStore::getAccounts(std::list<AccountData*>& pAccounts) { return mDataProvider->getAccounts(pAccounts); }


