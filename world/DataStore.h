#pragma once

#include "Types.h"
#include <list>

struct ItemData;
struct TransmutationComponent;

namespace Data {
	struct Account;
	struct Character;
	struct Zone;
	struct NPCType;
	struct NPCAppearance;
	struct Shop;
	struct AlternateCurrency;
	struct Spell;

	typedef std::list<Data::Account*>& AccountList;
	typedef std::list<Data::Zone*>& ZoneList;
	typedef std::list<Data::NPCAppearance*>& NPCAppearanceList;
	typedef std::list<Data::NPCType*>& NPCTypeList;
	typedef std::list<Data::Shop*>& ShopList;
	typedef std::list<Data::AlternateCurrency*>& AlternateCurrencyList;
}

class DataStore {
public:
	const bool initialise();
	
	// Settings
	static bool loadSettings();

	// Account Data
	bool loadAccounts(Data::AccountList pAccounts);
	bool saveAccounts(Data::AccountList pAccounts);
	const bool loadAccountCharacterData(Data::Account* pAccount);
	const bool saveAccountCharacterData(Data::Account* pAccount);

	// Character Data
	const bool loadCharacter(const String& pCharacterName, Data::Character* pCharacterData);
	const bool saveCharacter(const String& pCharacterName, const Data::Character* pCharacterData);
	const bool deleteCharacter(const String& pCharacterName);

	// Zone Data
	const bool loadZones(Data::ZoneList pZones);
	const bool saveZones(Data::ZoneList pZones);

	// NPC
	const bool loadNPCAppearanceData(Data::NPCAppearanceList pAppearances);
	const bool loadNPCTypeData(Data::NPCTypeList pTypes);

	// Spells
	const bool loadSpells(Data::Spell* pSpellData, u32& pNumSpellsLoaded);
	
	// Items
	const bool loadItems(ItemData* pItemData, u32& pNumItemsLoaded);
	const bool loadTransmutationComponents(std::list<TransmutationComponent*>& pComponents);

	const bool loadAlternateCurrencies(Data::AlternateCurrencyList pCurrencies);

	const bool loadShops(Data::ShopList pShops);
};