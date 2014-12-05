#pragma once

#include "Types.h"
#include "Data.h"
#include <list>

struct ItemData;

namespace Data {
	struct Account;
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
	bool loadAccounts(std::list<Data::Account*>& pAccounts);
	bool saveAccounts(std::list<Data::Account*>& pAccounts);
	const bool loadAccountCharacterData(Data::Account* pAccount);
	const bool saveAccountCharacterData(Data::Account* pAccount);

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
	const bool loadSpells(SpellData* pSpellData, u32& pNumSpellsLoaded);
	
	// Items
	const bool loadItems(ItemData* pItemData, u32& pNumItemsLoaded);
	const bool loadTransmutationComponents(std::list<TransmutationComponent*>& pComponents);

	const bool loadAlternateCurrencies(std::list<AlternateCurrency*>& pCurrencies);

	const bool loadShops(std::list<Data::Shop*>& pShops);
private:

	DataStore() {};
	~DataStore() {};
	DataStore(DataStore const&);
	void operator=(DataStore const&);
	
};