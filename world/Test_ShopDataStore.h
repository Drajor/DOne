#pragma once

#include "gtest/gtest.h"
#include "Test_Utility.h"

#include "ShopDataStore.h"

class ShopDataStoreTest : public ::testing::Test {
protected:
	virtual void SetUp() {
		mTrueDataStore = new TrueDataStore();
		mFalseDataStore = new FalseDataStore();
		mShopDataStore = new ShopDataStore();
		mLogFactory = new NullLogFactory();
	}

	virtual void TearDown() {
		delete mShopDataStore;
		mShopDataStore = nullptr;

		delete mTrueDataStore;
		mTrueDataStore = nullptr;

		delete mFalseDataStore;
		mFalseDataStore = nullptr;

		delete mLogFactory;
		mLogFactory = nullptr;
	}

	ShopDataStore* mShopDataStore = nullptr;
	TrueDataStore* mTrueDataStore = nullptr;
	FalseDataStore* mFalseDataStore = nullptr;
	NullLogFactory* mLogFactory = nullptr;
};

TEST_F(ShopDataStoreTest, InitialiseNull) {
	// Fail: Null IDataStore.
	EXPECT_EQ(false, mShopDataStore->initialise(nullptr, mLogFactory));

	// Fail: Null ILog.
	EXPECT_EQ(false, mShopDataStore->initialise(mTrueDataStore, nullptr));
}

TEST_F(ShopDataStoreTest, DoubleInitialise) {
	// Pass.
	EXPECT_EQ(true, mShopDataStore->initialise(mTrueDataStore, mLogFactory));

	// Fail: Already initialised.
	EXPECT_EQ(false, mShopDataStore->initialise(mTrueDataStore, mLogFactory));
}

TEST_F(ShopDataStoreTest, InitialiseFalseDataStore) {
	EXPECT_EQ(false, mShopDataStore->initialise(mFalseDataStore, mLogFactory));
}