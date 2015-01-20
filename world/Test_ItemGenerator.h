#pragma once

#include "gtest/gtest.h"
#include "Test_Utility.h"

#include "ItemFactory.h"
#include "ItemDataStore.h"
#include "ItemGenerator.h"

class ItemGeneratorTest : public ::testing::Test {
protected:
	virtual void SetUp() {
		mItemGenerator = new ItemGenerator();
		mTrueDataStore = new TrueDataStore();
		mFalseDataStore = new FalseDataStore();
		mItemFactory = new ItemFactory();
		mItemDataStore = new ItemDataStore();
		mLogFactory = new NullLogFactory();
	}

	virtual void TearDown() {
		delete mItemGenerator;
		mItemGenerator = nullptr;

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

	ItemGenerator* mItemGenerator = nullptr;
	ItemFactory* mItemFactory = nullptr;
	ItemDataStore* mItemDataStore = nullptr;
	TrueDataStore* mTrueDataStore = nullptr;
	FalseDataStore* mFalseDataStore = nullptr;
	NullLogFactory* mLogFactory = nullptr;
};

TEST_F(ItemGeneratorTest, InitialiseNull) {
	// Fail: Null ItemFactory.
	EXPECT_FALSE(mItemGenerator->initialise(nullptr, mTrueDataStore, mLogFactory));

	// Fail: Null IDataStore.
	EXPECT_FALSE(mItemGenerator->initialise(mItemFactory, nullptr, mLogFactory));

	// Fail: Null ILogFactory.
	EXPECT_FALSE(mItemGenerator->initialise(mItemFactory, mTrueDataStore, nullptr));

	// Pass.
	EXPECT_TRUE(mItemGenerator->initialise(mItemFactory, mTrueDataStore, mLogFactory));
}

TEST_F(ItemGeneratorTest, DoubleInitialise) {
	// Pass.
	EXPECT_TRUE(mItemGenerator->initialise(mItemFactory, mTrueDataStore, mLogFactory));

	// Fail: Already initialised.
	EXPECT_FALSE(mItemGenerator->initialise(mItemFactory, mTrueDataStore, mLogFactory));
}