#pragma once
#include "gtest/gtest.h"

#include "Test_Utility.h"

#include "ZoneConnection.h"
#include "GuildManager.h"
#include "Zone.h"

class ZoneConnectionHandlerSanityTest : public ::testing::Test {
protected:
	virtual void SetUp() {
		mLogFactory = new NullLogFactory();
		mGuildManager = new GuildManager();
		mZoneConnection = new ZoneConnection();
		mZone = new Zone(0, 0, 0);
		mStreamInterface = new NullEQStreamInterface();
	}

	virtual void TearDown() {
		delete mLogFactory;
		mLogFactory = nullptr;

		delete mZone;
		mZone = nullptr;

		delete mGuildManager;
		mGuildManager = nullptr;

		delete mZoneConnection;
		mZoneConnection = nullptr;

		delete mStreamInterface;
		mStreamInterface = nullptr;
	}

	bool initialise() {
		return mZoneConnection->initialise(mStreamInterface, mLogFactory, mZone, mGuildManager);
	}

	EQApplicationPacket* makePacket(const u32 pSize) {
		return new EQApplicationPacket(EmuOpcode::OP_Unknown, nullptr, pSize);
	}

	NullEQStreamInterface* mStreamInterface = nullptr;
	NullLogFactory* mLogFactory = nullptr;
	Zone* mZone = nullptr;
	GuildManager* mGuildManager = nullptr;
	ZoneConnection* mZoneConnection = nullptr;
};

/*
		handleZoneEntry
*/ 

TEST_F(ZoneConnectionHandlerSanityTest, handleZoneEntry_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleZoneEntry(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleZoneEntry_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::ZoneEntry::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleZoneEntry(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleZoneEntry_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::ZoneEntry::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleZoneEntry(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleZoneEntry_Overflow_0) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::ZoneEntry::size());
	auto payload = Payload::Zone::ZoneEntry::convert(p);
	// Setup string with no null termination.
	memset(payload->mCharacterName, 1, sizeof(payload->mCharacterName));
	
	EXPECT_FALSE(mZoneConnection->hasStringError());
	EXPECT_FALSE(mZoneConnection->handleZoneEntry(p));
	EXPECT_TRUE(mZoneConnection->hasStringError());

	delete p;
}

/*
		handleRequestClientSpawn
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleRequestClientSpawn_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleRequestClientSpawn(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleRequestClientSpawn_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleRequestClientSpawn(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleClientReady
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleClientReady_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleClientReady(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleClientReady_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleClientReady(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleRequestNewZoneData
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleRequestNewZoneData_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleRequestNewZoneData(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleRequestNewZoneData_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleRequestNewZoneData(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleClientUpdate
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleClientUpdate_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleClientUpdate(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleClientUpdate_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::PositionUpdate::size() + 2); // NOTE: +2 is used due to slight variation in payload size.
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleClientUpdate(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleClientUpdate_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::PositionUpdate::size() - 2); // NOTE: -2 is used due to slight variation in payload size.
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleClientUpdate(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleSpawnAppearance
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleSpawnAppearance_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleSpawnAppearance(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleSpawnAppearance_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::SpawnAppearance::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleSpawnAppearance(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleSpawnAppearance_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::SpawnAppearance::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleSpawnAppearance(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleTarget
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleTarget_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleTarget(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleTarget_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::Target::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleTarget(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleTarget_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::Target::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleTarget(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleGroupInvite
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleGroupInvite_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleGroupInvite(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGroupInvite_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Group::Invite::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleGroupInvite(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGroupInvite_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Group::Invite::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleGroupInvite(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGroupInvite_Overflow_0) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Group::Invite::size());
	auto payload = Payload::Group::Invite::convert(p);
	// Setup string with no null termination.
	memset(payload->mFrom, 1, sizeof(payload->mFrom));

	EXPECT_FALSE(mZoneConnection->hasStringError());
	EXPECT_FALSE(mZoneConnection->handleGroupInvite(p));
	EXPECT_TRUE(mZoneConnection->hasStringError());

	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGroupInvite_Overflow_1) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Group::Invite::size());
	auto payload = Payload::Group::Invite::convert(p);
	// Setup string with no null termination.
	memset(payload->mTo, 1, sizeof(payload->mTo));

	EXPECT_FALSE(mZoneConnection->hasStringError());
	EXPECT_FALSE(mZoneConnection->handleGroupInvite(p));
	EXPECT_TRUE(mZoneConnection->hasStringError());

	delete p;
}

/*
	handleGroupAcceptInvite
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleGroupAcceptInvite_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleGroupAcceptInvite(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGroupAcceptInvite_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Group::Follow::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleGroupAcceptInvite(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGroupAcceptInvite_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Group::Follow::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleGroupAcceptInvite(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGroupAcceptInvite_Overflow_0) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Group::Follow::size());
	auto payload = Payload::Group::Follow::convert(p);
	// Setup string with no null termination.
	memset(payload->mName1, 1, sizeof(payload->mName1));

	EXPECT_FALSE(mZoneConnection->hasStringError());
	EXPECT_FALSE(mZoneConnection->handleGroupAcceptInvite(p));
	EXPECT_TRUE(mZoneConnection->hasStringError());

	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGroupAcceptInvite_Overflow_1) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Group::Follow::size());
	auto payload = Payload::Group::Follow::convert(p);
	// Setup string with no null termination.
	memset(payload->mName2, 1, sizeof(payload->mName2));

	EXPECT_FALSE(mZoneConnection->hasStringError());
	EXPECT_FALSE(mZoneConnection->handleGroupAcceptInvite(p));
	EXPECT_TRUE(mZoneConnection->hasStringError());

	delete p;
}

/*
	handleGroupDeclineInvite
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleGroupDeclineInvite_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleGroupDeclineInvite(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGroupDeclineInvite_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Group::DeclineInvite::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleGroupDeclineInvite(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGroupDeclineInvite_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Group::DeclineInvite::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleGroupDeclineInvite(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGroupDeclineInvite_Overflow_0) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Group::DeclineInvite::size());
	auto payload = Payload::Group::DeclineInvite::convert(p);
	// Setup string with no null termination.
	memset(payload->mName1, 1, sizeof(payload->mName1));

	EXPECT_FALSE(mZoneConnection->hasStringError());
	EXPECT_FALSE(mZoneConnection->handleGroupDeclineInvite(p));
	EXPECT_TRUE(mZoneConnection->hasStringError());

	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGroupDeclineInvite_Overflow_1) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Group::DeclineInvite::size());
	auto payload = Payload::Group::DeclineInvite::convert(p);
	// Setup string with no null termination.
	memset(payload->mName2, 1, sizeof(payload->mName2));

	EXPECT_FALSE(mZoneConnection->hasStringError());
	EXPECT_FALSE(mZoneConnection->handleGroupDeclineInvite(p));
	EXPECT_TRUE(mZoneConnection->hasStringError());

	delete p;
}

/*
	handleGroupDisband
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleGroupDisband_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleGroupDisband(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGroupDisband_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Group::Disband::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleGroupDisband(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGroupDisband_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Group::Disband::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleGroupDisband(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGroupDisband_Overflow_0) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Group::Disband::size());
	auto payload = Payload::Group::Disband::convert(p);
	// Setup string with no null termination.
	memset(payload->name1, 1, sizeof(payload->name1));

	EXPECT_FALSE(mZoneConnection->hasStringError());
	EXPECT_FALSE(mZoneConnection->handleGroupDisband(p));
	EXPECT_TRUE(mZoneConnection->hasStringError());

	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGroupDisband_Overflow_1) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Group::Disband::size());
	auto payload = Payload::Group::Disband::convert(p);
	// Setup string with no null termination.
	memset(payload->name2, 1, sizeof(payload->name2));

	EXPECT_FALSE(mZoneConnection->hasStringError());
	EXPECT_FALSE(mZoneConnection->handleGroupDisband(p));
	EXPECT_TRUE(mZoneConnection->hasStringError());

	delete p;
}

/*
	handleGroupMakeLeader
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleGroupMakeLeader_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleGroupMakeLeader(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGroupMakeLeader_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Group::MakeLeader::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleGroupMakeLeader(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGroupMakeLeader_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Group::MakeLeader::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleGroupMakeLeader(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGroupMakeLeader_Overflow_0) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Group::MakeLeader::size());
	auto payload = Payload::Group::MakeLeader::convert(p);
	// Setup string with no null termination.
	memset(payload->mCurrentLeader, 1, sizeof(payload->mCurrentLeader));

	EXPECT_FALSE(mZoneConnection->hasStringError());
	EXPECT_FALSE(mZoneConnection->handleGroupMakeLeader(p));
	EXPECT_TRUE(mZoneConnection->hasStringError());

	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGroupMakeLeader_Overflow_1) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Group::MakeLeader::size());
	auto payload = Payload::Group::MakeLeader::convert(p);
	// Setup string with no null termination.
	memset(payload->mNewLeader, 1, sizeof(payload->mNewLeader));

	EXPECT_FALSE(mZoneConnection->hasStringError());
	EXPECT_FALSE(mZoneConnection->handleGroupMakeLeader(p));
	EXPECT_TRUE(mZoneConnection->hasStringError());

	delete p;
}

/*
	handleGuildCreate
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildCreate_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleGuildCreate(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildCreate_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Guild::Create::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleGuildCreate(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildCreate_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Guild::Create::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleGuildCreate(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildCreate_Overflow_0) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Guild::Create::size());
	auto payload = Payload::Guild::Create::convert(p);
	// Setup string with no null termination.
	memset(payload->mName, 1, sizeof(payload->mName));

	EXPECT_FALSE(mZoneConnection->hasStringError());
	EXPECT_FALSE(mZoneConnection->handleGuildCreate(p));
	EXPECT_TRUE(mZoneConnection->hasStringError());

	delete p;
}

/*
	handleGuildDelete
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildDelete_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleGuildDelete(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildDelete_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleGuildDelete(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleGuildInvite
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildInvite_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleGuildInvite(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildInvite_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Guild::Invite::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleGuildInvite(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildInvite_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Guild::Invite::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleGuildInvite(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildInvite_Overflow_0) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Guild::Invite::size());
	auto payload = Payload::Guild::Invite::convert(p);
	// Setup string with no null termination.
	memset(payload->mToCharacter, 1, sizeof(payload->mToCharacter));

	EXPECT_FALSE(mZoneConnection->hasStringError());
	EXPECT_FALSE(mZoneConnection->handleGuildInvite(p));
	EXPECT_TRUE(mZoneConnection->hasStringError());

	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildInvite_Overflow_1) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Guild::Invite::size());
	auto payload = Payload::Guild::Invite::convert(p);
	// Setup string with no null termination.
	memset(payload->mFromCharacter, 1, sizeof(payload->mFromCharacter));

	EXPECT_FALSE(mZoneConnection->hasStringError());
	EXPECT_FALSE(mZoneConnection->handleGuildInvite(p));
	EXPECT_TRUE(mZoneConnection->hasStringError());

	delete p;
}

/*
	handleGuildInviteResponse
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildInviteResponse_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleGuildInviteResponse(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildInviteResponse_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Guild::InviteResponse::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleGuildInviteResponse(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildInviteResponse_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Guild::InviteResponse::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleGuildInviteResponse(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildInviteResponse_Overflow_0) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Guild::InviteResponse::size());
	auto payload = Payload::Guild::InviteResponse::convert(p);
	// Setup string with no null termination.
	memset(payload->mInviter, 1, sizeof(payload->mInviter));

	EXPECT_FALSE(mZoneConnection->hasStringError());
	EXPECT_FALSE(mZoneConnection->handleGuildInviteResponse(p));
	EXPECT_TRUE(mZoneConnection->hasStringError());

	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildInviteResponse_Overflow_1) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Guild::InviteResponse::size());
	auto payload = Payload::Guild::InviteResponse::convert(p);
	// Setup string with no null termination.
	memset(payload->mNewMember, 1, sizeof(payload->mNewMember));

	EXPECT_FALSE(mZoneConnection->hasStringError());
	EXPECT_FALSE(mZoneConnection->handleGuildInviteResponse(p));
	EXPECT_TRUE(mZoneConnection->hasStringError());

	delete p;
}

/*
	handleGuildRemove
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildRemove_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleGuildRemove(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildRemove_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Guild::Remove::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleGuildRemove(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildRemove_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Guild::Remove::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleGuildRemove(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildRemove_Overflow_0) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Guild::Remove::size());
	auto payload = Payload::Guild::Remove::convert(p);
	// Setup string with no null termination.
	memset(payload->mToCharacter, 1, sizeof(payload->mToCharacter));

	EXPECT_FALSE(mZoneConnection->hasStringError());
	EXPECT_FALSE(mZoneConnection->handleGuildRemove(p));
	EXPECT_TRUE(mZoneConnection->hasStringError());

	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildRemove_Overflow_1) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Guild::Remove::size());
	auto payload = Payload::Guild::Remove::convert(p);
	// Setup string with no null termination.
	memset(payload->mFromCharacter, 1, sizeof(payload->mFromCharacter));

	EXPECT_FALSE(mZoneConnection->hasStringError());
	EXPECT_FALSE(mZoneConnection->handleGuildRemove(p));
	EXPECT_TRUE(mZoneConnection->hasStringError());

	delete p;
}

/*
	handleGuildSetMOTD
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildSetMOTD_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleGuildSetMOTD(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildSetMOTD_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Guild::MOTD::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleGuildSetMOTD(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildSetMOTD_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Guild::MOTD::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleGuildSetMOTD(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildSetMOTD_Overflow_0) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Guild::MOTD::size());
	auto payload = Payload::Guild::MOTD::convert(p);
	// Setup string with no null termination.
	memset(payload->mCharacterName, 1, sizeof(payload->mCharacterName));

	EXPECT_FALSE(mZoneConnection->hasStringError());
	EXPECT_FALSE(mZoneConnection->handleGuildSetMOTD(p));
	EXPECT_TRUE(mZoneConnection->hasStringError());

	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildSetMOTD_Overflow_1) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Guild::MOTD::size());
	auto payload = Payload::Guild::MOTD::convert(p);
	// Setup string with no null termination.
	memset(payload->mSetByName, 1, sizeof(payload->mSetByName));

	EXPECT_FALSE(mZoneConnection->hasStringError());
	EXPECT_FALSE(mZoneConnection->handleGuildSetMOTD(p));
	EXPECT_TRUE(mZoneConnection->hasStringError());

	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildSetMOTD_Overflow_2) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Guild::MOTD::size());
	auto payload = Payload::Guild::MOTD::convert(p);
	// Setup string with no null termination.
	memset(payload->mMOTD, 1, sizeof(payload->mMOTD));

	EXPECT_FALSE(mZoneConnection->hasStringError());
	EXPECT_FALSE(mZoneConnection->handleGuildSetMOTD(p));
	EXPECT_TRUE(mZoneConnection->hasStringError());

	delete p;
}

/*
	handleGuildGetMOTD
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildGetMOTD_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleGuildGetMOTD(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildGetMOTD_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleGuildGetMOTD(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleSetGuildURLOrChannel
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleSetGuildURLOrChannel_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleSetGuildURLOrChannel(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleSetGuildURLOrChannel_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Guild::GuildUpdate::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleSetGuildURLOrChannel(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleSetGuildURLOrChannel_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Guild::GuildUpdate::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleSetGuildURLOrChannel(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleSetGuildURLOrChannel_Overflow_0) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Guild::GuildUpdate::size());
	auto payload = Payload::Guild::GuildUpdate::convert(p);
	// Setup string with no null termination.
	memset(payload->mText, 1, sizeof(payload->mText));

	EXPECT_FALSE(mZoneConnection->hasStringError());
	EXPECT_FALSE(mZoneConnection->handleSetGuildURLOrChannel(p));
	EXPECT_TRUE(mZoneConnection->hasStringError());

	delete p;
}

/*
	handleSetGuildPublicNote
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleSetGuildPublicNote_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleSetGuildPublicNote(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleSetGuildPublicNote_Overflow_0) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Guild::PublicNote::size());
	auto payload = Payload::Guild::PublicNote::convert(p);
	// Setup string with no null termination.
	memset(payload->mSenderName, 1, sizeof(payload->mSenderName));

	EXPECT_FALSE(mZoneConnection->hasStringError());
	EXPECT_FALSE(mZoneConnection->handleSetGuildPublicNote(p));
	EXPECT_TRUE(mZoneConnection->hasStringError());

	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleSetGuildPublicNote_Overflow_1) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Guild::PublicNote::size());
	auto payload = Payload::Guild::PublicNote::convert(p);
	// Setup string with no null termination.
	memset(payload->mTargetName, 1, sizeof(payload->mTargetName));

	EXPECT_FALSE(mZoneConnection->hasStringError());
	EXPECT_FALSE(mZoneConnection->handleSetGuildPublicNote(p));
	EXPECT_TRUE(mZoneConnection->hasStringError());

	delete p;
}

/*
	handleGetGuildStatus
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleGetGuildStatus_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleGetGuildStatus(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGetGuildStatus_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Guild::StatusRequest::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleGetGuildStatus(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGetGuildStatus_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Guild::StatusRequest::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleGetGuildStatus(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGetGuildStatus_Overflow_0) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Guild::StatusRequest::size());
	auto payload = Payload::Guild::StatusRequest::convert(p);
	// Setup string with no null termination.
	memset(payload->mName, 1, sizeof(payload->mName));

	EXPECT_FALSE(mZoneConnection->hasStringError());
	EXPECT_FALSE(mZoneConnection->handleGetGuildStatus(p));
	EXPECT_TRUE(mZoneConnection->hasStringError());

	delete p;
}

/*
	handleGuildDemote
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildDemote_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleGuildDemote(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildDemote_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Guild::Demote::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleGuildDemote(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildDemote_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Guild::Demote::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleGuildDemote(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildDemote_Overflow_0) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Guild::Demote::size());
	auto payload = Payload::Guild::Demote::convert(p);
	// Setup string with no null termination.
	memset(payload->mCharacterName, 1, sizeof(payload->mCharacterName));

	EXPECT_FALSE(mZoneConnection->hasStringError());
	EXPECT_FALSE(mZoneConnection->handleGuildDemote(p));
	EXPECT_TRUE(mZoneConnection->hasStringError());

	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildDemote_Overflow_1) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Guild::Demote::size());
	auto payload = Payload::Guild::Demote::convert(p);
	// Setup string with no null termination.
	memset(payload->mDemoteName, 1, sizeof(payload->mDemoteName));

	EXPECT_FALSE(mZoneConnection->hasStringError());
	EXPECT_FALSE(mZoneConnection->handleGuildDemote(p));
	EXPECT_TRUE(mZoneConnection->hasStringError());

	delete p;
}

/*
	handleGuildSetFlags
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildSetFlags_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleGuildSetFlags(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildSetFlags_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Guild::FlagsUpdate::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleGuildSetFlags(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildSetFlags_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Guild::FlagsUpdate::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleGuildSetFlags(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildSetFlags_Overflow_0) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Guild::FlagsUpdate::size());
	auto payload = Payload::Guild::FlagsUpdate::convert(p);
	// Setup string with no null termination.
	memset(payload->mCharacterName, 1, sizeof(payload->mCharacterName));

	EXPECT_FALSE(mZoneConnection->hasStringError());
	EXPECT_FALSE(mZoneConnection->handleGuildSetFlags(p));
	EXPECT_TRUE(mZoneConnection->hasStringError());

	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildSetFlags_Overflow_1) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Guild::FlagsUpdate::size());
	auto payload = Payload::Guild::FlagsUpdate::convert(p);
	// Setup string with no null termination.
	memset(payload->mOtherName, 1, sizeof(payload->mOtherName));

	EXPECT_FALSE(mZoneConnection->hasStringError());
	EXPECT_FALSE(mZoneConnection->handleGuildSetFlags(p));
	EXPECT_TRUE(mZoneConnection->hasStringError());

	delete p;
}

/*
	handleGuildMakeLeader
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildMakeLeader_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleGuildMakeLeader(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildMakeLeader_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Guild::MakeLeader::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleGuildMakeLeader(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildMakeLeader_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Guild::MakeLeader::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleGuildMakeLeader(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildMakeLeader_Overflow_0) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Guild::MakeLeader::size());
	auto payload = Payload::Guild::MakeLeader::convert(p);
	// Setup string with no null termination.
	memset(payload->mCharacterName, 1, sizeof(payload->mCharacterName));

	EXPECT_FALSE(mZoneConnection->hasStringError());
	EXPECT_FALSE(mZoneConnection->handleGuildMakeLeader(p));
	EXPECT_TRUE(mZoneConnection->hasStringError());

	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildMakeLeader_Overflow_1) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Guild::MakeLeader::size());
	auto payload = Payload::Guild::MakeLeader::convert(p);
	// Setup string with no null termination.
	memset(payload->mLeaderName, 1, sizeof(payload->mLeaderName));

	EXPECT_FALSE(mZoneConnection->hasStringError());
	EXPECT_FALSE(mZoneConnection->handleGuildMakeLeader(p));
	EXPECT_TRUE(mZoneConnection->hasStringError());

	delete p;
}

/*
	handleZoneChange
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleZoneChange_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleZoneChange(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleZoneChange_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::ZoneChange::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleZoneChange(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleZoneChange_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::ZoneChange::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleZoneChange(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleZoneChange_Overflow_0) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::ZoneChange::size());
	auto payload = Payload::Zone::ZoneChange::convert(p);
	// Setup string with no null termination.
	memset(payload->mCharacterName, 1, sizeof(payload->mCharacterName));

	EXPECT_FALSE(mZoneConnection->hasStringError());
	EXPECT_FALSE(mZoneConnection->handleZoneChange(p));
	EXPECT_TRUE(mZoneConnection->hasStringError());

	delete p;
}

/*
	handleFaceChange
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleFaceChange_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleFaceChange(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleFaceChange_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::FaceChange::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleFaceChange(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleFaceChange_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::FaceChange::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleFaceChange(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleAutoAttack
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleAutoAttack_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleAutoAttack(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleAutoAttack_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::AutoAttack::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleAutoAttack(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleAutoAttack_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::AutoAttack::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleAutoAttack(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleMemoriseSpell
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleMemoriseSpell_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleMemoriseSpell(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleMemoriseSpell_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::MemoriseSpell::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleMemoriseSpell(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleMemoriseSpell_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::MemoriseSpell::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleMemoriseSpell(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleDeleteSpell
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleDeleteSpell_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleDeleteSpell(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleDeleteSpell_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::DeleteSpell::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleDeleteSpell(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleDeleteSpell_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::DeleteSpell::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleDeleteSpell(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleLoadSpellSet
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleLoadSpellSet_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleLoadSpellSet(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleLoadSpellSet_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::LoadSpellSet::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleLoadSpellSet(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleLoadSpellSet_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::LoadSpellSet::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleLoadSpellSet(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleSwapSpell
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleSwapSpell_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleSwapSpell(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleSwapSpell_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::SwapSpell::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleSwapSpell(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleSwapSpell_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::SwapSpell::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleSwapSpell(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleCastSpell
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleCastSpell_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleCastSpell(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleCastSpell_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::CastSpell::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleCastSpell(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleCastSpell_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::CastSpell::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleCastSpell(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleTGB
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleTGB_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleTGB(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleTGB_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::TGB::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleTGB(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleTGB_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::TGB::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleTGB(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleEmote
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleEmote_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleEmote(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleEmote_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::Emote::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleEmote(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleEmote_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::Emote::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleEmote(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleEmote_Overflow_0) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::Emote::size());
	auto payload = Payload::Zone::Emote::convert(p);
	// Setup string with no null termination.
	memset(payload->mMessage, 1, sizeof(payload->mMessage));

	EXPECT_FALSE(mZoneConnection->hasStringError());
	EXPECT_FALSE(mZoneConnection->handleEmote(p));
	EXPECT_TRUE(mZoneConnection->hasStringError());

	delete p;
}

/*
	handleAnimation
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleAnimation_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleAnimation(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleAnimation_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::ActorAnimation::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleAnimation(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleAnimation_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::ActorAnimation::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleAnimation(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleWhoRequest
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleWhoRequest_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleWhoRequest(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleWhoRequest_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::WhoRequest::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleWhoRequest(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleWhoRequest_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::WhoRequest::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleWhoRequest(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleWhoRequest_Overflow_0) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::WhoRequest::size());
	auto payload = Payload::Zone::WhoRequest::convert(p);
	// Setup string with no null termination.
	memset(payload->mCharacterName, 1, sizeof(payload->mCharacterName));

	EXPECT_FALSE(mZoneConnection->hasStringError());
	EXPECT_FALSE(mZoneConnection->handleWhoRequest(p));
	EXPECT_TRUE(mZoneConnection->hasStringError());

	delete p;
}

/*
	handleCombatAbility
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleCombatAbility_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleCombatAbility(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleCombatAbility_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::CombatAbility::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleCombatAbility(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleCombatAbility_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::CombatAbility::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleCombatAbility(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleTaunt
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleTaunt_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleTaunt(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleTaunt_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::Taunt::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleTaunt(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleTaunt_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::Taunt::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleTaunt(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleConsider
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleConsider_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleConsider(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleConsider_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::Consider::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleConsider(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleConsider_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::Consider::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleConsider(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleConsiderCorpse
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleConsiderCorpse_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleConsiderCorpse(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleConsiderCorpse_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::Consider::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleConsiderCorpse(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleConsiderCorpse_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::Consider::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleConsiderCorpse(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleBeginLootRequest
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleBeginLootRequest_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleBeginLootRequest(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleBeginLootRequest_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::LootBeginRequest::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleBeginLootRequest(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleBeginLootRequest_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::LootBeginRequest::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleBeginLootRequest(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleEndLootRequest
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleEndLootRequest_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleEndLootRequest(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleEndLootRequest_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleEndLootRequest(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleLootItem
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleLootItem_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleLootItem(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleLootItem_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::LootItem::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleLootItem(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleLootItem_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::LootItem::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleLootItem(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleMoveItemImpl
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleMoveItemImpl_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleMoveItemImpl(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleMoveItemImpl_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::MoveItem::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleMoveItemImpl(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleMoveItemImpl_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::MoveItem::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleMoveItemImpl(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleConsume
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleConsume_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleConsume(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleConsume_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::Consume::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleConsume(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleConsume_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::Consume::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleConsume(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleItemRightClick
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleItemRightClick_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleItemRightClick(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleItemRightClick_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::ItemRightClick::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleItemRightClick(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleItemRightClick_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::ItemRightClick::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleItemRightClick(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleOpenContainer
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleOpenContainer_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleOpenContainer(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleOpenContainer_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::OpenContainer::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleOpenContainer(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleOpenContainer_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::OpenContainer::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleOpenContainer(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleShopRequest
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleShopRequest_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleShopRequest(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleShopRequest_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::ShopRequest::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleShopRequest(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleShopRequest_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::ShopRequest::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleShopRequest(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleShopEnd
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleShopEnd_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleShopEnd(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleShopEnd_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::ShopEnd::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleShopEnd(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleShopEnd_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::ShopEnd::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleShopEnd(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleShopSell
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleShopSell_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleShopSell(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleShopSell_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::ShopSell::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleShopSell(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleShopSell_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::ShopSell::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleShopSell(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleShopBuy
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleShopBuy_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleShopBuy(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleShopBuy_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::ShopBuy::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleShopBuy(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleShopBuy_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::ShopBuy::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleShopBuy(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleTradeRequest
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleTradeRequest_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleTradeRequest(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleTradeRequest_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::TradeRequest::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleTradeRequest(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleTradeRequest_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::TradeRequest::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleTradeRequest(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleTradeRequestAck
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleTradeRequestAck_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleTradeRequestAck(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleTradeRequestAck_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::TradeRequest::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleTradeRequestAck(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleTradeRequestAck_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::TradeRequest::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleTradeRequestAck(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleCancelTrade
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleCancelTrade_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleCancelTrade(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleCancelTrade_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::TradeCancel::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleCancelTrade(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleCancelTrade_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::TradeCancel::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleCancelTrade(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleAcceptTrade
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleAcceptTrade_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleAcceptTrade(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleAcceptTrade_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::TradeAccept::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleAcceptTrade(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleAcceptTrade_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::TradeAccept::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleAcceptTrade(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleTradeBusy
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleTradeBusy_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleTradeBusy(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleTradeBusy_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::TradeBusy::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleTradeBusy(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleTradeBusy_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::TradeBusy::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleTradeBusy(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleSetServerFiler
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleSetServerFiler_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleSetServerFiler(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleSetServerFiler_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::ServerFilter::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleSetServerFiler(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleSetServerFiler_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::ServerFilter::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleSetServerFiler(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleItemLinkClick
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleItemLinkClick_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleItemLinkClick(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleItemLinkClick_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::ItemLink::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleItemLinkClick(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleItemLinkClick_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::ItemLink::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleItemLinkClick(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleMoveCoinImpl
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleMoveCoinImpl_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleMoveCoinImpl(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleMoveCoinImpl_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::MoveCoin::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleMoveCoinImpl(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleMoveCoinImpl_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::MoveCoin::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleMoveCoinImpl(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleCrystalCreate
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleCrystalCreate_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleCrystalCreate(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleCrystalCreate_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::CrystalCreate::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleCrystalCreate(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleCrystalCreate_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::CrystalCreate::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleCrystalCreate(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleCrystalReclaim
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleCrystalReclaim_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleCrystalReclaim(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleCrystalReclaim_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleCrystalReclaim(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleAugmentItem
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleAugmentItem_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleAugmentItem(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleAugmentItem_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::AugmentItem::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleAugmentItem(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleAugmentItem_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::AugmentItem::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleAugmentItem(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleAugmentInfo
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleAugmentInfo_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleAugmentInfo(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleAugmentInfo_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::AugmentInformation::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleAugmentInfo(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleAugmentInfo_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::AugmentInformation::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleAugmentInfo(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleReadBook
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleReadBook_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleReadBook(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleReadBook_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::BookRequest::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleReadBook(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleReadBook_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::BookRequest::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleReadBook(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleReadBook_Overflow_0) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::BookRequest::size());
	auto payload = Payload::Zone::BookRequest::convert(p);
	// Setup string with no null termination.
	memset(payload->mText, 1, sizeof(payload->mText));

	EXPECT_FALSE(mZoneConnection->hasStringError());
	EXPECT_FALSE(mZoneConnection->handleReadBook(p));
	EXPECT_TRUE(mZoneConnection->hasStringError());

	delete p;
}

/*
	handleCombine
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleCombine_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleCombine(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleCombine_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::Combine::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleCombine(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleCombine_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::Combine::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleCombine(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleEnvironmentDamage
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleEnvironmentDamage_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleEnvironmentDamage(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleEnvironmentDamage_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::EnvironmentDamage::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleEnvironmentDamage(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleEnvironmentDamage_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::EnvironmentDamage::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleEnvironmentDamage(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handlePopupResponse
*/

TEST_F(ZoneConnectionHandlerSanityTest, handlePopupResponse_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handlePopupResponse(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handlePopupResponse_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::PopupResponse::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handlePopupResponse(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handlePopupResponse_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::PopupResponse::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handlePopupResponse(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleClaimRequest
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleClaimRequest_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleClaimRequest(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleClaimRequest_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::ClaimRequest::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleClaimRequest(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleClaimRequest_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::ClaimRequest::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleClaimRequest(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleClaimRequest_Overflow_0) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::ClaimRequest::size());
	auto payload = Payload::Zone::ClaimRequest::convert(p);
	// Setup string with no null termination.
	memset(payload->mName, 1, sizeof(payload->mName));

	EXPECT_FALSE(mZoneConnection->hasStringError());
	EXPECT_FALSE(mZoneConnection->handleClaimRequest(p));
	EXPECT_TRUE(mZoneConnection->hasStringError());

	delete p;
}

/*
	handleCamp
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleCamp_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleCamp(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleCamp_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::Camp::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleCamp(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleCamp_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::Camp::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleCamp(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handlePotionBelt
*/

TEST_F(ZoneConnectionHandlerSanityTest, handlePotionBelt_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handlePotionBelt(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handlePotionBelt_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::PotionBelt::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handlePotionBelt(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handlePotionBelt_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::PotionBelt::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handlePotionBelt(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleAlternateCurrencyReclaim
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleAlternateCurrencyReclaim_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleAlternateCurrencyReclaim(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleAlternateCurrencyReclaim_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::AlternateCurrencyReclaim::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleAlternateCurrencyReclaim(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleAlternateCurrencyReclaim_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::AlternateCurrencyReclaim::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleAlternateCurrencyReclaim(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleRandomRequest
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleRandomRequest_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleRandomRequest(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleRandomRequest_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::RandomRequest::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleRandomRequest(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleRandomRequest_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::RandomRequest::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleRandomRequest(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleDropItem
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleDropItem_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleDropItem(nullptr));
}

/*
	handleXTargetAutoAddHaters
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleXTargetAutoAddHaters_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleXTargetAutoAddHaters(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleXTargetAutoAddHaters_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::ExtendedTarget::AutoAddHaters::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleXTargetAutoAddHaters(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleXTargetAutoAddHaters_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::ExtendedTarget::AutoAddHaters::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleXTargetAutoAddHaters(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleRespawnWindowSelect
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleRespawnWindowSelect_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleRespawnWindowSelect(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleRespawnWindowSelect_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::RespawnWindowSelect::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleRespawnWindowSelect(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleRespawnWindowSelect_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::RespawnWindowSelect::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleRespawnWindowSelect(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleAAAction
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleAAAction_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleAAAction(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleAAAction_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::AAAction::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleAAAction(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleAAAction_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::AAAction::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleAAAction(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleLeadershipExperienceToggle
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleLeadershipExperienceToggle_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleLeadershipExperienceToggle(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleLeadershipExperienceToggle_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::LeadershipExperienceToggle::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleLeadershipExperienceToggle(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleLeadershipExperienceToggle_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::LeadershipExperienceToggle::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleLeadershipExperienceToggle(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleSurname
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleSurname_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleSurname(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleSurname_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::Surname::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleSurname(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleSurname_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::Surname::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleSurname(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleSurname_Overflow_0) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::Surname::size());
	auto payload = Payload::Zone::Surname::convert(p);
	// Setup string with no null termination.
	memset(payload->mCharacterName, 1, sizeof(payload->mCharacterName));

	EXPECT_FALSE(mZoneConnection->hasStringError());
	EXPECT_FALSE(mZoneConnection->handleSurname(p));
	EXPECT_TRUE(mZoneConnection->hasStringError());

	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleSurname_Overflow_1) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::Surname::size());
	auto payload = Payload::Zone::Surname::convert(p);
	// Setup string with no null termination.
	memset(payload->mLastName, 1, sizeof(payload->mLastName));

	EXPECT_FALSE(mZoneConnection->hasStringError());
	EXPECT_FALSE(mZoneConnection->handleSurname(p));
	EXPECT_TRUE(mZoneConnection->hasStringError());

	delete p;
}

/*
	handleClearSurname
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleClearSurname_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleClearSurname(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleClearSurname_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleClearSurname(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleLogOut
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleLogOut_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleLogOut(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleLogOut_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleLogOut(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleDeleteSpawn
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleDeleteSpawn_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleDeleteSpawn(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleDeleteSpawn_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::DeleteSpawn::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleDeleteSpawn(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleDeleteSpawn_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::DeleteSpawn::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleDeleteSpawn(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleSetTitle
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleSetTitle_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleSetTitle(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleSetTitle_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::SetTitle::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleSetTitle(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleSetTitle_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::SetTitle::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleSetTitle(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleRequestTitles
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleRequestTitles_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleRequestTitles(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleRequestTitles_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleRequestTitles(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handlePetCommand
*/

TEST_F(ZoneConnectionHandlerSanityTest, handlePetCommand_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handlePetCommand(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handlePetCommand_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::PetCommand::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handlePetCommand(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handlePetCommand_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::PetCommand::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handlePetCommand(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

/*
	handleSaveRequest
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleSaveRequest_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleSaveRequest(nullptr));
}

TEST_F(ZoneConnectionHandlerSanityTest, handleSaveRequest_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::SaveRequest::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleSaveRequest(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}

TEST_F(ZoneConnectionHandlerSanityTest, handleSaveRequest_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::Zone::SaveRequest::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mZoneConnection->hasSizeError());
	EXPECT_FALSE(mZoneConnection->handleSaveRequest(p));
	EXPECT_TRUE(mZoneConnection->hasSizeError());
	delete p;
}