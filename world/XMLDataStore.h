#pragma once

#include "IDataStore.h"

class TiXmlElement;

class XMLDataStore : public IDataStore {
public:
	const bool initialise();
	
	// Settings
	const bool loadSettings();

	// Account Data
	const bool loadAccounts(Data::AccountList pAccounts);
	const bool saveAccounts(Data::AccountList pAccounts);
	const bool loadAccountCharacterData(Data::Account* pAccount);
	const bool saveAccountCharacterData(Data::Account* pAccount);

	// Character Data
	const bool loadCharacter(const String& pCharacterName, Data::Character* pCharacterData);
	const bool saveCharacter(const String& pCharacterName, const Data::Character* pCharacterData);
	const bool deleteCharacter(const String& pCharacterName);

	// Guild
	virtual const bool loadGuilds(Data::GuildList pGuilds);
	virtual const bool readGuild(TiXmlElement* pElement, Data::Guild* pGuild);
	virtual const bool readGuildMember(TiXmlElement* pElement, Data::GuildMember* pMember);

	virtual const bool saveGuilds(Data::GuildList pGuilds);
	virtual const bool writeGuild(TiXmlElement* pElement, Data::Guild* pGuild);
	virtual const bool writeGuildMember(TiXmlElement* pElement, Data::GuildMember* pMember);

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
	const bool loadTitles(Data::TitleList pTitles);
};