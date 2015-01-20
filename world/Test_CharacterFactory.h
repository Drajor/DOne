#pragma once

#include "gtest/gtest.h"
#include "Test_Utility.h"

#include "CharacterFactory.h"

class CharacterFactoryTest : public ::testing::Test {
protected:
	virtual void SetUp() {
		mTrueDataStore = new TrueDataStore();
		mFalseDataStore = new FalseDataStore();
		mCharacterFactory = new CharacterFactory();
		mItemFactory = new ItemFactory();
		mLogFactory = new NullLogFactory();
	}

	virtual void TearDown() {
		delete mCharacterFactory;
		mCharacterFactory = nullptr;

		delete mItemFactory;
		mItemFactory = nullptr;

		delete mTrueDataStore;
		mTrueDataStore = nullptr;

		delete mFalseDataStore;
		mFalseDataStore = nullptr;

		delete mLogFactory;
		mLogFactory = nullptr;
	}

	CharacterFactory* mCharacterFactory = nullptr;
	TrueDataStore* mTrueDataStore = nullptr;
	FalseDataStore* mFalseDataStore = nullptr;
	NullLogFactory* mLogFactory = nullptr;

	ItemFactory* mItemFactory = nullptr;
};

TEST_F(CharacterFactoryTest, InitialiseNull) {
	// Fail: Null IDataStore.
	EXPECT_FALSE(mCharacterFactory->initialise(nullptr, mLogFactory, mItemFactory));

	// Fail: Null ILogFactory.
	EXPECT_FALSE(mCharacterFactory->initialise(mTrueDataStore, nullptr, mItemFactory));

	// Fail: Null ItemFactory.
	EXPECT_FALSE(mCharacterFactory->initialise(mTrueDataStore, mLogFactory, nullptr));

	// Pass:
	EXPECT_TRUE(mCharacterFactory->initialise(mTrueDataStore, mLogFactory, mItemFactory));
}

TEST_F(CharacterFactoryTest, DoubleInitialise) {
	// Pass:
	EXPECT_TRUE(mCharacterFactory->initialise(mTrueDataStore, mLogFactory, mItemFactory));

	// Fail: Already initialised.
	EXPECT_FALSE(mCharacterFactory->initialise(mTrueDataStore, mLogFactory, mItemFactory));
}

TEST_F(CharacterFactoryTest, InitialiseFalseDataStore) {
	//EXPECT_FALSE(mCharacterFactory->initialise(mFalseDataStore, mLogFactory, mItemFactory));
}

TEST_F(CharacterFactoryTest, makeNullAccount) {
	// Pass:
	EXPECT_TRUE(mCharacterFactory->initialise(mTrueDataStore, mLogFactory, mItemFactory));

	EXPECT_FALSE(mCharacterFactory->make("", nullptr));
}