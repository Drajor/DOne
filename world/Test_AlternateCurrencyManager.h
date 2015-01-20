#pragma once

#include "gtest/gtest.h"
#include "Test_Utility.h"

#include "AlternateCurrencyManager.h"

class AlternateCurrencyManagerTest : public ::testing::Test {
protected:
	virtual void SetUp() {
		mTrueDataStore = new TrueDataStore();
		mAlternateCurrencyManager = new AlternateCurrencyManager();
		mLogFactory = new NullLogFactory();
	}

	virtual void TearDown() {
		delete mAlternateCurrencyManager;
		mAlternateCurrencyManager = nullptr;

		delete mTrueDataStore;
		mTrueDataStore = nullptr;

		delete mLogFactory;
		mLogFactory = nullptr;
	}

	AlternateCurrencyManager* mAlternateCurrencyManager = nullptr;
	TrueDataStore* mTrueDataStore = nullptr;
	NullLogFactory* mLogFactory = nullptr;
};

TEST_F(AlternateCurrencyManagerTest, InitialiseNull) {
	// Fail: Null IDataStore.
	EXPECT_EQ(false, mAlternateCurrencyManager->initialise(nullptr, mLogFactory));

	// Fail: Null ILog.
	EXPECT_EQ(false, mAlternateCurrencyManager->initialise(mTrueDataStore, nullptr));
}

TEST_F(AlternateCurrencyManagerTest, DoubleInitialise) {
	// Pass.
	EXPECT_EQ(true, mAlternateCurrencyManager->initialise(mTrueDataStore, mLogFactory));

	// Fail: Already initialised.
	EXPECT_EQ(false, mAlternateCurrencyManager->initialise(mTrueDataStore, mLogFactory));
}

TEST_F(AlternateCurrencyManagerTest, InitialiseFalseDataStore) {
	auto dataStore = new FalseDataStore();
	EXPECT_EQ(false, mAlternateCurrencyManager->initialise(dataStore, mLogFactory));
	delete dataStore;
}