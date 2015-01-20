#pragma once

#include "gtest/gtest.h"
#include "Test_Utility.h"

#include "ZoneData.h"

class ZoneDataStoreTest : public ::testing::Test {
protected:
	virtual void SetUp() {
		mTrueDataStore = new TrueDataStore();
		mFalseDataStore = new FalseDataStore();
		mZoneDataStore = new ZoneDataStore();
		mLogFactory = new NullLogFactory();
	}

	virtual void TearDown() {
		delete mZoneDataStore;
		mZoneDataStore = nullptr;

		delete mTrueDataStore;
		mTrueDataStore = nullptr;

		delete mFalseDataStore;
		mFalseDataStore = nullptr;

		delete mLogFactory;
		mLogFactory = nullptr;
	}

	ZoneDataStore* mZoneDataStore = nullptr;
	TrueDataStore* mTrueDataStore = nullptr;
	FalseDataStore* mFalseDataStore = nullptr;
	NullLogFactory* mLogFactory = nullptr;
};

TEST_F(ZoneDataStoreTest, InitialiseNull) {
	// Fail: Null IDataStore.
	EXPECT_EQ(false, mZoneDataStore->initialise(nullptr, mLogFactory));

	// Fail: Null ILog.
	EXPECT_EQ(false, mZoneDataStore->initialise(mTrueDataStore, nullptr));
}

TEST_F(ZoneDataStoreTest, DoubleInitialise) {
	// Pass.
	EXPECT_EQ(true, mZoneDataStore->initialise(mTrueDataStore, mLogFactory));

	// Fail: Already initialised.
	EXPECT_EQ(false, mZoneDataStore->initialise(mTrueDataStore, mLogFactory));
}

TEST_F(ZoneDataStoreTest, InitialiseFalseDataStore) {
	EXPECT_EQ(false, mZoneDataStore->initialise(mFalseDataStore, mLogFactory));
}