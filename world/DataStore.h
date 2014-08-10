#pragma once

#include "Constants.h"
#include "Data.h"

class DataProvider;
struct CharacterSelect_Struct;
struct PlayerProfile_Struct;
struct ExtendedProfile_Struct;

class DataStore {
public:
	static DataStore& getInstance() {
		static DataStore instance;
		return instance;
	}
	bool initialise();

	void update();

	// Account Data
	bool getAccounts(std::list<AccountData*>& pAccounts);
	bool getCharacterSelectInfo(uint32 pWorldAccountID, CharacterSelect_Struct* pCharacterSelectData);
	bool isCharacterNameUnique(String pCharacterName);
	bool deleteCharacter(String pCharacterName);
	bool createCharacter(uint32 pWorldAccountID, String pCharacterName, PlayerProfile_Struct* pProfile, ExtendedProfile_Struct* pExtendedProfile);

	// Returns true if account(pWorldAccoundID) owns a character(pCharacterName).
	bool checkOwnership(uint32 pWorldAccountID, String pCharacterName);
	bool loadCharacter(String pCharacterName, uint32& pCharacterID, PlayerProfile_Struct* pProfile, ExtendedProfile_Struct* pExtendedProfile);
	bool createAccount(uint32 pLoginServerAccountID, String pLoginServerAccountName);
	bool saveCharacter(uint32 pCharacterID, PlayerProfile_Struct* pProfile, ExtendedProfile_Struct* pExtendedProfile);
private:
	DataProvider* mDataProvider; // DataStore is not responsible for deleting.

	DataStore();
	~DataStore() {};
	DataStore(DataStore const&);
	void operator=(DataStore const&);
};