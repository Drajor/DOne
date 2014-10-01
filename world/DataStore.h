#pragma once

#include "Constants.h"
#include "Data.h"

struct ItemData;

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

	// Zone Data
	const bool loadZoneData(std::list<ZoneData*>& pZoneData);

	// NPC
	bool loadNPCAppearanceData(std::list<NPCAppearanceData*>& pAppearances);

	// Spells
	const bool loadSpells(SpellData* pSpellData, uint32& pNumSpellsLoaded);
	
	// Items
	const bool loadItems(ItemData* pItemData, uint32& pNumItemsLoaded);
private:

	DataStore() {};
	~DataStore() {};
	DataStore(DataStore const&);
	void operator=(DataStore const&);
	
};