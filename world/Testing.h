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

#include "Test_AccountManager.h"
#include "Test_GuildManager.h"
#include "Test_AlternateCurrencyManager.h"
#include "Test_ShopDataStore.h"
#include "Test_ZoneDataStore.h"
#include "Test_NPCFactory.h"
#include "Test_SpellDataStore.h"
//#include "Test_ItemDataStore.h"
#include "Test_ItemFactory.h"
#include "Test_ItemGenerator.h"
#include "Test_CharacterFactory.h"
#include "Test_Transmutation.h"
#include "Test_ZoneConnectionHandlerSanity.h"
#include "Test_WorldConnectionHandlerSanity.h"

TEST(ZonePayload, FixedSizes) {
	EXPECT_EQ(26632, Payload::Zone::CharacterProfile::size());
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
	EXPECT_EQ(44, Payload::Zone::LoadSpellSet::size());
	EXPECT_EQ(8, Payload::Zone::SwapSpell::size());
	EXPECT_EQ(36, Payload::Zone::CastSpell::size());
	EXPECT_EQ(8, Payload::Zone::BeginCast::size());
	EXPECT_EQ(20, Payload::Zone::ManaChange::size());
	EXPECT_EQ(10, Payload::Zone::ManaUpdate::size());
	EXPECT_EQ(10, Payload::Zone::EnduranceUpdate::size());
	// LoadSpellSet
	//TODO: More.

	EXPECT_EQ(8, Payload::Zone::SpawnAppearance::size());
	EXPECT_EQ(16, Payload::Zone::ActorSize::size());
	EXPECT_EQ(4, Payload::Zone::ActorAnimation::size());

	EXPECT_EQ(256, Payload::Zone::AppearanceUpdate::size());
	
	EXPECT_EQ(4292, Payload::Zone::PopupWindow::size());
	EXPECT_EQ(8, Payload::Zone::PopupResponse::size());

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
	EXPECT_EQ(37, Payload::Zone::EnvironmentDamage::size());
	EXPECT_EQ(72, Payload::Zone::ClaimRequest::size());
	EXPECT_EQ(4, Payload::Zone::Camp::size());
	EXPECT_EQ(12, Payload::Zone::PotionBelt::size());
	EXPECT_EQ(4, Payload::Zone::DeleteSpawn::size());
	EXPECT_EQ(8, Payload::Zone::PetCommand::size());
	EXPECT_EQ(368, Payload::Zone::SaveRequest::size());
	EXPECT_EQ(8, Payload::Zone::ApplyPoison::size());
	EXPECT_EQ(24, Payload::Zone::FindPersonRequest::size());
	EXPECT_EQ(8, Payload::Zone::InspectRequest::size());
	EXPECT_EQ(1860, Payload::Zone::InspectResponse::size());
	EXPECT_EQ(256, Payload::Zone::InspectMessage::size());
	EXPECT_EQ(8, Payload::Zone::RemoveBuffRequest::size());

	EXPECT_EQ(4, Payload::Zone::TaskHistoryRequest::size());

	// Group.
	EXPECT_EQ(148, Payload::Group::Invite::size());
	EXPECT_EQ(152, Payload::Group::AcceptInvite::size());
	EXPECT_EQ(152, Payload::Group::DeclineInvite::size());
	EXPECT_EQ(148, Payload::Group::Disband::size());
	EXPECT_EQ(148, Payload::Group::DisbandYou::size());
	EXPECT_EQ(148, Payload::Group::DisbandOther::size());
	EXPECT_EQ(456, Payload::Group::MakeLeader::size());
	EXPECT_EQ(148, Payload::Group::Roles::size());
	EXPECT_EQ(148, Payload::Group::LeaderUpdate::size());

	// Raid.
	EXPECT_EQ(140, Payload::Raid::Invite::size());

	// Guild.
	EXPECT_EQ(64, Payload::Guild::Create::size());
	EXPECT_EQ(68, Payload::Guild::Delete::size());
	EXPECT_EQ(80, Payload::Guild::RankUpdate::size());
	EXPECT_EQ(136, Payload::Guild::Invite::size());
	EXPECT_EQ(136, Payload::Guild::Remove::size());
	EXPECT_EQ(128, Payload::Guild::MakeLeader::size());
	EXPECT_EQ(128, Payload::Guild::Demote::size());
	EXPECT_EQ(136, Payload::Guild::FlagsUpdate::size());
	EXPECT_EQ(648, Payload::Guild::MOTD::size());
	EXPECT_EQ(648, Payload::Guild::MOTDResponse::size());
	EXPECT_EQ(648, Payload::Guild::MOTDRequest::size());
	EXPECT_EQ(4176, Payload::Guild::GuildUpdate::size());
	EXPECT_EQ(388, Payload::Guild::PublicNote::size());
	// MemberJoin
	// LevelUpdate
	EXPECT_EQ(72, Payload::Guild::LevelUpdate::size());
	EXPECT_EQ(78, Payload::Guild::MemberZoneUpdate::size());
	EXPECT_EQ(136, Payload::Guild::StatusRequest::size());

	// Login Server Connection
	
	EXPECT_EQ(12, Payload::LoginServer::WorldStatus::size());
	EXPECT_EQ(16, Payload::LoginServer::ConnectRequest::size());
	EXPECT_EQ(17, Payload::LoginServer::ConnectResponse::size());
	EXPECT_EQ(653, Payload::LoginServer::WorldInformation::size());
	//EXPECT_EQ(20, Payload::LoginServer::ClientAuthentication::size());

	// World

	EXPECT_EQ(464, Payload::World::Connect::size());
	EXPECT_EQ(72, Payload::World::EnterWorld::size());
	EXPECT_EQ(92, Payload::World::CreateCharacter::size());
	// LogServer
	EXPECT_EQ(4, Payload::World::ExpansionInfo::size());
	// CharacterSelect
	EXPECT_EQ(72, Payload::World::NameGeneration::size());
	EXPECT_EQ(544, Payload::World::ApproveWorld::size());

	EXPECT_EQ(76, Payload::World::ApproveName::size());
	EXPECT_EQ(1, Payload::World::ApproveNameResponse::size());
	
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

TEST(isSafe, Strings) {
	// Long String
	char buffer[] = "HelloWorld";
	EXPECT_EQ("", Utility::safeString(buffer, 2));

	// Fail: Null.
	EXPECT_FALSE(Utility::isSafe(nullptr, 0));

	char a[10];
	memset(a, 1, sizeof(a));

	// Fail: Not terminated.
	EXPECT_FALSE(Utility::isSafe(a, 0));
	EXPECT_FALSE(Utility::isSafe(a, 1));
	EXPECT_FALSE(Utility::isSafe(a, 2));
	EXPECT_FALSE(Utility::isSafe(a, 3));
	EXPECT_FALSE(Utility::isSafe(a, 4));
	EXPECT_FALSE(Utility::isSafe(a, 5));
	EXPECT_FALSE(Utility::isSafe(a, 6));

	char name[64];
	memset(name, 1, sizeof(name));
	EXPECT_FALSE(Utility::isSafe(name, 64));

	char name2[64];
	memset(name2, 0, sizeof(name2));
	strcpy(name2, "Drajor");
	EXPECT_TRUE(Utility::isSafe(name2, 64));
}

TEST(containsDigits, Strings) {

	for (auto i = 0; i <= 100; i++)
		EXPECT_TRUE(Utility::containsDigits(toString(i)));

	// Truth table
	std::map<bool, std::string> strings = {
		{ false, "" },
		{ true, "0" },
		{ true, "1" },
		{ false, "testings go!" },
		{ true, "testings 0go!" },
		{ true, "testings go!9" },
		{ true, "te77777stings go!" },
		{ false, "wowowowototototo" },
		{ false, "wowowowototot1to" },
	};

	for (auto i : strings) {
		EXPECT_EQ(i.first, Utility::containsDigits(i.second));
	}

}

TEST(LimitsTest, ShopQuantityValid) {
	EXPECT_TRUE(Limits::Shop::quantityValid(-1));
	EXPECT_FALSE(Limits::Shop::quantityValid(-2));
	EXPECT_FALSE(Limits::Shop::quantityValid(0));
	EXPECT_TRUE(Limits::Shop::quantityValid(1));
	EXPECT_TRUE(Limits::Shop::quantityValid(487422));
}

TEST(SlotIDTests, isPrimarySlot) {
	struct Range {
		bool mResult;
		u32 mStart;
		u32 mEnd;
	};
	std::list<Range> tests = {
		{ true, 0, 22 }, // Worn
		{ true, 23, 30 }, // Main.
		{ false, 31, 1999 }, // Gap.
		{ true, 2000, 2023}, // Bank.
		{ false, 2024, 2499 }, // Gap.
		{ true, 2500, 2501 }, // Shared Bank.
		{ false, 2502, 2999 }, // Gap.
		{ true, 3000, 3007 }, // Trade
		{ false, 3008, 10000 }, // Gap.
	};

	for (auto i : tests) {
		for (auto j = i.mStart; j <= i.mEnd; j++) {
			EXPECT_EQ(i.mResult, SlotID::isPrimarySlot(j));
		}
	}
}

TEST(SlotIDTests, isContainerSlot) {
	struct Range {
		bool mResult;
		u32 mStart;
		u32 mEnd;
	};
	std::list<Range> tests = {
		{ false, 0, 261}, // Gap.
		{ true, 262, 341 }, // Main Contents.
		{ false, 342, 2031 }, // Gap.
		{ true, 2032, 2271 }, // Bank Contents.
		{ false, 2272, 2531 }, // Gap.
		{ true, 2532, 2551 }, // Shared Bank Contents.
		{ false, 2552, 10000 }, // Gap.
	};

	for (auto i : tests) {
		for (auto j = i.mStart; j <= i.mEnd; j++) {
			EXPECT_EQ(i.mResult, SlotID::isContainerSlot(j));
		}
	}
}

TEST(SlotIDTests, getPrimarySlotIndex_PrimaryOnly) {
	struct Test {
		u32 mSlot;
		u32 mPrimaryIndex;
	};

	std::list<Test> tests = {
		{ 0, 0 }, // Charm
		{ 1, 1 }, // Left ear
		{ 2, 2 }, // Head
		{ 3, 3 }, // Face
		{ 4, 4 }, // Right ear
		{ 5, 5 }, // Neck
		{ 6, 6 }, // Shoulders
		{ 7, 7 }, // Arms
		{ 8, 8 }, // Back
		{ 9, 9 }, // Left wrist
		{ 10, 10 }, // Right wrist
		{ 11, 11 }, // Range
		{ 12, 12 }, // Hands
		{ 13, 13 }, // Primary
		{ 14, 14 }, // Secondary
		{ 15, 15 }, // Left ring
		{ 16, 16 }, // Right ring
		{ 17, 17 }, // Chest
		{ 18, 18 }, // Legs
		{ 19, 19 }, // Feet
		{ 20, 20 }, // Waist
		{ 21, 21 }, // Powersource
		{ 22, 22 }, // Ammo

		{ 23, 23 }, // Main 0
		{ 24, 24 }, // Main 1
		{ 25, 25 }, // Main 2
		{ 26, 26 }, // Main 3
		{ 27, 27 }, // Main 4
		{ 28, 28 }, // Main 5
		{ 29, 29 }, // Main 6
		{ 30, 30 }, // Main 7

		{ 2000, 31 }, // Bank 0
		{ 2001, 32 }, // Bank 1
		{ 2002, 33 }, // Bank 2
		{ 2003, 34 }, // Bank 3
		{ 2004, 35 }, // Bank 4
		{ 2005, 36 }, // Bank 5
		{ 2006, 37 }, // Bank 6
		{ 2007, 38 }, // Bank 7
		{ 2008, 39 }, // Bank 8
		{ 2009, 40 }, // Bank 9
		{ 2010, 41 }, // Bank 10
		{ 2011, 42 }, // Bank 11
		{ 2012, 43 }, // Bank 12
		{ 2013, 44 }, // Bank 13
		{ 2014, 45 }, // Bank 14
		{ 2015, 46 }, // Bank 15
		{ 2016, 47 }, // Bank 16
		{ 2017, 48 }, // Bank 17
		{ 2018, 49 }, // Bank 18
		{ 2019, 50 }, // Bank 19
		{ 2020, 51 }, // Bank 20
		{ 2021, 52 }, // Bank 21
		{ 2022, 53 }, // Bank 22
		{ 2023, 54 }, // Bank 23

		{ 2500, 55 }, // Shared Bank 0
		{ 2501, 56 }, // Shared Bank 1

		{ 3000, 57 }, // Trade 0
		{ 3001, 58 }, // Trade 1
		{ 3002, 59 }, // Trade 2
		{ 3003, 60 }, // Trade 3
		{ 3004, 61 }, // Trade 4
		{ 3005, 62 }, // Trade 5
		{ 3006, 63 }, // Trade 6
		{ 3007, 64 }, // Trade 7
	};

	for (auto i : tests) {
		EXPECT_EQ(i.mPrimaryIndex, SlotID::getPrimarySlotIndex(i.mSlot));
	}
}


class ExperienceControllerTest : public ::testing::Test {
protected:
	virtual void SetUp() {
		mController = new Experience::Controller();
	}
	virtual void TearDown() {
		delete mController;
		mController = nullptr;

		Experience::Controller::setRequiredAAExperienceFunction(nullptr);
		Experience::Controller::setRequiredExperienceFunction(nullptr);
	}

	void setExpFunction() {
		static std::function<u32(u8)> expF = [](u8) { return 1; };
		Experience::Controller::setRequiredExperienceFunction(&expF);
	}

	void setAAExpFunction() {
		static std::function<u32(u32)> expAAF = [](u32) { return 1; };
		Experience::Controller::setRequiredAAExperienceFunction(&expAAF);
	}

	void setData(const u8 pLevel, const u8 pMaximumLevel, const u32 pExperience, const u32 pExperienceToAA, const u32 pUnspentAAPoints, const u32 pMaximumUnspentAAPoints, const u32 pSpentAAPoints, const u32 pMaximumSpentAAPoints, const u32 pAAExperience) {
		mData.mLevel = pLevel;
		mData.mMaximumLevel = pMaximumLevel;
		mData.mExperience = pExperience;

		mData.mExperienceToAA = pExperienceToAA;
		mData.mUnspentAAPoints = pUnspentAAPoints;
		mData.mMaximumUnspentAA = pMaximumUnspentAAPoints;
		mData.mSpentAAPoints = pSpentAAPoints;
		mData.mMaximumSpentAA = pMaximumSpentAAPoints;
		mData.mAAExperience = pAAExperience;
	}

	Data::Experience mData;
	Experience::Controller* mController = 0;
};

TEST_F(ExperienceControllerTest, InitialiseFunctions) {
	// Make it fail!
	Experience::Controller::setRequiredAAExperienceFunction(nullptr);
	Experience::Controller::setRequiredExperienceFunction(nullptr);
	setData(1, 10, 2, 20, 3, 4, 4, 5, 6);
	
	EXPECT_FALSE(mController->onLoad(&mData));

	// Set the experience function.
	std::function<u32(u8)> expF = [](u8) { return 1; };
	Experience::Controller::setRequiredExperienceFunction(&expF);
	EXPECT_FALSE(mController->onLoad(&mData));

	// Set the AA experience function.
	std::function<u32(u32)> expAAF = [](u32) { return 1; };
	Experience::Controller::setRequiredAAExperienceFunction(&expAAF);
	EXPECT_FALSE(mController->onLoad(&mData));
}

TEST_F(ExperienceControllerTest, InitialiseParameterChecks) {
	setExpFunction();
	setAAExpFunction();
	// Fail: Zero level.
	setData(0, 2, 3, 70, 4, 5, 6, 7, 8);
	EXPECT_FALSE(mController->onLoad(&mData));

	// Fail: Level greater than max level.
	setData(3, 2, 3, 70, 4, 5, 6, 7, 8);
	EXPECT_FALSE(mController->onLoad(&mData));

	// Fail: Unspent AA greater than max.
	setData(1, 2, 3, 70, 5, 4, 6, 7, 8);
	EXPECT_FALSE(mController->onLoad(&mData));

	// Fail: Spent AA greater than max.
	setData(1, 2, 3, 70, 4, 5, 7, 6, 8);
	EXPECT_FALSE(mController->onLoad(&mData));
}

TEST_F(ExperienceControllerTest, DoubleInitialise) {
	setExpFunction();
	setAAExpFunction();

	setData(1, 10, 2, 20, 3, 4, 5, 6, 7);
	EXPECT_TRUE(mController->onLoad(&mData));
	EXPECT_FALSE(mController->onLoad(&mData));
}

TEST_F(ExperienceControllerTest, InitalisedValues) {
	setExpFunction();
	setAAExpFunction();

	setData(1, 10, 2, 20, 3, 4, 5, 6, 7);
	EXPECT_TRUE(mController->onLoad(&mData));
	EXPECT_EQ(1, mController->getLevel());
	EXPECT_EQ(10, mController->getMaximumLevel());
	EXPECT_EQ(2, mController->getExperience());
	EXPECT_EQ(20, mController->getExperienceToAA());
	EXPECT_EQ(3, mController->getUnspentAA());
	EXPECT_EQ(4, mController->getMaximumUnspentAA());
	EXPECT_EQ(5, mController->getSpentAA());
	EXPECT_EQ(6, mController->getMaximumSpentAA());
	EXPECT_EQ(7, mController->getAAExperience());
}

class ExperienceControllerTestAddExperience : public ::testing::Test {
protected:
	virtual void SetUp() {
		mController = std::make_shared<Experience::Controller>();
		Experience::Controller::setRequiredExperienceFunction(&expF);
		Experience::Controller::setRequiredAAExperienceFunction(&expAAF);

		setData(1, 20, 0, 0, 0, 3, 0, 0, 0);
		mController->onLoad(&mData);
	}
	virtual void TearDown() {
		mController = nullptr;

		Experience::Controller::setRequiredAAExperienceFunction(nullptr);
		Experience::Controller::setRequiredExperienceFunction(nullptr);
	}

	void setData(const u8 pLevel, const u8 pMaximumLevel, const u32 pExperience, const u32 pExperienceToAA, const u32 pUnspentAAPoints, const u32 pMaximumUnspentAAPoints, const u32 pSpentAAPoints, const u32 pMaximumSpentAAPoints, const u32 pAAExperience) {
		mData.mLevel = pLevel;
		mData.mMaximumLevel = pMaximumLevel;
		mData.mExperience = pExperience;

		mData.mExperienceToAA = pExperienceToAA;
		mData.mUnspentAAPoints = pUnspentAAPoints;
		mData.mMaximumUnspentAA = pMaximumUnspentAAPoints;
		mData.mSpentAAPoints = pSpentAAPoints;
		mData.mMaximumSpentAA = pMaximumSpentAAPoints;
		mData.mAAExperience = pAAExperience;
	}

	Data::Experience mData;

	std::function<u32(u8)> expF = [](u8 pLevel) { return pLevel * 5; };
	std::function<u32(u32)> expAAF = [](u32 pPoints) { return 10; };
	std::shared_ptr<Experience::Controller> mController = 0;
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
	EXPECT_FALSE(mController->canGainExperience()); // Can no longer gain experience.


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
	EXPECT_EQ(0, mController->getUnspentAA()); // Start with 0 unspent AA.
	EXPECT_EQ(0, mController->getAAExperience()); // Start with 0 AA experience.

	mController->addAAExperience(4);
	EXPECT_EQ(0, mController->getUnspentAA()); // Still 0 unspent AA
	EXPECT_EQ(4, mController->getAAExperience()); // Gained 4 experience.

	mController->addAAExperience(12);
	EXPECT_EQ(1, mController->getUnspentAA()); // Now 1 unspent AA.
	EXPECT_EQ(6, mController->getAAExperience()); // We expect 3 experience after wrap.

	mController->addAAExperience(998374623); // Really big hit, max unspent points.
	EXPECT_EQ(3, mController->getUnspentAA());
	EXPECT_FALSE(mController->canGainAAExperience()); // Can no longer gain AA experience.
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
	EXPECT_FALSE(mLootController->hasLooter());
	EXPECT_EQ(nullptr, mLootController->getLooter());
	EXPECT_FALSE(mLootController->isOpen());
}

TEST_F(LootControllerTest, Nulls) {
	EXPECT_FALSE( mLootController->canLoot(nullptr));
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
		mInventory = new Inventory();
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

	Inventory* mInventory;
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

//TEST_F(InventoryCurrencyTest, AcceptTrade) {
//	// Setup
//	setupPersonal(120, 87, 23, 933);
//	EXPECT_EQ((120 * 1000) + (87 * 100) + (23 * 10) + 933, mInventory->getTotalPersonalCurrency());
//	EXPECT_EQ(mInventory->getTotalPersonalCurrency(), mInventory->getTotalCurrency());
//
//	setupTrade(43, 0, 34, 51);
//	EXPECT_EQ((43 * 1000) + (34 * 10) + 51, mInventory->getTotalTradeCurrency());
//	EXPECT_EQ(mInventory->getTotalCurrency(), mInventory->getTotalPersonalCurrency() + mInventory->getTotalTradeCurrency());
//
//	EXPECT_TRUE(mInventory->onTradeAccept());
//
//	EXPECT_EQ(0, mInventory->getTotalTradeCurrency()); // Trade currency should be clear.
//	EXPECT_EQ((120 * 1000) + (87 * 100) + (23 * 10) + 933, mInventory->getTotalPersonalCurrency());
//}
//
//TEST_F(InventoryCurrencyTest, CancelTrade) {
//	// Setup
//	EXPECT_TRUE(mInventory->addCurrency(CurrencySlot::Personal, CurrencyType::Copper, 897));
//	EXPECT_TRUE(mInventory->addCurrency(CurrencySlot::Personal, CurrencyType::Silver, 434));
//	EXPECT_TRUE(mInventory->addCurrency(CurrencySlot::Personal, CurrencyType::Gold, 249));
//	EXPECT_TRUE(mInventory->addCurrency(CurrencySlot::Personal, CurrencyType::Platinum, 67));
//
//	EXPECT_EQ(97137, mInventory->getTotalPersonalCurrency());
//
//	// Move platinum into trade.
//	EXPECT_TRUE(mInventory->moveCurrency(CurrencySlot::Personal, CurrencySlot::Cursor, CurrencyType::Platinum, CurrencyType::Platinum, 67));
//	EXPECT_TRUE(mInventory->moveCurrency(CurrencySlot::Cursor, CurrencySlot::Trade, CurrencyType::Platinum, CurrencyType::Platinum, 67));
//
//	// Move gold into trade.
//	EXPECT_TRUE(mInventory->moveCurrency(CurrencySlot::Personal, CurrencySlot::Cursor, CurrencyType::Gold, CurrencyType::Gold, 249));
//	EXPECT_TRUE(mInventory->moveCurrency(CurrencySlot::Cursor, CurrencySlot::Trade, CurrencyType::Gold, CurrencyType::Gold, 249));
//
//	// Move silver into trade.
//	EXPECT_TRUE(mInventory->moveCurrency(CurrencySlot::Personal, CurrencySlot::Cursor, CurrencyType::Silver, CurrencyType::Silver, 434));
//	EXPECT_TRUE(mInventory->moveCurrency(CurrencySlot::Cursor, CurrencySlot::Trade, CurrencyType::Silver, CurrencyType::Silver, 434));
//
//	// Move copper into trade.
//	EXPECT_TRUE(mInventory->moveCurrency(CurrencySlot::Personal, CurrencySlot::Cursor, CurrencyType::Copper, CurrencyType::Copper, 897));
//	EXPECT_TRUE(mInventory->moveCurrency(CurrencySlot::Cursor, CurrencySlot::Trade, CurrencyType::Copper, CurrencyType::Copper, 897));
//
//	EXPECT_EQ(0, mInventory->getTotalPersonalCurrency());
//	EXPECT_EQ(97137, mInventory->getTotalCurrency());
//	EXPECT_EQ(97137, mInventory->getTotalTradeCurrency());
//
//	// Cancel trade.
//	EXPECT_TRUE(mInventory->onTradeCancel());
//
//	EXPECT_EQ(97137, mInventory->getTotalPersonalCurrency());
//	EXPECT_EQ(97137, mInventory->getTotalCurrency());
//	EXPECT_EQ(0, mInventory->getTotalTradeCurrency()); // Trade currency should be clear.
//}

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
		mInventory = new Inventory();
	}

	virtual void TearDown() {
		delete mInventory;
		mInventory = nullptr;
	}

	Inventory* mInventory;
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