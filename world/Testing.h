#pragma once

#include "Constants.h"
#include "Data.h"
#include "Utility.h"
#include "Limits.h"
#include "Inventory.h"
#include "LootController.h"
#include "Payload.h"
#include "NPC.h"
#include "ExperienceController.h"

#include "gtest/gtest.h"

TEST(ZonePayload, FixedSizes) {
	EXPECT_EQ(944, Payload::Zone::ZoneData::size());
	EXPECT_EQ(8, Payload::Zone::Time::size());
	EXPECT_EQ(68, Payload::Zone::ZoneEntry::size());
	EXPECT_EQ(28, Payload::Zone::AddNimbus::size());
	EXPECT_EQ(8, Payload::Zone::RemoveNimbus::size());
	EXPECT_EQ(8, Payload::Zone::SetTitle::size());
	EXPECT_EQ(40, Payload::Zone::TitleUpdate::size());
	EXPECT_EQ(24, Payload::Zone::FaceChange::size());
	// WearChange
	EXPECT_EQ(4, Payload::Zone::AutoAttack::size());
	EXPECT_EQ(16, Payload::Zone::MemoriseSpell::size());
	EXPECT_EQ(8, Payload::Zone::DeleteSpell::size());
	EXPECT_EQ(40, Payload::Zone::LoadSpellSet::size());
	EXPECT_EQ(8, Payload::Zone::SwapSpell::size());
	EXPECT_EQ(36, Payload::Zone::CastSpell::size());
	EXPECT_EQ(8, Payload::Zone::BeginCast::size());
	EXPECT_EQ(20, Payload::Zone::ManaChange::size());
	EXPECT_EQ(10, Payload::Zone::ManaUpdate::size());
	EXPECT_EQ(10, Payload::Zone::EnduranceUpdate::size());
	// LoadSpellSet
	//TODO: More.

	EXPECT_EQ(256, Payload::Zone::AppearanceUpdate::size());
	
	// AugmentInformation
	EXPECT_EQ(8212, Payload::Zone::BookRequest::size());
	EXPECT_EQ(4, Payload::Zone::Combine::size());
	EXPECT_EQ(16, Payload::Zone::ShopRequest::size());
	EXPECT_EQ(8, Payload::Zone::ShopEnd::size());
	EXPECT_EQ(16, Payload::Zone::ShopSell::size());
	EXPECT_EQ(32, Payload::Zone::ShopBuy::size());
	// ShopDeleteItem
	EXPECT_EQ(88, Payload::Zone::UpdateAlternateCurrency::size());
	EXPECT_EQ(16, Payload::Zone::AlternateCurrencyReclaim::size());
	EXPECT_EQ(8, Payload::Zone::RandomRequest::size());
	EXPECT_EQ(76, Payload::Zone::RandomReply::size());
	EXPECT_EQ(8, Payload::Zone::ExperienceUpdate::size());
	EXPECT_EQ(12, Payload::Zone::AAExperienceUpdate::size());
	EXPECT_EQ(28, Payload::Zone::LeadershipExperienceUpdate::size());
	EXPECT_EQ(12, Payload::Zone::LevelUpdate::size());
	EXPECT_EQ(64, Payload::Zone::LevelAppearance::size());
	EXPECT_EQ(1028, Payload::Zone::Emote::size());

	EXPECT_EQ(156, Payload::Zone::WhoRequest::size());
	EXPECT_EQ(16, Payload::Zone::AAAction::size());
	EXPECT_EQ(1, Payload::Zone::LeadershipExperienceToggle::size());

	// Login Server Connection
	
	EXPECT_EQ(12, Payload::LoginServer::WorldStatus::size());
	EXPECT_EQ(16, Payload::LoginServer::ConnectRequest::size());
	EXPECT_EQ(17, Payload::LoginServer::ConnectResponse::size());
	EXPECT_EQ(653, Payload::LoginServer::WorldInformation::size());
	//EXPECT_EQ(20, Payload::LoginServer::ClientAuthentication::size());

	// World

	EXPECT_EQ(464, Payload::World::Connect::size());
	EXPECT_EQ(72, Payload::World::EnterWorld::size());
	EXPECT_EQ(88, Payload::World::CreateCharacter::size());
	// LogServer
	EXPECT_EQ(4, Payload::World::ExpansionInfo::size());
	// CharacterSelect
	EXPECT_EQ(72, Payload::World::NameGeneration::size());
	EXPECT_EQ(544, Payload::World::ApproveWorld::size());
	
}

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

// Utility
TEST(safeString, Strings) {
	// Long String
	char buffer[] = "HelloWorld";
	EXPECT_EQ("", Utility::safeString(buffer, 2));
}

TEST(LimitsTest, ShopQuantityValid) {
	EXPECT_TRUE(Limits::Shop::quantityValid(-1));
	EXPECT_FALSE(Limits::Shop::quantityValid(-2));
	EXPECT_FALSE(Limits::Shop::quantityValid(0));
	EXPECT_TRUE(Limits::Shop::quantityValid(1));
	EXPECT_TRUE(Limits::Shop::quantityValid(487422));
}

class ExperienceControllerTest : public ::testing::Test {
protected:
	virtual void SetUp() {
		mController = new ExperienceController();
	}
	virtual void TearDown() {
		delete mController;
		mController = nullptr;

		ExperienceController::setRequiredAAExperienceFunction(nullptr);
		ExperienceController::setRequiredExperienceFunction(nullptr);
		ExperienceController::setRequiredGroupExperienceFunction(nullptr);
		ExperienceController::setRequiredRaidExperienceFunction(nullptr);
	}

	void setExpFunction() {
		static std::function<u32(u8)> expF = [](u8) { return 1; };
		ExperienceController::setRequiredExperienceFunction(&expF);
	}

	void setAAExpFunction() {
		static std::function<u32(u32)> expAAF = [](u32) { return 1; };
		ExperienceController::setRequiredAAExperienceFunction(&expAAF);
	}

	void setGroupExpFunction() {
		static std::function<u32(u32)> expGLF = [](u32) { return 1; };
		ExperienceController::setRequiredGroupExperienceFunction(&expGLF);
	}

	void setRaidExpFunction() {
		static std::function<u32(u32)> expRLF = [](u32) { return 1; };
		ExperienceController::setRequiredRaidExperienceFunction(&expRLF);
	}

	void setData(const u8 pLevel, const u8 pMaximumLevel, const u32 pExperience, const u32 pExperienceToAA, const u32 pUnspentAAPoints, const u32 pMaximumUnspentAAPoints, const u32 pSpentAAPoints, const u32 pMaximumSpentAAPoints, const u32 pAAExperience, const u32 pGroupExperience, const u32 pGroupPoints, const u32 pRaidExperience, const u32 pRaidPoints) {
		mData.mLevel = pLevel;
		mData.mMaximumLevel = pMaximumLevel;
		mData.mExperience = pExperience;

		mData.mExperienceToAA = pExperienceToAA;
		mData.mUnspentAAPoints = pUnspentAAPoints;
		mData.mMaximumUnspentAA = pMaximumUnspentAAPoints;
		mData.mSpentAAPoints = pSpentAAPoints;
		mData.mMaximumSpentAA = pMaximumSpentAAPoints;
		mData.mAAExperience = pAAExperience;

		mData.mGroupExperience = pGroupExperience;
		mData.mGroupPoints = pGroupPoints;

		mData.mRaidExperience = pRaidExperience;
		mData.mRaidPoints = pRaidPoints;
	}

	Data::Experience mData;
	ExperienceController* mController = 0;
};

TEST_F(ExperienceControllerTest, InitialiseFunctions) {
	// Make it fail!
	ExperienceController::setRequiredAAExperienceFunction(nullptr);
	ExperienceController::setRequiredExperienceFunction(nullptr);
	ExperienceController::setRequiredGroupExperienceFunction(nullptr);
	ExperienceController::setRequiredRaidExperienceFunction(nullptr);
	setData(1, 10, 2, 20, 3, 4, 4, 5, 6, 7, 1, 6, 2);
	
	EXPECT_EQ(false, mController->initalise(mData));

	// Set the experience function.
	std::function<u32(u8)> expF = [](u8) { return 1; };
	ExperienceController::setRequiredExperienceFunction(&expF);
	EXPECT_EQ(false, mController->initalise(mData));

	// Set the AA experience function.
	std::function<u32(u32)> expAAF = [](u32) { return 1; };
	ExperienceController::setRequiredAAExperienceFunction(&expAAF);
	EXPECT_EQ(false, mController->initalise(mData));

	// Set the Group Leadership experience function.
	std::function<u32(u32)> expGLF = [](u32) { return 1; };
	ExperienceController::setRequiredGroupExperienceFunction(&expGLF);
	EXPECT_EQ(false, mController->initalise(mData));

	// Set the Raid Leadership experience function. (should succeed)
	std::function<u32(u32)> expRLF = [](u32) { return 1; };
	ExperienceController::setRequiredRaidExperienceFunction(&expRLF);
	EXPECT_EQ(true, mController->initalise(mData));
}

TEST_F(ExperienceControllerTest, InitialiseParameterChecks) {
	setExpFunction();
	setAAExpFunction();
	setGroupExpFunction();
	setRaidExpFunction();

	// Fail: Zero level.
	setData(0, 2, 3, 70, 4, 5, 6, 7, 8, 9, 10, 11, 12);
	EXPECT_EQ(false, mController->initalise(mData));

	// Fail: Level greater than max level.
	setData(3, 2, 3, 70, 4, 5, 6, 7, 8, 9, 10, 11, 12);
	EXPECT_EQ(false, mController->initalise(mData));

	// Fail: Unspent AA greater than max.
	setData(1, 2, 3, 70, 5, 4, 6, 7, 8, 9, 10, 11, 12);
	EXPECT_EQ(false, mController->initalise(mData));

	// Fail: Spent AA greater than max.
	setData(1, 2, 3, 70, 4, 5, 7, 6, 8, 9, 10, 11, 12);
	EXPECT_EQ(false, mController->initalise(mData));
}

TEST_F(ExperienceControllerTest, DoubleInitialise) {
	setExpFunction();
	setAAExpFunction();
	setGroupExpFunction();
	setRaidExpFunction();

	setData(1, 10, 2, 20, 3, 4, 5, 6, 7, 8, 9, 10, 11);
	EXPECT_EQ(true, mController->initalise(mData));
	EXPECT_EQ(false, mController->initalise(mData));
}

TEST_F(ExperienceControllerTest, InitalisedValues) {
	setExpFunction();
	setAAExpFunction();
	setGroupExpFunction();
	setRaidExpFunction();

	setData(1, 10, 2, 20, 3, 4, 5, 6, 7, 8, 9, 10, 11);
	EXPECT_EQ(true, mController->initalise(mData));
	EXPECT_EQ(1, mController->getLevel());
	EXPECT_EQ(10, mController->getMaximumLevel());
	EXPECT_EQ(2, mController->getExperience());
	EXPECT_EQ(20, mController->getExperienceToAA());
	EXPECT_EQ(3, mController->getUnspentAAPoints());
	EXPECT_EQ(4, mController->getMaximumUnspentAAPoints());
	EXPECT_EQ(5, mController->getSpentAAPoints());
	EXPECT_EQ(6, mController->getMaximumSpentAAPoints());
	EXPECT_EQ(7, mController->getAAExperience());
	EXPECT_EQ(8, mController->getGroupExperience());
	EXPECT_EQ(9, mController->getGroupPoints());
	EXPECT_EQ(10, mController->getRaidExperience());
	EXPECT_EQ(11, mController->getRaidPoints());
}

class ExperienceControllerTestAddExperience : public ::testing::Test {
protected:
	virtual void SetUp() {
		mController = std::make_shared<ExperienceController>();
		ExperienceController::setRequiredExperienceFunction(&expF);
		ExperienceController::setRequiredAAExperienceFunction(&expAAF);
		ExperienceController::setRequiredGroupExperienceFunction(&expGF);
		ExperienceController::setRequiredRaidExperienceFunction(&expRF);

		setData(1, 20, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0);
		mController->initalise(mData);
	}
	virtual void TearDown() {
		mController = nullptr;

		ExperienceController::setRequiredAAExperienceFunction(nullptr);
		ExperienceController::setRequiredExperienceFunction(nullptr);
	}

	void setData(const u8 pLevel, const u8 pMaximumLevel, const u32 pExperience, const u32 pExperienceToAA, const u32 pUnspentAAPoints, const u32 pMaximumUnspentAAPoints, const u32 pSpentAAPoints, const u32 pMaximumSpentAAPoints, const u32 pAAExperience, const u32 pGroupExperience, const u32 pGroupPoints, const u32 pRaidExperience, const u32 pRaidPoints) {
		mData.mLevel = pLevel;
		mData.mMaximumLevel = pMaximumLevel;
		mData.mExperience = pExperience;

		mData.mExperienceToAA = pExperienceToAA;
		mData.mUnspentAAPoints = pUnspentAAPoints;
		mData.mMaximumUnspentAA = pMaximumUnspentAAPoints;
		mData.mSpentAAPoints = pSpentAAPoints;
		mData.mMaximumSpentAA = pMaximumSpentAAPoints;
		mData.mAAExperience = pAAExperience;

		mData.mGroupExperience = pGroupExperience;
		mData.mGroupPoints = pGroupPoints;

		mData.mRaidExperience = pRaidExperience;
		mData.mRaidPoints = pRaidPoints;
	}

	Data::Experience mData;

	std::function<u32(u8)> expF = [](u8 pLevel) { return pLevel * 5; };
	std::function<u32(u32)> expAAF = [](u32 pPoints) { return 10; };
	std::function<u32(u32)> expGF = [](u32 pPoints) { return 5; };
	std::function<u32(u32)> expRF = [](u32 pPoints) { return 5; };
	std::shared_ptr<ExperienceController> mController = 0;
};

TEST_F(ExperienceControllerTestAddExperience, Adding) {
	EXPECT_EQ(1, mController->getLevel()); // Start at level 1.
	EXPECT_EQ(0, mController->getExperience()); // Start with 0 experience.

	mController->addExperience(4);
	EXPECT_EQ(1, mController->getLevel()); // Still level 1.
	EXPECT_EQ(4, mController->getExperience()); // Gained 4 experience.

	mController->addExperience(5);
	EXPECT_EQ(1, mController->getLevel()); // Still level 1.
	EXPECT_EQ(9, mController->getExperience()); // Gained 9 experience.

	mController->addExperience(7);
	EXPECT_EQ(2, mController->getLevel()); // Now level 2.
	EXPECT_EQ(6, mController->getExperience()); // After experience wrap, we should be 6 experience into level 2.

	mController->addExperience(70); // Big exp hit.
	EXPECT_EQ(5, mController->getLevel()); // Now level 5.
	EXPECT_EQ(16, mController->getExperience()); // After experience wrap, we should be 6 experience into level 5.

	mController->addExperience(998374623); // Really big hit, max level.
	EXPECT_EQ(mController->getMaximumLevel(), mController->getLevel());
	EXPECT_EQ(mController->getExperienceCap(), mController->getExperience()); // Experience is capped.
	EXPECT_EQ(false, mController->canGainExperience()); // Can no longer gain experience.


	mController->setLevel(1);
	EXPECT_EQ(1, mController->getLevel()); // Back to level 1.
	EXPECT_EQ(0, mController->getExperience()); // Experience wiped.
	
	// Try setting to zero.
	mController->setLevel(0);
	EXPECT_EQ(1, mController->getLevel()); // Still level 1.
	EXPECT_EQ(0, mController->getExperience()); // Experience wiped.

	// Try setting to over max.
	mController->setLevel(mController->getMaximumLevel() + 1);
	EXPECT_EQ(mController->getMaximumLevel(), mController->getLevel()); // At max level.
	EXPECT_EQ(0, mController->getExperience()); // Experience wiped.
}

TEST_F(ExperienceControllerTestAddExperience, AddingAAExperience) {
	EXPECT_EQ(0, mController->getUnspentAAPoints()); // Start with 0 unspent AA.
	EXPECT_EQ(0, mController->getAAExperience()); // Start with 0 AA experience.

	mController->addAAExperience(4);
	EXPECT_EQ(0, mController->getUnspentAAPoints()); // Still 0 unspent AA
	EXPECT_EQ(4, mController->getAAExperience()); // Gained 4 experience.

	mController->addAAExperience(12);
	EXPECT_EQ(1, mController->getUnspentAAPoints()); // Now 1 unspent AA.
	EXPECT_EQ(6, mController->getAAExperience()); // We expect 3 experience after wrap.

	mController->addAAExperience(998374623); // Really big hit, max unspent points.
	EXPECT_EQ(3, mController->getUnspentAAPoints());
	EXPECT_EQ(false, mController->canGainAAExperience()); // Can no longer gain AA experience.
}

TEST_F(ExperienceControllerTestAddExperience, AddingGroupLeadershipExperience) {
	EXPECT_EQ(1, mController->getLevel()); // Level 1 (Max group points depends on level).
	EXPECT_EQ(0, mController->getGroupPoints()); // Start with 0 unspent group points.
	EXPECT_EQ(0, mController->getGroupExperience()); // Start with 0 group experience.

	mController->addGroupExperience(3);
	EXPECT_EQ(0, mController->getGroupPoints()); // Still 0 group points.
	EXPECT_EQ(3, mController->getGroupExperience()); // Now 3 group experience.

	mController->addGroupExperience(4);
	EXPECT_EQ(1, mController->getGroupPoints()); // Now 1 group points.
	EXPECT_EQ(2, mController->getGroupExperience()); // Now 2 group experience after wrap.

	mController->addGroupExperience(12312312);
	EXPECT_EQ(mController->getMaxGroupPoints(), mController->getGroupPoints()); // At max group points.
	EXPECT_EQ(mController->getGroupExperienceForNextPoint() - 1, mController->getGroupExperience()); // At group experience cap.
}

TEST_F(ExperienceControllerTestAddExperience, AddingRaidLeadershipExperience) {
	EXPECT_EQ(1, mController->getLevel()); // Level 1 (Max raid points depends on level).
	EXPECT_EQ(0, mController->getRaidPoints()); // Start with 0 unspent raid points.
	EXPECT_EQ(0, mController->getRaidExperience()); // Start with 0 raid experience.

	mController->addRaidExperience(3);
	EXPECT_EQ(0, mController->getRaidPoints()); // Still 0 raid points.
	EXPECT_EQ(3, mController->getRaidExperience()); // Now 3 raid experience.

	mController->addRaidExperience(4);
	EXPECT_EQ(1, mController->getRaidPoints()); // Now 1 raid points.
	EXPECT_EQ(2, mController->getRaidExperience()); // Now 2 raid experience after wrap.

	mController->addRaidExperience(12312312);
	EXPECT_EQ(mController->getMaxRaidPoints(), mController->getRaidPoints()); // At max raid points.
	EXPECT_EQ(mController->getRaidExperienceForNextPoint() - 1, mController->getRaidExperience()); // At raid experience cap.
}

class LootControllerTest : public ::testing::Test {
protected:
	virtual void SetUp() {
		mLootController = new LootController();
	}
	virtual void TearDown() {
		delete mLootController;
		mLootController = nullptr;
	}

	LootController* mLootController;
};

TEST_F(LootControllerTest, Defaults) {
	EXPECT_EQ(false, mLootController->hasLooter());
	EXPECT_EQ(nullptr, mLootController->getLooter());
	EXPECT_EQ(false, mLootController->isOpen());
}

TEST_F(LootControllerTest, Nulls) {
	mLootController->addLooter(nullptr);
	mLootController->removeLooter(nullptr);
	EXPECT_EQ(false,  mLootController->canLoot(nullptr));
	EXPECT_EQ(false, mLootController->isLooter(nullptr));
	mLootController->configure(nullptr);
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
	}

	virtual void TearDown() {
		delete mInventory;
		mInventory = nullptr;
	}

	void setupPersonal(const int32 pPlatinum, const int32 pGold, const int32 pSilver, const int32 pCopper) {
		_setup(CurrencySlot::Personal, pPlatinum, pGold, pSilver, pCopper);
	}

	void setupTrade(const int32 pPlatinum, const int32 pGold, const int32 pSilver, const int32 pCopper) {
		_setup(CurrencySlot::Trade, pPlatinum, pGold, pSilver, pCopper);
	}

	void _setup(const uint32 pSlot, const int32 pPlatinum, const int32 pGold, const int32 pSilver, const int32 pCopper) {
		mInventory->setCurrency(pSlot, CurrencyType::Platinum, pPlatinum);
		mInventory->setCurrency(pSlot, CurrencyType::Gold, pGold);
		mInventory->setCurrency(pSlot, CurrencyType::Silver, pSilver);
		mInventory->setCurrency(pSlot, CurrencyType::Copper, pCopper);
	}

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

TEST_F(InventoryCurrencyTest, MoveDown) {
	// Setup
	EXPECT_TRUE(mInventory->addCurrency(CurrencySlot::Personal, CurrencyType::Copper, 897));
	EXPECT_TRUE(mInventory->addCurrency(CurrencySlot::Personal, CurrencyType::Silver, 434));
	EXPECT_TRUE(mInventory->addCurrency(CurrencySlot::Personal, CurrencyType::Gold, 249));
	EXPECT_TRUE(mInventory->addCurrency(CurrencySlot::Personal, CurrencyType::Platinum, 67));

	EXPECT_EQ(97137, mInventory->getTotalPersonalCurrency());

	EXPECT_TRUE(mInventory->moveCurrency(CurrencySlot::Personal, CurrencySlot::Cursor, CurrencyType::Platinum, CurrencyType::Platinum, 67)); // Pick up
	EXPECT_TRUE(mInventory->moveCurrency(CurrencySlot::Cursor, CurrencySlot::Personal, CurrencyType::Platinum, CurrencyType::Gold, 67));
	EXPECT_TRUE(mInventory->moveCurrency(CurrencySlot::Personal, CurrencySlot::Cursor, CurrencyType::Gold, CurrencyType::Gold, 670 + 249)); // Pick up
	EXPECT_TRUE(mInventory->moveCurrency(CurrencySlot::Cursor, CurrencySlot::Personal, CurrencyType::Gold, CurrencyType::Silver, 670 + 249));
	EXPECT_TRUE(mInventory->moveCurrency(CurrencySlot::Personal, CurrencySlot::Cursor, CurrencyType::Silver, CurrencyType::Silver, 6700 + 2490 + 434)); // Pick up
	EXPECT_TRUE(mInventory->moveCurrency(CurrencySlot::Cursor, CurrencySlot::Personal, CurrencyType::Silver, CurrencyType::Copper, 6700 + 2490 + 434));
	EXPECT_TRUE(mInventory->moveCurrency(CurrencySlot::Personal, CurrencySlot::Cursor, CurrencyType::Copper, CurrencyType::Copper, 67000 + 24900 + 4340 + 897)); // Pick up

	EXPECT_EQ(97137, mInventory->getTotalCurrency());
	EXPECT_EQ(97137, mInventory->getTotalCursorCurrency());
}

TEST_F(InventoryCurrencyTest, AcceptTrade) {
	// Setup
	setupPersonal(120, 87, 23, 933);
	EXPECT_EQ((120 * 1000) + (87 * 100) + (23 * 10) + 933, mInventory->getTotalPersonalCurrency());
	EXPECT_EQ(mInventory->getTotalPersonalCurrency(), mInventory->getTotalCurrency());

	setupTrade(43, 0, 34, 51);
	EXPECT_EQ((43 * 1000) + (34 * 10) + 51, mInventory->getTotalTradeCurrency());
	EXPECT_EQ(mInventory->getTotalCurrency(), mInventory->getTotalPersonalCurrency() + mInventory->getTotalTradeCurrency());

	EXPECT_TRUE(mInventory->onTradeAccept());

	EXPECT_EQ(0, mInventory->getTotalTradeCurrency()); // Trade currency should be clear.
	EXPECT_EQ((120 * 1000) + (87 * 100) + (23 * 10) + 933, mInventory->getTotalPersonalCurrency());
}

TEST_F(InventoryCurrencyTest, CancelTrade) {
	// Setup
	EXPECT_TRUE(mInventory->addCurrency(CurrencySlot::Personal, CurrencyType::Copper, 897));
	EXPECT_TRUE(mInventory->addCurrency(CurrencySlot::Personal, CurrencyType::Silver, 434));
	EXPECT_TRUE(mInventory->addCurrency(CurrencySlot::Personal, CurrencyType::Gold, 249));
	EXPECT_TRUE(mInventory->addCurrency(CurrencySlot::Personal, CurrencyType::Platinum, 67));

	EXPECT_EQ(97137, mInventory->getTotalPersonalCurrency());

	// Move platinum into trade.
	EXPECT_TRUE(mInventory->moveCurrency(CurrencySlot::Personal, CurrencySlot::Cursor, CurrencyType::Platinum, CurrencyType::Platinum, 67));
	EXPECT_TRUE(mInventory->moveCurrency(CurrencySlot::Cursor, CurrencySlot::Trade, CurrencyType::Platinum, CurrencyType::Platinum, 67));

	// Move gold into trade.
	EXPECT_TRUE(mInventory->moveCurrency(CurrencySlot::Personal, CurrencySlot::Cursor, CurrencyType::Gold, CurrencyType::Gold, 249));
	EXPECT_TRUE(mInventory->moveCurrency(CurrencySlot::Cursor, CurrencySlot::Trade, CurrencyType::Gold, CurrencyType::Gold, 249));

	// Move silver into trade.
	EXPECT_TRUE(mInventory->moveCurrency(CurrencySlot::Personal, CurrencySlot::Cursor, CurrencyType::Silver, CurrencyType::Silver, 434));
	EXPECT_TRUE(mInventory->moveCurrency(CurrencySlot::Cursor, CurrencySlot::Trade, CurrencyType::Silver, CurrencyType::Silver, 434));

	// Move copper into trade.
	EXPECT_TRUE(mInventory->moveCurrency(CurrencySlot::Personal, CurrencySlot::Cursor, CurrencyType::Copper, CurrencyType::Copper, 897));
	EXPECT_TRUE(mInventory->moveCurrency(CurrencySlot::Cursor, CurrencySlot::Trade, CurrencyType::Copper, CurrencyType::Copper, 897));

	EXPECT_EQ(0, mInventory->getTotalPersonalCurrency());
	EXPECT_EQ(97137, mInventory->getTotalCurrency());
	EXPECT_EQ(97137, mInventory->getTotalTradeCurrency());

	// Cancel trade.
	EXPECT_TRUE(mInventory->onTradeCancel());

	EXPECT_EQ(97137, mInventory->getTotalPersonalCurrency());
	EXPECT_EQ(97137, mInventory->getTotalCurrency());
	EXPECT_EQ(0, mInventory->getTotalTradeCurrency()); // Trade currency should be clear.
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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class InventoryAlternateCurrencyTest : public ::testing::Test {
protected:
	virtual void SetUp() {
		mInventory = new Inventoryy();
	}

	virtual void TearDown() {
		delete mInventory;
		mInventory = nullptr;
	}

	Inventoryy* mInventory;
};

TEST_F(InventoryAlternateCurrencyTest, General) {
	
	// Add
	mInventory->addAlternateCurrency(1, 12);
	mInventory->addAlternateCurrency(2, 57);
	EXPECT_EQ(12, mInventory->getAlternateCurrencyQuantity(1));

	mInventory->addAlternateCurrency(1, 4);
	EXPECT_EQ(16, mInventory->getAlternateCurrencyQuantity(1));

	// Remove
	mInventory->removeAlternateCurrency(1, 7);
	EXPECT_EQ(9, mInventory->getAlternateCurrencyQuantity(1));

	mInventory->removeAlternateCurrency(1, 3);
	EXPECT_EQ(6, mInventory->getAlternateCurrencyQuantity(1));

	// Set
	mInventory->setAlternateCurrencyQuantity(1, 999);
	EXPECT_EQ(999, mInventory->getAlternateCurrencyQuantity(1));

	// Get
	auto currencies = mInventory->getAlternateCurrency();
	EXPECT_EQ(2, currencies.size());
	EXPECT_EQ(57, currencies[2]);
	EXPECT_EQ(999, currencies[1]);
}