#pragma once

#include "gtest/gtest.h"
#include "Test_Utility.h"

#include "ItemFactory.h"
#include "ItemDataStore.h"

class ItemFactoryTest : public ::testing::Test {
protected:
	virtual void SetUp() {
		mTrueDataStore = new TrueDataStore();
		mFalseDataStore = new FalseDataStore();
		mItemFactory = new ItemFactory();
		mItemDataStore = new ItemDataStore();
		mLogFactory = new NullLogFactory();
	}

	virtual void TearDown() {
		delete mItemFactory;
		mItemFactory = nullptr;

		delete mItemDataStore;
		mItemDataStore = nullptr;

		delete mTrueDataStore;
		mTrueDataStore = nullptr;

		delete mFalseDataStore;
		mFalseDataStore = nullptr;

		delete mLogFactory;
		mLogFactory = nullptr;
	}

	ItemFactory* mItemFactory = nullptr;
	ItemDataStore* mItemDataStore = nullptr;
	TrueDataStore* mTrueDataStore = nullptr;
	FalseDataStore* mFalseDataStore = nullptr;
	NullLogFactory* mLogFactory = nullptr;
};

TEST_F(ItemFactoryTest, InitialiseNull) {
	// Fail: Null ItemDataStore.
	EXPECT_FALSE(mItemFactory->initialise(nullptr, mLogFactory));

	// Fail: Null ILogFactory.
	EXPECT_FALSE(mItemFactory->initialise(mItemDataStore, nullptr));

	// Pass.
	EXPECT_TRUE(mItemFactory->initialise(mItemDataStore, mLogFactory));
}

TEST_F(ItemFactoryTest, DoubleInitialise) {
	// Pass.
	EXPECT_TRUE(mItemFactory->initialise(mItemDataStore, mLogFactory));

	// Fail: Already initialised.
	EXPECT_FALSE(mItemFactory->initialise(mItemDataStore, mLogFactory));
}