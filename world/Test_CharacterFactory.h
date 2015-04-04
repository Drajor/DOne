#pragma once

#include "gtest/gtest.h"
#include "Test_Utility.h"

#include "CharacterFactory.h"
#include "TaskDataStore.h"

class CharacterFactoryTest : public ::testing::Test {
protected:
	virtual void SetUp() {
		mTrueDataStore = new TrueDataStore();
		mFalseDataStore = new FalseDataStore();
		mCharacterFactory = new CharacterFactory();
		mTaskDataStore = new TaskDataStore();

		mItemFactory = new ItemFactory();
		mLogFactory = new NullLogFactory();
	}

	virtual void TearDown() {
		delete mCharacterFactory;
		mCharacterFactory = nullptr;

		delete mTaskDataStore;
		mTaskDataStore = nullptr;

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
	TaskDataStore* mTaskDataStore = nullptr;
	TrueDataStore* mTrueDataStore = nullptr;
	FalseDataStore* mFalseDataStore = nullptr;
	NullLogFactory* mLogFactory = nullptr;

	ItemFactory* mItemFactory = nullptr;
};

TEST_F(CharacterFactoryTest, InitialiseNull) {
	// Fail: Null IDataStore.
	EXPECT_FALSE(mCharacterFactory->initialise(nullptr, mLogFactory, mItemFactory, mTaskDataStore));

	// Fail: Null ILogFactory.
	EXPECT_FALSE(mCharacterFactory->initialise(mTrueDataStore, nullptr, mItemFactory, mTaskDataStore));

	// Fail: Null ItemFactory.
	EXPECT_FALSE(mCharacterFactory->initialise(mTrueDataStore, mLogFactory, nullptr, mTaskDataStore));

	// Fail: Null TaskDataStore.
	EXPECT_FALSE(mCharacterFactory->initialise(mTrueDataStore, mLogFactory, mItemFactory, nullptr));

	// Pass:
	EXPECT_TRUE(mCharacterFactory->initialise(mTrueDataStore, mLogFactory, mItemFactory, mTaskDataStore));
}

TEST_F(CharacterFactoryTest, DoubleInitialise) {
	// Pass:
	EXPECT_TRUE(mCharacterFactory->initialise(mTrueDataStore, mLogFactory, mItemFactory, mTaskDataStore));

	// Fail: Already initialised.
	EXPECT_FALSE(mCharacterFactory->initialise(mTrueDataStore, mLogFactory, mItemFactory, mTaskDataStore));
}

TEST_F(CharacterFactoryTest, InitialiseFalseDataStore) {
	//EXPECT_FALSE(mCharacterFactory->initialise(mFalseDataStore, mLogFactory, mItemFactory));
}

TEST_F(CharacterFactoryTest, makeNullAccount) {
	// Pass:
	EXPECT_TRUE(mCharacterFactory->initialise(mTrueDataStore, mLogFactory, mItemFactory, mTaskDataStore));

	EXPECT_FALSE(mCharacterFactory->make("", nullptr));
}