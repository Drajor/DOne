#include "ServiceLocator.h"

ItemDataStore* ServiceLocator::mItemDataStore = nullptr;
SpellDataStore* ServiceLocator::mSpellDataStore = nullptr;
ShopDataStore* ServiceLocator::mShopDataStore = nullptr;
ZoneDataStore* ServiceLocator::mZoneDataManager = nullptr;
TitleManager* ServiceLocator::mTitleManager = nullptr;
AccountManager* ServiceLocator::mAccountManager = nullptr;
World* ServiceLocator::mWorld = nullptr;
UCS* ServiceLocator::mUCS = nullptr;
ZoneManager* ServiceLocator::mZoneManager = nullptr;
GroupManager* ServiceLocator::mGroupManager = nullptr;
RaidManager* ServiceLocator::mRaidManager = nullptr;
GuildManager* ServiceLocator::mGuildManager = nullptr;
NPCFactory* ServiceLocator::mNPCFactory = nullptr;
ItemFactory* ServiceLocator::mItemFactory = nullptr;
AlternateCurrencyManager* ServiceLocator::mAlternateCurrencyManager = nullptr;
ItemGenerator* ServiceLocator::mItemGenerator = nullptr;