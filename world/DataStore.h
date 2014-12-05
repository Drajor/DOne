#pragma once

#include "Constants.h"
#include "Data.h"

struct ItemData;

namespace Data {
	struct NPCType;
	struct NPCAppearance;
	struct Shop;
}

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
	const bool loadAccountCharacterData(AccountData* pAccount);
	const bool saveAccountCharacterData(AccountData* pAccount);

	// Character Data
	const bool loadCharacter(const String& pCharacterName, CharacterData* pCharacterData);
	const bool saveCharacter(const String& pCharacterName, const CharacterData* pCharacterData);
	const bool deleteCharacter(const String& pCharacterName);

	// Zone Data
	const bool loadZoneData(std::list<Data::Zone*>& pZoneData);
	const bool saveZoneData(std::list<Data::Zone*>& pZoneData);

	// NPC
	const bool loadNPCAppearanceData(std::list<Data::NPCAppearance*>& pAppearances);
	const bool loadNPCTypeData(std::list<Data::NPCType*>& pTypes);

	// Spells
	const bool loadSpells(SpellData* pSpellData, uint32& pNumSpellsLoaded);
	
	// Items
	const bool loadItems(ItemData* pItemData, uint32& pNumItemsLoaded);
	const bool loadTransmutationComponents(std::list<TransmutationComponent*>& pComponents);

	const bool loadAlternateCurrencies(std::list<AlternateCurrency*>& pCurrencies);

	const bool loadShops(std::list<Data::Shop*>& pShops);
private:

	DataStore() {};
	~DataStore() {};
	DataStore(DataStore const&);
	void operator=(DataStore const&);
	
};