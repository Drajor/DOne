#pragma once

#include "Constants.h"
#include "Utility.h"
#include "gtest/gtest.h"

// 
TEST(canClassTaunt, PlayerClasses) {
	EXPECT_TRUE(Utility::canClassTaunt(ClassID::Warrior));
	EXPECT_FALSE(Utility::canClassTaunt(ClassID::Cleric));
	EXPECT_TRUE(Utility::canClassTaunt(ClassID::Paladin));
	EXPECT_FALSE(Utility::canClassTaunt(ClassID::Ranger));
	EXPECT_TRUE(Utility::canClassTaunt(ClassID::Shadowknight));
	EXPECT_FALSE(Utility::canClassTaunt(ClassID::Druid));
	EXPECT_FALSE(Utility::canClassTaunt(ClassID::Monk));
	EXPECT_FALSE(Utility::canClassTaunt(ClassID::Bard));
	EXPECT_FALSE(Utility::canClassTaunt(ClassID::Rogue));
	EXPECT_FALSE(Utility::canClassTaunt(ClassID::Shaman));
	EXPECT_FALSE(Utility::canClassTaunt(ClassID::Necromancer));
	EXPECT_FALSE(Utility::canClassTaunt(ClassID::Necromancer));
	EXPECT_FALSE(Utility::canClassTaunt(ClassID::Wizard));
	EXPECT_FALSE(Utility::canClassTaunt(ClassID::Magician));
	EXPECT_FALSE(Utility::canClassTaunt(ClassID::Enchanter));
	EXPECT_FALSE(Utility::canClassTaunt(ClassID::Beastlord));
	EXPECT_FALSE(Utility::canClassTaunt(ClassID::Berserker));
}

class convertCurrencyTest : public ::testing::Test {
protected:
	virtual void SetUp() {
		mPlatinum = 0;
		mGold = 0;
		mSilver = 0;
		mCopper = 0;
	}

	int32 mPlatinum = 0;
	int32 mGold = 0;
	int32 mSilver = 0;
	int32 mCopper = 0;
};

TEST_F(convertCurrencyTest, Zeros) {
	Utility::convertFromCopper(0, mPlatinum, mGold, mSilver, mCopper);

	EXPECT_EQ(0, mPlatinum);
	EXPECT_EQ(0, mGold);
	EXPECT_EQ(0, mSilver);
	EXPECT_EQ(0, mCopper);
}

TEST_F(convertCurrencyTest, Nines) {
	Utility::convertFromCopper(999999999, mPlatinum, mGold, mSilver, mCopper);

	EXPECT_EQ(999999, mPlatinum);
	EXPECT_EQ(9, mGold);
	EXPECT_EQ(9, mSilver);
	EXPECT_EQ(9, mCopper);
}

TEST_F(convertCurrencyTest, Test1) {
	Utility::convertFromCopper(1234, mPlatinum, mGold, mSilver, mCopper);

	EXPECT_EQ(1, mPlatinum);
	EXPECT_EQ(2, mGold);
	EXPECT_EQ(3, mSilver);
	EXPECT_EQ(4, mCopper);
}

TEST_F(convertCurrencyTest, Test2) {
	Utility::convertFromCopper(90000, mPlatinum, mGold, mSilver, mCopper);

	EXPECT_EQ(90, mPlatinum);
	EXPECT_EQ(0, mGold);
	EXPECT_EQ(0, mSilver);
	EXPECT_EQ(0, mCopper);
}

TEST_F(convertCurrencyTest, Test3) {
	Utility::convertFromCopper(190, mPlatinum, mGold, mSilver, mCopper);

	EXPECT_EQ(0, mPlatinum);
	EXPECT_EQ(1, mGold);
	EXPECT_EQ(9, mSilver);
	EXPECT_EQ(0, mCopper);
}

class convertCurrencyTest2 : public ::testing::Test {
protected:
	virtual void SetUp() {
		mResult = 0;
		set();
	}

	void set(const int32 pPlatinum = 0, const int32 pGold = 0, const int32 pSilver = 0, const int32 pCopper = 0) {
		mPlatinum = pPlatinum;
		mGold = pGold;
		mSilver = pSilver;
		mCopper = pCopper;
	}

	int64 mResult = 0;
	int32 mPlatinum = 0;
	int32 mGold = 0;
	int32 mSilver = 0;
	int32 mCopper = 0;
};

TEST_F(convertCurrencyTest2, PlatinumZero) {
	mPlatinum = 0;
	EXPECT_TRUE(Utility::convertCurrency(mResult, mPlatinum, mGold, mSilver, mCopper));
	EXPECT_EQ(0, mResult);
}

TEST_F(convertCurrencyTest2, PlatinumOne) {
	mPlatinum = 1;
	EXPECT_TRUE(Utility::convertCurrency(mResult, mPlatinum, mGold, mSilver, mCopper));
	EXPECT_EQ(1000, mResult);
}

TEST_F(convertCurrencyTest2, PlatinumNine) {
	mPlatinum = 9;
	EXPECT_TRUE(Utility::convertCurrency(mResult, mPlatinum, mGold, mSilver, mCopper));
	EXPECT_EQ(9000, mResult);
}

TEST_F(convertCurrencyTest2, PlatinumNegative) {
	mPlatinum = -1;
	EXPECT_FALSE(Utility::convertCurrency(mResult, mPlatinum, mGold, mSilver, mCopper));
	EXPECT_EQ(0, mResult);
}

TEST_F(convertCurrencyTest2, GoldZero) {
	mGold = 0;
	EXPECT_TRUE(Utility::convertCurrency(mResult, mPlatinum, mGold, mSilver, mCopper));
	EXPECT_EQ(0, mResult);
}

TEST_F(convertCurrencyTest2, GoldOne) {
	mGold = 1;
	EXPECT_TRUE(Utility::convertCurrency(mResult, mPlatinum, mGold, mSilver, mCopper));
	EXPECT_EQ(100, mResult);
}

TEST_F(convertCurrencyTest2, GoldNine) {
	mGold = 9;
	EXPECT_TRUE(Utility::convertCurrency(mResult, mPlatinum, mGold, mSilver, mCopper));
	EXPECT_EQ(900, mResult);
}

TEST_F(convertCurrencyTest2, GoldNegative) {
	mGold = -1;
	EXPECT_FALSE(Utility::convertCurrency(mResult, mPlatinum, mGold, mSilver, mCopper));
	EXPECT_EQ(0, mResult);
}

TEST_F(convertCurrencyTest2, SilverZero) {
	mSilver = 0;
	EXPECT_TRUE(Utility::convertCurrency(mResult, mPlatinum, mGold, mSilver, mCopper));
	EXPECT_EQ(0, mResult);
}

TEST_F(convertCurrencyTest2, SilverOne) {
	mSilver = 1;
	EXPECT_TRUE(Utility::convertCurrency(mResult, mPlatinum, mGold, mSilver, mCopper));
	EXPECT_EQ(10, mResult);
}
TEST_F(convertCurrencyTest2, SilverNine) {
	mSilver = 9;
	EXPECT_TRUE(Utility::convertCurrency(mResult, mPlatinum, mGold, mSilver, mCopper));
	EXPECT_EQ(90, mResult);
}

TEST_F(convertCurrencyTest2, SilverNegative) {
	mSilver = -1;
	EXPECT_FALSE(Utility::convertCurrency(mResult, mPlatinum, mGold, mSilver, mCopper));
	EXPECT_EQ(0, mResult);
}

TEST_F(convertCurrencyTest2, CopperZero) {
	mCopper = 0;
	EXPECT_TRUE(Utility::convertCurrency(mResult, mPlatinum, mGold, mSilver, mCopper));
	EXPECT_EQ(0, mResult);
}

TEST_F(convertCurrencyTest2, CopperOne) {
	mCopper = 1;
	EXPECT_TRUE(Utility::convertCurrency(mResult, mPlatinum, mGold, mSilver, mCopper));
	EXPECT_EQ(1, mResult);
}

TEST_F(convertCurrencyTest2, CopperNine) {
	mCopper = 9;
	EXPECT_TRUE(Utility::convertCurrency(mResult, mPlatinum, mGold, mSilver, mCopper));
	EXPECT_EQ(9, mResult);
}

TEST_F(convertCurrencyTest2, CopperNegative) {
	mCopper = -1;
	EXPECT_FALSE(Utility::convertCurrency(mResult, mPlatinum, mGold, mSilver, mCopper));
	EXPECT_EQ(0, mResult);
}

TEST_F(convertCurrencyTest2, ResultNonZero) {
	mResult = -1;
	EXPECT_FALSE(Utility::convertCurrency(mResult, mPlatinum, mGold, mSilver, mCopper));
	EXPECT_EQ(-1, mResult);
}

TEST_F(convertCurrencyTest2, Test1) {
	set(1, 2, 3, 4);
	EXPECT_TRUE(Utility::convertCurrency(mResult, mPlatinum, mGold, mSilver, mCopper));
	EXPECT_EQ(1234, mResult);
}

TEST_F(convertCurrencyTest2, Test2) {
	set(9, 9, 9, 9);
	EXPECT_TRUE(Utility::convertCurrency(mResult, mPlatinum, mGold, mSilver, mCopper));
	EXPECT_EQ(9999, mResult);
}

TEST_F(convertCurrencyTest2, Test3) {
	set(9913, 9953, 9925, 991);
	EXPECT_TRUE(Utility::convertCurrency(mResult, mPlatinum, mGold, mSilver, mCopper));
	EXPECT_EQ(11008541, mResult);
}

TEST_F(convertCurrencyTest2, Test4) {
	set(0, 9953, 9925, 991);
	EXPECT_TRUE(Utility::convertCurrency(mResult, mPlatinum, mGold, mSilver, mCopper));
	EXPECT_EQ(1095541, mResult);
}

TEST_F(convertCurrencyTest2, Test5) {
	set(0, 0, 9925, 991);
	EXPECT_TRUE(Utility::convertCurrency(mResult, mPlatinum, mGold, mSilver, mCopper));
	EXPECT_EQ(100241, mResult);
}

TEST_F(convertCurrencyTest2, Test6) {
	set(0, 0, 0, 991);
	EXPECT_TRUE(Utility::convertCurrency(mResult, mPlatinum, mGold, mSilver, mCopper));
	EXPECT_EQ(991, mResult);
}