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
	EXPECT_FALSE(mShopDataStore->initialise(nullptr, mLogFactory));

	// Fail: Null ILogFactory.
	EXPECT_FALSE(mShopDataStore->initialise(mTrueDataStore, nullptr));

	// Pass.
	EXPECT_TRUE(mShopDataStore->initialise(mTrueDataStore, mLogFactory));
}

TEST_F(ShopDataStoreTest, DoubleInitialise) {
	// Pass.
	EXPECT_TRUE(mShopDataStore->initialise(mTrueDataStore, mLogFactory));

	// Fail: Already initialised.
	EXPECT_FALSE(mShopDataStore->initialise(mTrueDataStore, mLogFactory));
}

TEST_F(ShopDataStoreTest, InitialiseFalseDataStore) {
	EXPECT_FALSE(mShopDataStore->initialise(mFalseDataStore, mLogFactory));
}