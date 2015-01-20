#pragma once

class ILogFactory;
class IDataStore;
class ShopDataStore;
class AccountManager;
class ZoneDataStore;
class TitleManager;
class SpellDataStore;
class AlternateCurrencyManager;
class ItemDataStore;
class ItemFactory;
class ZoneManager;
class GroupManager;
class RaidManager;
class GuildManager;
class CommandHandler;
class NPCFactory;
class World;
class UCS;
class CharacterFactory;

class TestServer {
public:
	TestServer();
	~TestServer();

	const bool initialise();

private:

	ILogFactory* mLogFactory = nullptr;
	IDataStore* mDataStore = nullptr;
	ShopDataStore* mShopDataStore = nullptr;
	AccountManager* mAccountManager = nullptr;
	ZoneDataStore* mZoneDataStore = nullptr;
	TitleManager* mTitleManager = nullptr;
	SpellDataStore* mSpellDataStore = nullptr;
	AlternateCurrencyManager* mAlternateCurrencyManager = nullptr;
	ItemDataStore* mItemDataStore = nullptr;
	ItemFactory* mItemFactory = nullptr;
	ZoneManager* mZoneManager = nullptr;
	GroupManager* mGroupManager = nullptr;
	RaidManager* mRaidManager = nullptr;
	GuildManager* mGuildManager = nullptr;
	CommandHandler* mCommandHandler = nullptr;
	NPCFactory* mNPCFactory = nullptr;
	World* mWorld = nullptr;
	UCS* mUCS = nullptr;
	CharacterFactory* mCharacterFactory = nullptr;
};