#pragma once

#include "gtest/gtest.h"
#include "Test_Utility.h"

#include "NPCFactory.h"
#include "ItemFactory.h"
#include "ShopDataStore.h"

class NPCFactoryTest : public ::testing::Test {
protected:
	virtual void SetUp() {
		mTrueDataStore = new TrueDataStore();
		mFalseDataStore = new FalseDataStore();
		mNPCFactory = new NPCFactory();
		mItemFactory = new ItemFactory();
		mShopDataStore = new ShopDataStore();
		mLogFactory = new NullLogFactory();
	}

	virtual void TearDown() {
		delete mNPCFactory;
		mNPCFactory = nullptr;

		delete mItemFactory;
		mItemFactory = nullptr;

		delete mShopDataStore;
		mShopDataStore = nullptr;

		delete mTrueDataStore;
		mTrueDataStore = nullptr;

		delete mFalseDataStore;
		mFalseDataStore = nullptr;

		delete mLogFactory;
		mLogFactory = nullptr;
	}

	bool initialise() {

	}

	NPCFactory* mNPCFactory = nullptr;
	TrueDataStore* mTrueDataStore = nullptr;
	FalseDataStore* mFalseDataStore = nullptr;
	NullLogFactory* mLogFactory = nullptr;

	ItemFactory* mItemFactory = nullptr;
	ShopDataStore* mShopDataStore = nullptr;
};

TEST_F(NPCFactoryTest, InitialiseNull) {
	// Fail: Null IDataStore.
	EXPECT_FALSE(mNPCFactory->initialise(nullptr, mLogFactory, mItemFactory, mShopDataStore));

	// Fail: Null ILogFactory.
	EXPECT_FALSE(mNPCFactory->initialise(mTrueDataStore, nullptr, mItemFactory, mShopDataStore));

	// Fail: Null ItemFactory.
	EXPECT_FALSE(mNPCFactory->initialise(mTrueDataStore, mLogFactory, nullptr, mShopDataStore));

	// Fail: Null ShopDataStore.
	EXPECT_FALSE(mNPCFactory->initialise(mTrueDataStore, mLogFactory, mItemFactory, nullptr));

	// Pass:
	EXPECT_TRUE(mNPCFactory->initialise(mTrueDataStore, mLogFactory, mItemFactory, mShopDataStore));
}

TEST_F(NPCFactoryTest, DoubleInitialise) {
	// Pass:
	EXPECT_TRUE(mNPCFactory->initialise(mTrueDataStore, mLogFactory, mItemFactory, mShopDataStore));

	// Fail: Already initialised.
	EXPECT_FALSE(mNPCFactory->initialise(mTrueDataStore, mLogFactory, mItemFactory, mShopDataStore));
}

TEST_F(NPCFactoryTest, InitialiseFalseDataStore) {
	EXPECT_FALSE(mNPCFactory->initialise(mFalseDataStore, mLogFactory, mItemFactory, mShopDataStore));
}