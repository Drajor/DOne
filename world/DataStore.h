#pragma once

#include "../common/types.h"
#include "Data.h"
#include <list>

class DataProvider;
struct CharacterSelect_Struct;
struct PlayerProfile_Struct;
struct ExtendedProfile_Struct;

class DataStore {
public:
	DataStore();
	~DataStore();
	void setProvider(DataProvider* pDataProvider);
	void update();

	// Account Data
	bool getAccounts(std::list<AccountData*>& pAccounts);
	bool getCharacterSelectInfo(uint32 pWorldAccountID, CharacterSelect_Struct* pCharacterSelectData);
	bool isCharacterNameUnique(std::string pCharacterName);
	bool deleteCharacter(std::string pCharacterName);
	bool createCharacter(uint32 pWorldAccountID, std::string pCharacterName, PlayerProfile_Struct* pProfile, ExtendedProfile_Struct* pExtendedProfile);

	// Returns true if account(pWorldAccoundID) owns a character(pCharacterName).
	bool checkOwnership(uint32 pWorldAccountID, std::string pCharacterName);
	bool loadCharacter(std::string pCharacterName, uint32& pCharacterID, PlayerProfile_Struct* pProfile, ExtendedProfile_Struct* pExtendedProfile);
private:
	DataProvider* mDataProvider; // DataStore is not responsible for deleting.
};