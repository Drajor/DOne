#pragma once

#include "IDataStore.h"

class ILogFactory;
class ILog;
class TiXmlElement;

namespace Poco {
	namespace Data {
		class Session;
	}
}

class XMLDataStore : public IDataStore {
public:
	~XMLDataStore();
	const bool initialise(ILogFactory* pLogFactory);
	const bool setup();
	
	// Settings
	const bool loadSettings();

	// Account Data

	i32 insertQuery(const String& pQuery);
	const bool updateQuery(const String& pQuery);

	// Returns whether an account already exists
	const bool accountExists(const u32 pLSAccountID, const u32 pLSID);

	// Creates an account and returns the ID.
	i32 accountCreate(const u32 pLSAccountID, const String& pLSAccountName, const u32 pLSID, const u32 pStatus);

	// Loads an Account.
	const bool accountLoad(Account* pAccount, const u32 pLSAccountID, const u32 pLSID);

	// Saves an Account.
	const bool accountSave(Account* pAccount);

	const i32 accountConnect(Account* pAccount);
	const bool accountDisconnect(Account* pAccount);

	const bool accountCharactersLoad(Account* pAccount);

	const bool loadAccountCharacterData(Data::Account* pAccount);
	const bool saveAccountCharacterData(Data::Account* pAccount);

	// Character Data
	const bool loadCharacter(const String& pCharacterName, Data::Character* pCharacterData);
	const bool loadInventory(TiXmlElement* pElement, Data::Inventory& pInventory);

	const bool saveCharacter(const String& pCharacterName, const Data::Character* pCharacterData);
	const bool saveInventory(TiXmlElement* pElement, const Data::Inventory& pInventory);

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
	const bool loadSpells(Data::SpellDataArray pSpells, const u32 pMaxSpellID, u32& pNumSpellsLoaded);
	
	// Items
	const bool loadItems(ItemData* pItemData, u32& pNumItemsLoaded);
	const bool loadTransmutationComponents(Data::TransmutationComponentList pComponents);

	const bool loadAlternateCurrencies(Data::AlternateCurrencyList pCurrencies);

	const bool loadShops(Data::ShopList pShops);
	const bool loadTitles(Data::TitleList pTitles);

	const bool loadTasks(Data::TaskDataArray& pTasks, const u32 pMaxTaskID, u32& pTasksLoaded);
	const bool readTask(TiXmlElement* pElement, Data::Task* pTask);
	const bool readTaskObjective(TiXmlElement* pElement, Data::TaskObjective* pObjective);

private:

	bool mInitialised = false;
	ILog* mLog = nullptr;
	Poco::Data::Session* mSession = nullptr;
};