#pragma once

#include "Constants.h"
#include "Utility.h"
#include "Inventory.h"

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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class InventoryCurrencyTest : public ::testing::Test {
protected:
	virtual void SetUp() {
		mInventory = new Inventoryy();
		mPlatinum = 0;
		mGold = 0;
		mSilver = 0;
		mCopper = 0;
	}

	virtual void TearDown() {
		delete mInventory;
		mInventory = nullptr;
	}

	int32 mPlatinum = 0;
	int32 mGold = 0;
	int32 mSilver = 0;
	int32 mCopper = 0;
	Inventoryy* mInventory;
};

TEST_F(InventoryCurrencyTest, DefaultChecks) {
	// Cursor.
	EXPECT_EQ(0, mInventory->getCursorCopper());
	EXPECT_EQ(0, mInventory->getCursorSilver());
	EXPECT_EQ(0, mInventory->getCursorGold());
	EXPECT_EQ(0, mInventory->getCursorPlatinum());

	// Personal.
	EXPECT_EQ(0, mInventory->getPersonalCopper());
	EXPECT_EQ(0, mInventory->getPersonalSilver());
	EXPECT_EQ(0, mInventory->getPersonalGold());
	EXPECT_EQ(0, mInventory->getPersonalPlatinum());

	// Bank.
	EXPECT_EQ(0, mInventory->getBankCopper());
	EXPECT_EQ(0, mInventory->getBankSilver());
	EXPECT_EQ(0, mInventory->getBankGold());
	EXPECT_EQ(0, mInventory->getBankPlatinum());

	// Shared Bank.
	EXPECT_EQ(0, mInventory->getSharedBankPlatinum());

	// Total
	EXPECT_EQ(0, mInventory->getTotalCursorCurrency());
	EXPECT_EQ(0, mInventory->getTotalPersonalCurrency());
	EXPECT_EQ(0, mInventory->getTotalBankCurrency());
	EXPECT_EQ(0, mInventory->getTotalSharedBankCurrency());
	EXPECT_EQ(0, mInventory->getTotalCurrency());
}

TEST_F(InventoryCurrencyTest, AddBadSlot) {
	EXPECT_FALSE(mInventory->addCurrency(5, CurrencyType::Copper, 1));
	EXPECT_EQ(0, mInventory->getTotalCurrency());
}

TEST_F(InventoryCurrencyTest, AddBadType) {
	EXPECT_FALSE(mInventory->addCurrency(CurrencySlot::Personal, 4, 1));
	EXPECT_EQ(0, mInventory->getTotalCurrency());
}

TEST_F(InventoryCurrencyTest, AddSingleSlotSingleCurrency) {
	EXPECT_TRUE(mInventory->addCurrency(CurrencySlot::Personal, CurrencyType::Copper, 1));
	EXPECT_EQ(1, mInventory->getPersonalCopper());
	
	// Total
	EXPECT_EQ(1, mInventory->getTotalCurrency());
	EXPECT_EQ(1, mInventory->getTotalPersonalCurrency());
}

TEST_F(InventoryCurrencyTest, AddMultipleSlotSingleCurrency) {
	EXPECT_TRUE(mInventory->addCurrency(CurrencySlot::Personal, CurrencyType::Copper, 1));
	EXPECT_TRUE(mInventory->addCurrency(CurrencySlot::Bank, CurrencyType::Copper, 3));
	EXPECT_EQ(1, mInventory->getPersonalCopper());
	EXPECT_EQ(3, mInventory->getBankCopper());
	
	// Total
	EXPECT_EQ(4, mInventory->getTotalCurrency());
	EXPECT_EQ(1, mInventory->getTotalPersonalCurrency());
	EXPECT_EQ(3, mInventory->getTotalBankCurrency());
}

TEST_F(InventoryCurrencyTest, AddSingleSlotMultipleType) {
	EXPECT_TRUE(mInventory->addCurrency(CurrencySlot::Personal, CurrencyType::Copper, 50));
	EXPECT_TRUE(mInventory->addCurrency(CurrencySlot::Personal, CurrencyType::Gold, 7));
	EXPECT_EQ(50, mInventory->getPersonalCopper());
	EXPECT_EQ(7, mInventory->getPersonalGold());

	// Total
	EXPECT_EQ(750, mInventory->getTotalCurrency());
	EXPECT_EQ(750, mInventory->getTotalPersonalCurrency());
}

TEST_F(InventoryCurrencyTest, AddMultipleSlotMultipleType) {
	EXPECT_TRUE(mInventory->addCurrency(CurrencySlot::Personal, CurrencyType::Copper, 11));
	EXPECT_TRUE(mInventory->addCurrency(CurrencySlot::Bank, CurrencyType::Silver, 66));
	EXPECT_TRUE(mInventory->addCurrency(CurrencySlot::Cursor, CurrencyType::Gold, 2));
	EXPECT_TRUE(mInventory->addCurrency(CurrencySlot::SharedBank, CurrencyType::Platinum, 14));
	
	EXPECT_EQ(11, mInventory->getPersonalCopper());
	EXPECT_EQ(66, mInventory->getBankSilver());
	EXPECT_EQ(2, mInventory->getCursorGold());
	EXPECT_EQ(14, mInventory->getSharedBankPlatinum());

	// Total
	EXPECT_EQ(14871, mInventory->getTotalCurrency());
	EXPECT_EQ(11, mInventory->getTotalPersonalCurrency());
	EXPECT_EQ(660, mInventory->getTotalBankCurrency());
	EXPECT_EQ(200, mInventory->getTotalCursorCurrency());
	EXPECT_EQ(14000, mInventory->getTotalSharedBankCurrency());
}

TEST_F(InventoryCurrencyTest, AddPersonal) {
	EXPECT_TRUE(mInventory->addCurrency(CurrencySlot::Personal, CurrencyType::Copper, 1));
	EXPECT_EQ(1, mInventory->getPersonalCopper());
	EXPECT_TRUE(mInventory->addCurrency(CurrencySlot::Personal, CurrencyType::Silver, 2));
	EXPECT_EQ(2, mInventory->getPersonalSilver());
	EXPECT_TRUE(mInventory->addCurrency(CurrencySlot::Personal, CurrencyType::Gold, 3));
	EXPECT_EQ(3, mInventory->getPersonalGold());
	EXPECT_TRUE(mInventory->addCurrency(CurrencySlot::Personal, CurrencyType::Platinum, 4));
	EXPECT_EQ(4, mInventory->getPersonalPlatinum());

	// Total
	EXPECT_EQ(4321, mInventory->getTotalCurrency());
	EXPECT_EQ(4321, mInventory->getTotalPersonalCurrency());
}

TEST_F(InventoryCurrencyTest, MoveBroadOne) {
	// Setup
	EXPECT_TRUE(mInventory->addCurrency(CurrencySlot::Personal, CurrencyType::Copper, 897));
	EXPECT_TRUE(mInventory->addCurrency(CurrencySlot::Personal, CurrencyType::Silver, 434));
	EXPECT_TRUE(mInventory->addCurrency(CurrencySlot::Personal, CurrencyType::Gold, 249));
	EXPECT_TRUE(mInventory->addCurrency(CurrencySlot::Personal, CurrencyType::Platinum, 67));

	EXPECT_EQ(97137, mInventory->getTotalPersonalCurrency());

	EXPECT_TRUE(mInventory->addCurrency(CurrencySlot::Bank, CurrencyType::Copper, 3));
	EXPECT_TRUE(mInventory->addCurrency(CurrencySlot::Bank, CurrencyType::Silver, 2));
	EXPECT_TRUE(mInventory->addCurrency(CurrencySlot::Bank, CurrencyType::Gold, 7));
	EXPECT_TRUE(mInventory->addCurrency(CurrencySlot::Bank, CurrencyType::Platinum, 12398));

	EXPECT_EQ(12398723, mInventory->getTotalBankCurrency());

	EXPECT_EQ(12398723 + 97137, mInventory->getTotalCurrency());

	// Transfer personal copper into bank.

	// Pick up copper.
	EXPECT_TRUE(mInventory->moveCurrency(CurrencySlot::Personal, CurrencySlot::Cursor, CurrencyType::Copper, CurrencyType::Copper, 897));
	EXPECT_EQ(0, mInventory->getPersonalCopper());
	EXPECT_EQ(897, mInventory->getCursorCopper());
	// Drop on bank gold.
	EXPECT_TRUE(mInventory->moveCurrency(CurrencySlot::Cursor, CurrencySlot::Bank, CurrencyType::Copper, CurrencyType::Gold, 897));
	EXPECT_EQ(97, mInventory->getCursorCopper());
	EXPECT_EQ(15, mInventory->getBankGold());
	// Drop on bank silver.
	EXPECT_TRUE(mInventory->moveCurrency(CurrencySlot::Cursor, CurrencySlot::Bank, CurrencyType::Copper, CurrencyType::Silver, 90));
	EXPECT_EQ(7, mInventory->getCursorCopper());
	EXPECT_EQ(11, mInventory->getBankSilver());
	// Drop on bank copper.
	EXPECT_TRUE(mInventory->moveCurrency(CurrencySlot::Cursor, CurrencySlot::Bank, CurrencyType::Copper, CurrencyType::Copper, 7));
	EXPECT_EQ(0, mInventory->getCursorCopper());
	EXPECT_EQ(10, mInventory->getBankCopper());

	// Transfer personal silver into bank.

	// Pick up silver.
	EXPECT_TRUE(mInventory->moveCurrency(CurrencySlot::Personal, CurrencySlot::Cursor, CurrencyType::Silver, CurrencyType::Silver, 434));
	EXPECT_EQ(0, mInventory->getPersonalSilver());
	EXPECT_EQ(434, mInventory->getCursorSilver());
	// Drop on bank platinum.
	EXPECT_TRUE(mInventory->moveCurrency(CurrencySlot::Cursor, CurrencySlot::Bank, CurrencyType::Silver, CurrencyType::Platinum, 400));
	EXPECT_EQ(34, mInventory->getCursorSilver());
	EXPECT_EQ(12402, mInventory->getBankPlatinum());
	// Drop on bank gold.
	EXPECT_TRUE(mInventory->moveCurrency(CurrencySlot::Cursor, CurrencySlot::Bank, CurrencyType::Silver, CurrencyType::Gold, 30));
	EXPECT_EQ(4, mInventory->getCursorSilver());
	EXPECT_EQ(18, mInventory->getBankGold());
	// Drop on bank silver.
	EXPECT_TRUE(mInventory->moveCurrency(CurrencySlot::Cursor, CurrencySlot::Bank, CurrencyType::Silver, CurrencyType::Silver, 4));
	EXPECT_EQ(0, mInventory->getCursorSilver());
	EXPECT_EQ(15, mInventory->getBankSilver());

	// Transfer personal gold into bank.
	
	// Pick up gold.
	EXPECT_TRUE(mInventory->moveCurrency(CurrencySlot::Personal, CurrencySlot::Cursor, CurrencyType::Gold, CurrencyType::Gold, 249));
	EXPECT_EQ(0, mInventory->getPersonalGold());
	EXPECT_EQ(249, mInventory->getCursorGold());
	// Drop on bank platinum.
	EXPECT_TRUE(mInventory->moveCurrency(CurrencySlot::Cursor, CurrencySlot::Bank, CurrencyType::Gold, CurrencyType::Platinum, 240));
	EXPECT_EQ(9, mInventory->getCursorGold());
	EXPECT_EQ(12426, mInventory->getBankPlatinum());
	// Drop on bank gold.
	EXPECT_TRUE(mInventory->moveCurrency(CurrencySlot::Cursor, CurrencySlot::Bank, CurrencyType::Gold, CurrencyType::Gold, 9));
	EXPECT_EQ(0, mInventory->getCursorGold());
	EXPECT_EQ(27, mInventory->getBankGold());

	// Transfer personal platinum into bank.

	// Pick up platinum.
	EXPECT_TRUE(mInventory->moveCurrency(CurrencySlot::Personal, CurrencySlot::Cursor, CurrencyType::Platinum, CurrencyType::Platinum, 67));
	EXPECT_EQ(0, mInventory->getPersonalPlatinum());
	EXPECT_EQ(67, mInventory->getCursorPlatinum());
	// Drop on bank platinum
	EXPECT_TRUE(mInventory->moveCurrency(CurrencySlot::Cursor, CurrencySlot::Bank, CurrencyType::Platinum, CurrencyType::Platinum, 67));
	EXPECT_EQ(0, mInventory->getCursorPlatinum());

	EXPECT_EQ(0, mInventory->getTotalPersonalCurrency()); // All currency should be transfered into bank.

	EXPECT_EQ(12398723 + 97137, mInventory->getTotalCurrency());
	EXPECT_EQ(12398723 + 97137, mInventory->getTotalBankCurrency());
}