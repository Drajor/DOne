#pragma once

#include "LogSystem.h"
#include "IDataStore.h"

class TrueDataStore : public IDataStore {
public:
	virtual const bool initialise() { return true; }
	virtual const bool loadSettings() { return true; }
	virtual const bool loadAccounts(Data::AccountList pAccounts) { return true; }
	virtual const bool saveAccounts(Data::AccountList pAccounts) { return true; }
	virtual const bool loadAccountCharacterData(Data::Account* pAccount) { return true; }
	virtual const bool saveAccountCharacterData(Data::Account* pAccount) { return true; }
	virtual const bool loadCharacter(const String& pCharacterName, Data::Character* pCharacterData) { return true; }
	virtual const bool saveCharacter(const String& pCharacterName, const Data::Character* pCharacterData) { return true; }
	virtual const bool deleteCharacter(const String& pCharacterName) { return true; }
	virtual const bool loadGuilds(Data::GuildList pGuilds) { return true; };
	virtual const bool saveGuilds(Data::GuildList pGuilds) { return true; };
	virtual const bool loadZones(Data::ZoneList pZones) { return true; }
	virtual const bool saveZones(Data::ZoneList pZones) { return true; }
	virtual const bool loadNPCAppearanceData(Data::NPCAppearanceList pAppearances) { return true; }
	virtual const bool loadNPCTypeData(Data::NPCTypeList pTypes) { return true; }
	virtual const bool loadSpells(Data::SpellDataArray pSpells, const u32 pMaxSpellID, u32& pNumSpellsLoaded) { return true; }
	virtual const bool loadItems(ItemData* pItemData, u32& pNumItemsLoaded) { return true; }
	virtual const bool loadTransmutationComponents(Data::TransmutationComponentList pComponents) { return true; }
	virtual const bool loadAlternateCurrencies(Data::AlternateCurrencyList pCurrencies) { return true; }
	virtual const bool loadShops(Data::ShopList pShops) { return true; }
	virtual const bool loadTitles(Data::TitleList pTitles) { return true; }
	virtual const bool loadTasks(Data::TaskList& pTasks) { return true; }
};

class FalseDataStore : public IDataStore {
public:
	virtual const bool initialise() { return false; }
	virtual const bool loadSettings() { return false; }
	virtual const bool loadAccounts(Data::AccountList pAccounts) { return false; }
	virtual const bool saveAccounts(Data::AccountList pAccounts) { return false; }
	virtual const bool loadAccountCharacterData(Data::Account* pAccount) { return false; }
	virtual const bool saveAccountCharacterData(Data::Account* pAccount) { return false; }
	virtual const bool loadCharacter(const String& pCharacterName, Data::Character* pCharacterData) { return false; }
	virtual const bool saveCharacter(const String& pCharacterName, const Data::Character* pCharacterData) { return false; }
	virtual const bool deleteCharacter(const String& pCharacterName) { return false; }
	virtual const bool loadGuilds(Data::GuildList pGuilds) { return false; };
	virtual const bool saveGuilds(Data::GuildList pGuilds) { return false; };
	virtual const bool loadZones(Data::ZoneList pZones) { return false; }
	virtual const bool saveZones(Data::ZoneList pZones) { return false; }
	virtual const bool loadNPCAppearanceData(Data::NPCAppearanceList pAppearances) { return false; }
	virtual const bool loadNPCTypeData(Data::NPCTypeList pTypes) { return false; }
	virtual const bool loadSpells(Data::SpellDataArray pSpells, const u32 pMaxSpellID, u32& pNumSpellsLoaded) { return false; }
	virtual const bool loadItems(ItemData* pItemData, u32& pNumItemsLoaded) { return false; }
	virtual const bool loadTransmutationComponents(Data::TransmutationComponentList pComponents) { return false; }
	virtual const bool loadAlternateCurrencies(Data::AlternateCurrencyList pCurrencies) { return false; }
	virtual const bool loadShops(Data::ShopList pShops) { return false; }
	virtual const bool loadTitles(Data::TitleList pTitles) { return false; }
	virtual const bool loadTasks(Data::TaskList& pTasks) { return false; }
};

class NullLog : public ILog {
public:
	virtual void status(const String& pMessage) { }
	virtual void info(const String& pMessage) { }
	virtual void error(const String& pMessage) { }
	virtual void setContext(String pContext) { }
};

class NullLogFactory : public  ILogFactory {
public:
	ILog* make() { return new NullLog(); }
};

#include "../common/EQStreamIntf.h"

class NullEQStreamInterface : public EQStreamInterface {
	void QueuePacket(const EQApplicationPacket *p, bool ack_req = true) { };
	void FastQueuePacket(EQApplicationPacket **p, bool ack_req = true) { };
	EQApplicationPacket *PopPacket() { return nullptr; };
	void Close() {};
	void ReleaseFromUse() {};
	void RemoveData() {};
	uint32 GetRemoteIP() const { return 0; }
	uint16 GetRemotePort() const { return 0; }
	bool CheckState(EQStreamState state) { return EQStreamState::ESTABLISHED; }
	std::string Describe() const { return ""; }
};