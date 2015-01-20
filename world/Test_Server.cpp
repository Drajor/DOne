#include "Test_Server.h"
#include "Test_Utility.h"

#include "ShopDataStore.h"
#include "AccountManager.h"
#include "ZoneData.h"
#include "TitleManager.h"
#include "SpellDataStore.h"
#include "AlternateCurrencyManager.h"
#include "ItemDataStore.h"
#include "ItemFactory.h"
#include "ZoneManager.h"
#include "GroupManager.h"
#include "RaidManager.h"
#include "GuildManager.h"
#include "CommandHandler.h"
#include "NPCFactory.h"
#include "World.h"
#include "UCS.h"
#include "CharacterFactory.h"

TestServer::~TestServer() {
	
	if (mShopDataStore) {
		delete mShopDataStore;
		mShopDataStore = nullptr;
	}

	if (mAccountManager) {
		delete mAccountManager;
		mAccountManager = nullptr;
	}

	if (mZoneDataStore) {
		delete mZoneDataStore;
		mZoneDataStore = nullptr;
	}
}

const bool TestServer::initialise() {
	mLogFactory = new NullLogFactory();
	mDataStore = new TrueDataStore();
	if (!mDataStore->loadSettings()) return false;
	if (!mDataStore->initialise()) return false;

	mShopDataStore = new ShopDataStore();
	if (!mShopDataStore->initialise(mDataStore, mLogFactory)) return false;

	mAccountManager = new AccountManager();
	if (!mAccountManager->initialise(mDataStore, mLogFactory)) return false;

	mZoneDataStore = new ZoneDataStore();
	if (!mZoneDataStore->initialise(mDataStore, mLogFactory)) return false;

	mTitleManager = new TitleManager();
	if (!mTitleManager->initialise(mDataStore, mLogFactory)) return false;

	mSpellDataStore = new SpellDataStore();
	if (!mSpellDataStore->initialise(mDataStore, mLogFactory)) return false;

	mAlternateCurrencyManager = new AlternateCurrencyManager();
	if (!mAlternateCurrencyManager->initialise(mDataStore, mLogFactory)) return false;

	mItemDataStore = new ItemDataStore();
	if (!mItemDataStore->initialise(mDataStore, mLogFactory)) return false;

	mItemFactory = new ItemFactory();
	if (!mItemFactory->initialise(mItemDataStore, mLogFactory)) return false;

	mZoneManager = new ZoneManager();
	mGroupManager = new GroupManager();
	mRaidManager = new RaidManager();
	mGuildManager = new GuildManager();
	mCommandHandler = new CommandHandler();

	mNPCFactory = new NPCFactory();
	if (!mNPCFactory->initialise(mDataStore, mLogFactory, mItemFactory, mShopDataStore)) return false;

	mCharacterFactory = new CharacterFactory();
	if (!mCharacterFactory->initialise(mDataStore, mLogFactory, mItemFactory)) return false;

	mWorld = new World();

	if (!mZoneManager->initialise(mWorld, mZoneDataStore, mGroupManager, mRaidManager, mGuildManager, mCommandHandler, mItemFactory, mLogFactory, mNPCFactory)) return false;
	if (!mGroupManager->initialise(mZoneManager)) return false;
	if (!mRaidManager->initialise(mZoneManager)) return false;
	if (!mGuildManager->initialise(mDataStore, mLogFactory)) return false;


	if(!mWorld->initialise(mDataStore, mLogFactory, mCharacterFactory, mGuildManager, mZoneManager, mAccountManager)) return false;

	mUCS = new UCS();
	if (!mUCS->initialise()) return false;

	return true;
}
