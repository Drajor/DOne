#include "DataStore.h"
#include "DataProvider.h"

DataStore::DataStore() : mDataProvider(0) { }
DataStore::~DataStore() { }

void DataStore::setProvider(DataProvider* pDataProvider) { mDataProvider = pDataProvider; }
void DataStore::update() { mDataProvider->update(); }
bool DataStore::getAccounts(std::list<AccountData*>& pAccounts) { return mDataProvider->getAccounts(pAccounts); }
bool DataStore::isCharacterNameUnique(String pCharacterName) { return mDataProvider->isCharacterNameUnique(pCharacterName); }
bool DataStore::getCharacterSelectInfo(uint32 pWorldAccountID, CharacterSelect_Struct* pCharacterSelectData) { return mDataProvider->getCharacterSelectInfo(pWorldAccountID, pCharacterSelectData); }

bool DataStore::deleteCharacter(String pCharacterName) { return mDataProvider->deleteCharacter(pCharacterName); }

bool DataStore::createCharacter(uint32 pWorldAccountID, String pCharacterName, PlayerProfile_Struct* pProfile, ExtendedProfile_Struct* pExtendedProfile) {
	return mDataProvider->createCharacter(pWorldAccountID, pCharacterName, pProfile, pExtendedProfile);
}

bool DataStore::checkOwnership(uint32 pWorldAccountID, String pCharacterName) { return mDataProvider->checkOwnership(pWorldAccountID, pCharacterName); }

bool DataStore::loadCharacter(String pCharacterName, uint32& pCharacterID, PlayerProfile_Struct* pProfile, ExtendedProfile_Struct* pExtendedProfile) {
	return mDataProvider->loadCharacter(pCharacterName, pCharacterID, pProfile, pExtendedProfile);
}

bool DataStore::createAccount(uint32 pLoginServerAccountID, String pLoginServerAccountName) {
	return mDataProvider->createAccount(pLoginServerAccountID, pLoginServerAccountName);
}

bool DataStore::saveCharacter(uint32 pCharacterID, PlayerProfile_Struct* pProfile, ExtendedProfile_Struct* pExtendedProfile) {
	return mDataProvider->saveCharacter(pCharacterID, pProfile, pExtendedProfile);
}




