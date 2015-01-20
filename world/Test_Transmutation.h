#pragma once

#include "gtest/gtest.h"
#include "Test_Utility.h"

#include "Transmutation.h"
#include "ItemFactory.h"

class TransmutationTest : public ::testing::Test {
protected:
	virtual void SetUp() {
		mItemFactory = new ItemFactory();
		mTrueDataStore = new TrueDataStore();
		mFalseDataStore = new FalseDataStore();
		mTransmutation = new Transmutation();
		mLogFactory = new NullLogFactory();
	}

	virtual void TearDown() {
		delete mItemFactory;
		mItemFactory = nullptr;

		delete mTransmutation;
		mTransmutation = nullptr;

		delete mTrueDataStore;
		mTrueDataStore = nullptr;

		delete mFalseDataStore;
		mFalseDataStore = nullptr;

		delete mLogFactory;
		mLogFactory = nullptr;
	}

	ItemFactory* mItemFactory = nullptr;
	Transmutation* mTransmutation = nullptr;
	TrueDataStore* mTrueDataStore = nullptr;
	FalseDataStore* mFalseDataStore = nullptr;
	NullLogFactory* mLogFactory = nullptr;
};

TEST_F(TransmutationTest, InitialiseNull) {
	// Fail: Null IDataStore.
	EXPECT_FALSE(mTransmutation->initialise(nullptr, mLogFactory, mItemFactory));

	// Fail: Null ILogFactory.
	EXPECT_FALSE(mTransmutation->initialise(mTrueDataStore, nullptr, mItemFactory));

	// Fail: Null ItemFactory.
	EXPECT_FALSE(mTransmutation->initialise(mTrueDataStore, mLogFactory, nullptr));

	// Pass.
	EXPECT_TRUE(mTransmutation->initialise(mTrueDataStore, mLogFactory, mItemFactory));
}

TEST_F(TransmutationTest, DoubleInitialise) {
	// Pass.
	EXPECT_TRUE(mTransmutation->initialise(mTrueDataStore, mLogFactory, mItemFactory));

	// Fail: Already initialised.
	EXPECT_FALSE(mTransmutation->initialise(mTrueDataStore, mLogFactory, mItemFactory));
}

TEST_F(TransmutationTest, InitialiseFalseDataStore) {
	EXPECT_FALSE(mTransmutation->initialise(mFalseDataStore, mLogFactory, mItemFactory));
}