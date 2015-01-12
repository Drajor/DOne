#pragma once

#include "gtest/gtest.h"

#include "AccountManager.h"
#include "Account.h"
#include "IDataStore.h"

class NullDataStore : public IDataStore {
public:
	virtual const bool initialise()  { return true; }
	virtual const bool loadSettings()  { return true; }
	virtual const bool loadAccounts(Data::AccountList pAccounts)  { return true; }
	virtual const bool saveAccounts(Data::AccountList pAccounts)  { return true; }
	virtual const bool loadAccountCharacterData(Data::Account* pAccount)  { return true; }
	virtual const bool saveAccountCharacterData(Data::Account* pAccount)  { return true; }
	virtual const bool loadCharacter(const String& pCharacterName, Data::Character* pCharacterData)  { return true; }
	virtual const bool saveCharacter(const String& pCharacterName, const Data::Character* pCharacterData)  { return true; }
	virtual const bool deleteCharacter(const String& pCharacterName)  { return true; }
	virtual const bool loadZones(Data::ZoneList pZones)  { return true; }
	virtual const bool saveZones(Data::ZoneList pZones)  { return true; }
	virtual const bool loadNPCAppearanceData(Data::NPCAppearanceList pAppearances)  { return true; }
	virtual const bool loadNPCTypeData(Data::NPCTypeList pTypes)  { return true; }
	virtual const bool loadSpells(Data::Spell* pSpellData, u32& pNumSpellsLoaded)  { return true; }
	virtual const bool loadItems(ItemData* pItemData, u32& pNumItemsLoaded)  { return true; }
	virtual const bool loadTransmutationComponents(std::list<TransmutationComponent*>& pComponents)  { return true; }
	virtual const bool loadAlternateCurrencies(Data::AlternateCurrencyList pCurrencies)  { return true; }
	virtual const bool loadShops(Data::ShopList pShops)  { return true; }
};

class NullLog : public ILog {
public:
	void status(const String& pMessage) { }
	void info(const String& pMessage) { }
	void error(const String& pMessage) { }
	void setContext(String pContext) { }
};

class NullLogFactory : public  ILogFactory {
public:
	ILog* make() { return new NullLog(); }
};

class AccountManagerTest : public ::testing::Test {
protected:
	virtual void SetUp() {
		mDataStore = new NullDataStore();
		mAccountManager = new AccountManager();
		mLogFactory = new NullLogFactory();
	}

	virtual void TearDown() {
		delete mAccountManager;
		mAccountManager = nullptr;

		delete mDataStore;
		mDataStore = nullptr;

		delete mLogFactory;
		mLogFactory = nullptr;
	}

	AccountManager* mAccountManager = nullptr;
	NullDataStore* mDataStore = nullptr;
	NullLogFactory* mLogFactory = nullptr;
};

TEST_F(AccountManagerTest, Initialise) {
	// Fail: Null IDataStore.
	EXPECT_EQ(false, mAccountManager->initialise(nullptr, mLogFactory));

	// Fail: Null ILog.
	EXPECT_EQ(false, mAccountManager->initialise(mDataStore, nullptr));

	// Pass.
	EXPECT_EQ(true, mAccountManager->initialise(mDataStore, mLogFactory));

	// Fail: Already initialised.
	EXPECT_EQ(false, mAccountManager->initialise(mDataStore, mLogFactory));
}

TEST_F(AccountManagerTest, createAccount) {
	// Pass.
	ASSERT_TRUE(mAccountManager->initialise(mDataStore, mLogFactory));
	EXPECT_TRUE(mAccountManager->numAccounts() == 0);

	// Create an Account.
	auto accountA = mAccountManager->createAccount(65, "test", 207);
	ASSERT_TRUE(accountA != nullptr);

	// Check: AccountManager now has an Account / Data::Account
	EXPECT_TRUE(mAccountManager->numAccounts() == 1);

	// Check: AccountManager configured the Account object correctly.
	EXPECT_EQ(65, accountA->getLoginAccountID());
	EXPECT_EQ("test", accountA->getLoginAccountName());
	EXPECT_EQ(207, accountA->getLoginServerID());

	// Create another Account.
	auto accountB = mAccountManager->createAccount(623, "anothertest", 12);
	ASSERT_TRUE(accountB != nullptr);

	EXPECT_TRUE(mAccountManager->numAccounts() == 2);

	// Check: AccountManager configured the Account object correctly.
	EXPECT_EQ(623, accountB->getLoginAccountID());
	EXPECT_EQ("anothertest", accountB->getLoginAccountName());
	EXPECT_EQ(12, accountB->getLoginServerID());

	// Create another Account.
	// Fail: Account already exists.
	auto accountC = mAccountManager->createAccount(623, "anothertest", 12);
	ASSERT_TRUE(accountC == nullptr);
}

TEST_F(AccountManagerTest, BanAndSuspend) {
	// Pass.
	ASSERT_TRUE(mAccountManager->initialise(mDataStore, mLogFactory));
	EXPECT_TRUE(mAccountManager->numAccounts() == 0);

	// Create an Account.
	auto accountA = mAccountManager->createAccount(65, "test", 207);
	ASSERT_TRUE(accountA != nullptr);

	accountA = mAccountManager->getAccount(65, 207);
	ASSERT_TRUE(accountA != nullptr);

	// Ban
	EXPECT_TRUE(mAccountManager->ban(accountA));
	EXPECT_TRUE(accountA->getStatus() == AccountStatus::Banned);
	EXPECT_TRUE(mAccountManager->getStatus(65, 207) == AccountStatus::Banned);

	// Remove ban
	EXPECT_TRUE(mAccountManager->removeBan(accountA));
	EXPECT_TRUE(accountA->getStatus() == AccountStatus::Default);
	EXPECT_TRUE(mAccountManager->getStatus(65, 207) == AccountStatus::Default);

	// Suspend
	EXPECT_TRUE(mAccountManager->suspend(accountA, 10102232));
	EXPECT_TRUE(accountA->getStatus() == AccountStatus::Suspended);
	EXPECT_TRUE(accountA->getSuspensionTime() == 10102232);
	EXPECT_TRUE(mAccountManager->getStatus(65, 207) == AccountStatus::Suspended);

	// Remove suspension
	EXPECT_TRUE(mAccountManager->removeSuspension(accountA));
	EXPECT_TRUE(accountA->getStatus() == AccountStatus::Default);
	EXPECT_TRUE(accountA->getSuspensionTime() == 0);
	EXPECT_TRUE(mAccountManager->getStatus(65, 207) == AccountStatus::Default);
}

TEST_F(AccountManagerTest, CharacterNames) {
	// Pass.
	ASSERT_TRUE(mAccountManager->initialise(mDataStore, mLogFactory));

	// Truth table
	std::map<bool, std::string> names = {
		{ false, "" }, // Bad: empty
		{ false, "Sho" }, // Bad: too short
		{ false, "Shor" }, // Bad: too short
		{ true, "Short" }, // Good: normal
		{ true, "Drajor" }, // Good: really good :p
		{ false, "drajor" }, // Bad: missing leading uppercase character.
		{ false, "Draj0r" },  // Bad: non-alpha character.
		{ false, "DrAjor" }, // Bad: random uppercase character.
		{ true, "Derpatron" }, // Good: normal
		{ false, "Abcdefghijklmnop" }, // Bad: Too long.
		{ false, "J_eepers" }, // Bad: non-alpha character.
	};

	for (auto i : names) {
		EXPECT_EQ(i.first, mAccountManager->isCharacterNameAllowed(i.second));
	}
}

TEST_F(AccountManagerTest, CreateCharacter) {

}