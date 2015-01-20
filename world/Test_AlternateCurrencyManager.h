#pragma once

#include "gtest/gtest.h"
#include "Test_Utility.h"

#include "AlternateCurrencyManager.h"

class AlternateCurrencyManagerTest : public ::testing::Test {
protected:
	virtual void SetUp() {
		mTrueDataStore = new TrueDataStore();
		mFalseDataStore = new FalseDataStore();
		mAlternateCurrencyManager = new AlternateCurrencyManager();
		mLogFactory = new NullLogFactory();
	}

	virtual void TearDown() {
		delete mAlternateCurrencyManager;
		mAlternateCurrencyManager = nullptr;

		delete mTrueDataStore;
		mTrueDataStore = nullptr;

		delete mFalseDataStore;
		mFalseDataStore = nullptr;

		delete mLogFactory;
		mLogFactory = nullptr;
	}

	AlternateCurrencyManager* mAlternateCurrencyManager = nullptr;
	TrueDataStore* mTrueDataStore = nullptr;
	FalseDataStore* mFalseDataStore = nullptr;
	NullLogFactory* mLogFactory = nullptr;
};

TEST_F(AlternateCurrencyManagerTest, InitialiseNull) {
	// Fail: Null IDataStore.
	EXPECT_FALSE(mAlternateCurrencyManager->initialise(nullptr, mLogFactory));

	// Fail: Null ILogFactory.
	EXPECT_FALSE(mAlternateCurrencyManager->initialise(mTrueDataStore, nullptr));
}

TEST_F(AlternateCurrencyManagerTest, DoubleInitialise) {
	// Pass.
	EXPECT_TRUE(mAlternateCurrencyManager->initialise(mTrueDataStore, mLogFactory));

	// Fail: Already initialised.
	EXPECT_FALSE(mAlternateCurrencyManager->initialise(mTrueDataStore, mLogFactory));
}

TEST_F(AlternateCurrencyManagerTest, InitialiseFalseDataStore) {
	EXPECT_FALSE(mAlternateCurrencyManager->initialise(mFalseDataStore, mLogFactory));
}