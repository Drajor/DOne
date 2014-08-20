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

	// Account Data
	bool loadAccounts(std::list<AccountData*>& pAccounts);
	bool saveAccounts(std::list<AccountData*>& pAccounts);
	bool loadAccountCharacterData(AccountData* pAccount);
	bool saveAccountCharacterData(AccountData* pAccount);

	// Character Data
	bool loadCharacter(const String& pCharacterName, CharacterData* pCharacterData);
	bool saveCharacter(const String& pCharacterName, const CharacterData* pCharacterData);
	
private:

	DataStore() {};
	~DataStore() {};
	DataStore(DataStore const&);
	void operator=(DataStore const&);
};