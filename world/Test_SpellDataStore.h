#pragma once

#include "gtest/gtest.h"
#include "Test_Utility.h"

#include "SpellDataStore.h"

class SpellDataStoreTest : public ::testing::Test {
protected:
	virtual void SetUp() {
		mTrueDataStore = new TrueDataStore();
		mFalseDataStore = new FalseDataStore();
		mSpellDataStore = new SpellDataStore();
		mLogFactory = new NullLogFactory();
	}

	virtual void TearDown() {
		delete mSpellDataStore;
		mSpellDataStore = nullptr;

		delete mTrueDataStore;
		mTrueDataStore = nullptr;

		delete mFalseDataStore;
		mFalseDataStore = nullptr;

		delete mLogFactory;
		mLogFactory = nullptr;
	}

	SpellDataStore* mSpellDataStore = nullptr;
	TrueDataStore* mTrueDataStore = nullptr;
	FalseDataStore* mFalseDataStore = nullptr;
	NullLogFactory* mLogFactory = nullptr;
};

TEST_F(SpellDataStoreTest, InitialiseNull) {
	// Fail: Null IDataStore.
	EXPECT_EQ(false, mSpellDataStore->initialise(nullptr, mLogFactory));

	// Fail: Null ILogFactory.
	EXPECT_EQ(false, mSpellDataStore->initialise(mTrueDataStore, nullptr));
}

TEST_F(SpellDataStoreTest, DoubleInitialise) {
	// Pass.
	EXPECT_EQ(true, mSpellDataStore->initialise(mTrueDataStore, mLogFactory));

	// Fail: Already initialised.
	EXPECT_EQ(false, mSpellDataStore->initialise(mTrueDataStore, mLogFactory));
}

TEST_F(SpellDataStoreTest, InitialiseFalseDataStore) {
	EXPECT_EQ(false, mSpellDataStore->initialise(mFalseDataStore, mLogFactory));
}