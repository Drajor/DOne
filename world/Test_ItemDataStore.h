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
	EXPECT_EQ(false, mItemDataStore->initialise(nullptr, mLogFactory));

	// Fail: Null ILogFactory.
	EXPECT_EQ(false, mItemDataStore->initialise(mTrueDataStore, nullptr));
}

TEST_F(ItemDataStoreTest, DoubleInitialise) {
	// Pass.
	EXPECT_EQ(true, mItemDataStore->initialise(mTrueDataStore, mLogFactory));

	// Fail: Already initialised.
	EXPECT_EQ(false, mItemDataStore->initialise(mTrueDataStore, mLogFactory));
}

TEST_F(ItemDataStoreTest, InitialiseFalseDataStore) {
	// Disabled until ItemDataStore starts using DataStore.
	//EXPECT_EQ(false, mItemDataStore->initialise(mFalseDataStore, mLogFactory));
}