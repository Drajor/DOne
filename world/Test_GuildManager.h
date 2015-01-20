#pragma once

#include "gtest/gtest.h"

#include "Test_Utility.h"

#include "GuildManager.h"
#include "Guild.h"

class FakeGuildDataStore : public TrueDataStore {
public:
	const bool loadGuilds(Data::GuildList pGuilds) { 
		return true;
	};
};

class GuildManagerTest : public ::testing::Test {
protected:
	virtual void SetUp() {
		mTrueDataStore = new TrueDataStore();
		mFalseDataStore = new FalseDataStore();
		mFakeGuildDataStore = new FakeGuildDataStore();
		mGuildManager = new GuildManager();
		mLogFactory = new NullLogFactory();
	}

	virtual void TearDown() {
		delete mGuildManager;
		mGuildManager = nullptr;

		delete mTrueDataStore;
		mTrueDataStore = nullptr;

		delete mFalseDataStore;
		mFalseDataStore = nullptr;

		delete mFakeGuildDataStore;
		mTrueDataStore = nullptr;

		delete mLogFactory;
		mLogFactory = nullptr;
	}

	GuildManager* mGuildManager = nullptr;
	TrueDataStore* mTrueDataStore = nullptr;
	FalseDataStore* mFalseDataStore = nullptr;
	NullLogFactory* mLogFactory = nullptr;
	FakeGuildDataStore* mFakeGuildDataStore = nullptr;
};

TEST_F(GuildManagerTest, InitialiseNull) {
	// Fail: Null IDataStore.
	EXPECT_FALSE(mGuildManager->initialise(nullptr, mLogFactory));

	// Fail: Null ILogFactory.
	EXPECT_FALSE(mGuildManager->initialise(mTrueDataStore, nullptr));

	// Pass.
	EXPECT_TRUE(mGuildManager->initialise(mTrueDataStore, mLogFactory));
}

TEST_F(GuildManagerTest, DoubleInitialise) {
	// Pass.
	EXPECT_TRUE(mGuildManager->initialise(mTrueDataStore, mLogFactory));

	// Fail: Already initialised.
	EXPECT_FALSE(mGuildManager->initialise(mTrueDataStore, mLogFactory));
}

TEST_F(GuildManagerTest, InitialiseFalseDataStore) {
	EXPECT_FALSE(mGuildManager->initialise(mFalseDataStore, mLogFactory));
}

TEST_F(GuildManagerTest, onCreateNull) {
	// Pass.
	EXPECT_EQ(true, mGuildManager->initialise(mTrueDataStore, mLogFactory));

	EXPECT_FALSE(mGuildManager->onCreate(nullptr, "My Test Guild"));
}

TEST_F(GuildManagerTest, onCreateHasGuild) {
	// Pass.
	EXPECT_EQ(true, mGuildManager->initialise(mTrueDataStore, mLogFactory));
}

TEST_F(GuildManagerTest, onCreateNameLength) {
	// Pass.
	EXPECT_EQ(true, mGuildManager->initialise(mTrueDataStore, mLogFactory));

	// Fail: Too short.
	EXPECT_FALSE(mGuildManager->onCreate(nullptr, ""));

	// Fail: Too long.

	// Pass: Just right.
}