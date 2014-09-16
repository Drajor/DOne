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
	
	// Settings
	static bool loadSettings();

	// Account Data
	bool loadAccounts(std::list<AccountData*>& pAccounts);
	bool saveAccounts(std::list<AccountData*>& pAccounts);
	bool loadAccountCharacterData(AccountData* pAccount);
	bool saveAccountCharacterData(AccountData* pAccount);

	// Character Data
	const bool loadCharacter(const String& pCharacterName, CharacterData* pCharacterData);
	const bool saveCharacter(const String& pCharacterName, const CharacterData* pCharacterData);
	const bool deleteCharacter(const String& pCharacterName);

	// Spawn Points
	bool loadSpawnPointData(const String& pZoneShortName, std::list<SpawnPointData*>& pSpawnPoints);

	// NPC
	bool loadNPCAppearanceData(std::list<NPCAppearanceData*>& pAppearances);

	// Spells
	const bool loadSpells(SpellData* pSpellData, uint32& pNumSpellsLoaded);
	
private:

	DataStore() {};
	~DataStore() {};
	DataStore(DataStore const&);
	void operator=(DataStore const&);
};