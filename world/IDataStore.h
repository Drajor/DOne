#pragma once

#include "Types.h"
#include "SpellContants.h"
#include <list>
#include <array>

class ILogFactory;
struct ItemData;

namespace Data {
	struct Account;
	struct Character;
	struct Inventory;
	struct Guild;
	struct GuildMember;
	struct Zone;
	struct NPCType;
	struct NPCAppearance;
	struct Shop;
	struct AlternateCurrency;
	struct Spell;
	struct SpellEffect;
	struct SpellComponent;
	struct Title;
	struct TransmutationComponent;
	struct Task;
	struct TaskObjective;

	typedef std::list<Data::Account*>& AccountList;
	typedef std::list<Data::Guild*>& GuildList;
	typedef std::list<Data::Zone*>& ZoneList;
	typedef std::list<Data::NPCAppearance*>& NPCAppearanceList;
	typedef std::list<Data::NPCType*>& NPCTypeList;
	typedef std::list<Data::Shop*>& ShopList;
	typedef std::list<Data::AlternateCurrency*>& AlternateCurrencyList;
	typedef std::list<Data::Title*>& TitleList;
	typedef std::list<Data::TransmutationComponent*>& TransmutationComponentList;
	typedef std::array<Data::Spell*, MaxSpellID>& SpellDataArray;
	typedef std::list<Data::Task*> TaskList;
	typedef std::list<Data::TaskObjective*> TaskObjectiveList;
}

class IDataStore {
public:

	virtual ~IDataStore() { };

	// Settings
	virtual const bool loadSettings() = 0;

	// Account Data
	virtual const bool loadAccounts(Data::AccountList pAccounts) = 0;
	virtual const bool saveAccounts(Data::AccountList pAccounts) = 0;
	virtual const bool loadAccountCharacterData(Data::Account* pAccount) = 0;
	virtual const bool saveAccountCharacterData(Data::Account* pAccount) = 0;

	// Character Data
	virtual const bool loadCharacter(const String& pCharacterName, Data::Character* pCharacterData) = 0;
	virtual const bool saveCharacter(const String& pCharacterName, const Data::Character* pCharacterData) = 0;
	virtual const bool deleteCharacter(const String& pCharacterName) = 0;

	// Guild
	virtual const bool loadGuilds(Data::GuildList pGuilds) = 0;
	virtual const bool saveGuilds(Data::GuildList pGuilds) = 0;

	// Zone Data
	virtual const bool loadZones(Data::ZoneList pZones) = 0;
	virtual const bool saveZones(Data::ZoneList pZones) = 0;

	// NPC
	virtual const bool loadNPCAppearanceData(Data::NPCAppearanceList pAppearances) = 0;
	virtual const bool loadNPCTypeData(Data::NPCTypeList pTypes) = 0;

	// Spells
	virtual const bool loadSpells(Data::SpellDataArray pSpellData, const u32 pMaxSpellID, u32& pNumSpellsLoaded) = 0;

	// Items
	virtual const bool loadItems(ItemData* pItemData, u32& pNumItemsLoaded) = 0;
	virtual const bool loadTransmutationComponents(Data::TransmutationComponentList pComponents) = 0;

	virtual const bool loadAlternateCurrencies(Data::AlternateCurrencyList pCurrencies) = 0;

	virtual const bool loadShops(Data::ShopList pShops) = 0;
	virtual const bool loadTitles(Data::TitleList pTitles) = 0;
	
	// Tasks
	virtual const bool loadTasks(Data::TaskList& pTasks) = 0;
};