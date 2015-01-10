#pragma once

class IDataStore;
class ItemDataStore;
class SpellDataStore;
class ShopDataStore;
class ZoneDataManager; // TODO: ZoneDataStore.
class TitleManager; 
class AccountManager;
class World;
class UCS;
class ZoneManager;
class GroupManager;
class RaidManager;
class GuildManager;
class NPCFactory;
class ItemFactory;
class AlternateCurrencyManager;
class ItemGenerator;

class ServiceLocator {
public:
	
	// DataStore
	static inline IDataStore* getDataStore() { return mDataStore; }
	static inline void setDataStore(IDataStore* pDataStore) { mDataStore = pDataStore; }

	// ItemDataStore
	static inline ItemDataStore* getItemDataStore() { return mItemDataStore; }
	static inline void setItemDataStore(ItemDataStore* pItemDataStore) { mItemDataStore = pItemDataStore; }

	// SpellDataStore
	static inline SpellDataStore* getSpellDataStore() { return mSpellDataStore; }
	static inline void setSpellDataStore(SpellDataStore* pSpellDataStore) { mSpellDataStore = pSpellDataStore; }

	// ShopDataStore
	static inline ShopDataStore* getShopDataStore() { return mShopDataStore; }
	static inline void setShopDataStore(ShopDataStore* pShopDataStore) { mShopDataStore = pShopDataStore; }

	// ZoneDataManager
	static inline ZoneDataManager* getZoneDataManager() { return mZoneDataManager; }
	static inline void setZoneDataManager(ZoneDataManager* pZoneDataManager) { mZoneDataManager = pZoneDataManager; }

	// TitleManager
	static inline TitleManager* getTitleManager() { return mTitleManager; }
	static inline void setTitleManager(TitleManager* pTitleManager) { mTitleManager = pTitleManager; }

	// AccountManager
	static inline AccountManager* getAccountManager() { return mAccountManager; }
	static inline void setAccountManager(AccountManager* pAccountManager) { mAccountManager = pAccountManager; }
	
	// World
	static inline World* getWorld() { return mWorld; }
	static inline void setWorld(World* pWorld) { mWorld = pWorld; }

	// UCS
	static inline UCS* getUCS() { return mUCS; }
	static inline void setUCS(UCS* pUCS) { mUCS = pUCS; }

	// ZoneManager
	static inline ZoneManager* getZoneManager() { return mZoneManager; }
	static inline void setZoneManager(ZoneManager* pZoneManager) { mZoneManager = pZoneManager; }

	// GroupManager
	static inline GroupManager* getGroupManager() { return mGroupManager; }
	static inline void setGroupManager(GroupManager* pGroupManager) { mGroupManager = pGroupManager; }

	// RaidManager
	static inline RaidManager* getRaidManager() { return mRaidManager; }
	static inline void setRaidManager(RaidManager* pRaidManager) { mRaidManager = pRaidManager; }

	// GuildManager
	static inline GuildManager* getGuildManager() { return mGuildManager; }
	static inline void setGuildManager(GuildManager* pGuildManager) { mGuildManager = pGuildManager; }

	// NPCFactory
	static inline NPCFactory* getNPCFactory() { return mNPCFactory; }
	static inline void setNPCFactory(NPCFactory* pNPCFactory) { mNPCFactory = pNPCFactory; }

	// ItemFactory
	static inline ItemFactory* getItemFactory() { return mItemFactory; }
	static inline void setItemFactory(ItemFactory* pItemFactory) { mItemFactory = pItemFactory; }

	// AlternateCurrencyManager
	static inline AlternateCurrencyManager* getAlternateCurrencyManager() { return mAlternateCurrencyManager; }
	static inline void setAlternateCurrencyManager(AlternateCurrencyManager* pAlternateCurrencyManager) { mAlternateCurrencyManager = pAlternateCurrencyManager; }

	// ItemGenerator
	static inline ItemGenerator* getItemGenerator() { return mItemGenerator; }
	static inline void setItemGenerator(ItemGenerator* pItemGenerator) { mItemGenerator = pItemGenerator; }

	static void reset() {
		mDataStore = nullptr;
		mItemDataStore = nullptr;
		mSpellDataStore = nullptr;
		mShopDataStore = nullptr;
		mZoneDataManager = nullptr;
		mTitleManager = nullptr;
		mAccountManager = nullptr;
		mWorld = nullptr;
		mUCS = nullptr;
		mZoneManager = nullptr;
		mGroupManager = nullptr;
		mRaidManager = nullptr;
		mGuildManager = nullptr;
		mNPCFactory = nullptr;
		mItemFactory = nullptr;
		mAlternateCurrencyManager = nullptr;
		mItemGenerator = nullptr;
	}
private:

	ServiceLocator() { reset(); }
	~ServiceLocator();

	static IDataStore* mDataStore;
	static ItemDataStore* mItemDataStore;
	static SpellDataStore* mSpellDataStore;
	static ShopDataStore* mShopDataStore;
	static ZoneDataManager* mZoneDataManager;
	static TitleManager* mTitleManager;
	static AccountManager* mAccountManager;
	static World* mWorld;
	static UCS* mUCS;
	static ZoneManager* mZoneManager;
	static GroupManager* mGroupManager;
	static RaidManager* mRaidManager;
	static GuildManager* mGuildManager;
	static NPCFactory* mNPCFactory;
	static ItemFactory* mItemFactory;
	static AlternateCurrencyManager* mAlternateCurrencyManager;
	static ItemGenerator* mItemGenerator;
};