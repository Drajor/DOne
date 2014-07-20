#include "DataStore.h"
#include "DataProvider.h"

DataStore::DataStore() : mDataProvider(0) { }
DataStore::~DataStore() { }

void DataStore::setProvider(DataProvider* pDataProvider) { mDataProvider = pDataProvider; }
void DataStore::update() { mDataProvider->update(); }
bool DataStore::getAccounts(std::list<AccountData*>& pAccounts) { return mDataProvider->getAccounts(pAccounts); }
bool DataStore::isCharacterNameUnique(std::string pCharacterName) { return mDataProvider->isCharacterNameUnique(pCharacterName); }
bool DataStore::getCharacterSelectInfo(uint32 pWorldAccountID, CharacterSelect_Struct* pCharacterSelectData) { return mDataProvider->getCharacterSelectInfo(pWorldAccountID, pCharacterSelectData); }

bool DataStore::deleteCharacter(std::string pCharacterName) { return mDataProvider->deleteCharacter(pCharacterName); }

bool DataStore::createCharacter(uint32 pWorldAccountID, std::string pCharacterName, PlayerProfile_Struct* pProfile, ExtendedProfile_Struct* pExtendedProfile) {
	return mDataProvider->createCharacter(pWorldAccountID, pCharacterName, pProfile, pExtendedProfile);
}

bool DataStore::checkOwnership(uint32 pWorldAccountID, std::string pCharacterName) { return mDataProvider->checkOwnership(pWorldAccountID, pCharacterName); }

bool DataStore::loadCharacter(std::string pCharacterName, uint32& pCharacterID, PlayerProfile_Struct* pProfile, ExtendedProfile_Struct* pExtendedProfile) {
	return mDataProvider->loadCharacter(pCharacterName, pCharacterID, pProfile, pExtendedProfile);
}




