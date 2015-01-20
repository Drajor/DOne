#pragma once

#include "gtest/gtest.h"
#include "Test_Utility.h"

#include "AccountManager.h"
#include "Account.h"

class AccountManagerTest : public ::testing::Test {
protected:
	virtual void SetUp() {
		mTrueDataStore = new TrueDataStore();
		mFalseDataStore = new FalseDataStore();
		mAccountManager = new AccountManager();
		mLogFactory = new NullLogFactory();
	}

	virtual void TearDown() {
		delete mAccountManager;
		mAccountManager = nullptr;

		delete mTrueDataStore;
		mTrueDataStore = nullptr;

		delete mFalseDataStore;
		mFalseDataStore = nullptr;

		delete mLogFactory;
		mLogFactory = nullptr;
	}

	AccountManager* mAccountManager = nullptr;
	TrueDataStore* mTrueDataStore = nullptr;
	FalseDataStore* mFalseDataStore = nullptr;
	NullLogFactory* mLogFactory = nullptr;
};

TEST_F(AccountManagerTest, InitialiseNull) {
	// Fail: Null IDataStore.
	EXPECT_FALSE(mAccountManager->initialise(nullptr, mLogFactory));

	// Fail: Null ILogFactory.
	EXPECT_FALSE(mAccountManager->initialise(mTrueDataStore, nullptr));

	// Pass.
	EXPECT_TRUE(mAccountManager->initialise(mTrueDataStore, mLogFactory));
}

TEST_F(AccountManagerTest, DoubleInitialise) {
	// Pass.
	EXPECT_TRUE(mAccountManager->initialise(mTrueDataStore, mLogFactory));

	// Fail: Already initialised.
	EXPECT_FALSE(mAccountManager->initialise(mTrueDataStore, mLogFactory));
}

TEST_F(AccountManagerTest, InitialiseFalseDataStore) {
	EXPECT_FALSE(mAccountManager->initialise(mFalseDataStore, mLogFactory));
}

TEST_F(AccountManagerTest, createAccount) {
	// Pass.
	ASSERT_TRUE(mAccountManager->initialise(mTrueDataStore, mLogFactory));
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
	ASSERT_TRUE(mAccountManager->initialise(mTrueDataStore, mLogFactory));
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
	ASSERT_TRUE(mAccountManager->initialise(mTrueDataStore, mLogFactory));

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

TEST_F(AccountManagerTest, CreateCharacterA) {
	// Pass.
	ASSERT_TRUE(mAccountManager->initialise(mTrueDataStore, mLogFactory));

	// Create an Account.
	auto accountA = mAccountManager->createAccount(12, "test", 1);
	ASSERT_TRUE(accountA != nullptr);

	// Fail: Null payload.
	ASSERT_FALSE(mAccountManager->createCharacter(accountA, nullptr));

	auto p = new Payload::World::CreateCharacter();
	p->mClass = ClassID::Warrior;
	p->mRace = PlayableRaceIDs::Human;
	p->mGender = Gender::Female;
	p->mDeity = PlayerDeityIDs::Agnostic;

	// Fail: Null Account.
	ASSERT_FALSE(mAccountManager->createCharacter(nullptr, p));

	// Fail: Account not loaded.
	ASSERT_FALSE(mAccountManager->createCharacter(accountA, p));

	// Trigger Account to be loaded.
	ASSERT_TRUE(mAccountManager->onConnect(accountA));

	// Fail: Account has no reserved Character name.
	ASSERT_FALSE(mAccountManager->createCharacter(accountA, p));

	// Set the reserved Character name.
	accountA->setReservedCharacterName("Drajor");

	// Pass
	ASSERT_TRUE(mAccountManager->createCharacter(accountA, p));
	
	// Check: The reserved Character name on the Account has been wiped.
	ASSERT_FALSE(accountA->hasReservedCharacterName());
	
	// Check: Account now has a single Character.
	EXPECT_EQ(1, accountA->numCharacters());

	delete p;
}

TEST_F(AccountManagerTest, DeleteCharacterA) {
	// Pass.
	ASSERT_TRUE(mAccountManager->initialise(mTrueDataStore, mLogFactory));

	// Create an Account.
	auto accountA = mAccountManager->createAccount(12, "test", 1);
	ASSERT_TRUE(accountA != nullptr);

	// Trigger Account to be loaded.
	ASSERT_TRUE(mAccountManager->onConnect(accountA));

	// Set the reserved Character name.
	accountA->setReservedCharacterName("Drajor");

	auto p = new Payload::World::CreateCharacter();
	p->mClass = ClassID::Warrior;
	p->mRace = PlayableRaceIDs::Human;
	p->mGender = Gender::Female;
	p->mDeity = PlayerDeityIDs::Agnostic;

	// Pass
	ASSERT_TRUE(mAccountManager->createCharacter(accountA, p));

	// Check: Account now has a single Character.
	EXPECT_EQ(1, accountA->numCharacters());

	// Null Account.
	EXPECT_FALSE(mAccountManager->deleteCharacter(nullptr, "Soandso"));

	// Try deleting a Character that does not belong to the account.
	EXPECT_FALSE(mAccountManager->deleteCharacter(accountA, "Soandso"));

	EXPECT_TRUE(mAccountManager->deleteCharacter(accountA, "Drajor"));

	// Check: Account now has no Characters.
	EXPECT_EQ(0, accountA->numCharacters());

	delete p;
}