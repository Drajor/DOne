#pragma once

#include "gtest/gtest.h"
#include "Test_Utility.h"

#include "ItemDataStore.h"

class ItemDataStoreTest : public ::testing::Test {
protected:
	virtual void SetUp() {
		mTrueDataStore = new TrueDataStore();
		mFalseDataStore = new FalseDataStore();
		mItemDataStore = new ItemDataStore();
		mLogFactory = new NullLogFactory();
	}

	virtual void TearDown() {
		delete mItemDataStore;
		mItemDataStore = nullptr;

		delete mTrueDataStore;
		mTrueDataStore = nullptr;

		delete mFalseDataStore;
		mFalseDataStore = nullptr;

		delete mLogFactory;
		mLogFactory = nullptr;
	}

	ItemDataStore* mItemDataStore = nullptr;
	TrueDataStore* mTrueDataStore = nullptr;
	FalseDataStore* mFalseDataStore = nullptr;
	NullLogFactory* mLogFactory = nullptr;
};

TEST_F(ItemDataStoreTest, InitialiseNull) {
	// Fail: Null IDataStore.
	EXPECT_FALSE(mItemDataStore->initialise(nullptr, mLogFactory));

	// Fail: Null ILogFactory.
	EXPECT_FALSE(mItemDataStore->initialise(mTrueDataStore, nullptr));

	// Pass.
	EXPECT_TRUE(mItemDataStore->initialise(mTrueDataStore, mLogFactory));
}

TEST_F(ItemDataStoreTest, DoubleInitialise) {
	// Pass.
	EXPECT_TRUE(mItemDataStore->initialise(mTrueDataStore, mLogFactory));

	// Fail: Already initialised.
	EXPECT_FALSE(mItemDataStore->initialise(mTrueDataStore, mLogFactory));
}

TEST_F(ItemDataStoreTest, InitialiseFalseDataStore) {
	// Disabled until ItemDataStore starts using DataStore.
	//EXPECT_FALSE(mItemDataStore->initialise(mFalseDataStore, mLogFactory));
}