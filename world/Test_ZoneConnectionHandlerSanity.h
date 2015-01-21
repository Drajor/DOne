#pragma once
#include "gtest/gtest.h"

#include "Test_Utility.h"

#include "ZoneConnection.h"
#include "GuildManager.h"
#include "Zone.h"
#include "../common/EQStreamIntf.h"

class NullEQStreamInterface : public EQStreamInterface {
	void QueuePacket(const EQApplicationPacket *p, bool ack_req = true) { };
	void FastQueuePacket(EQApplicationPacket **p, bool ack_req = true) { };
	EQApplicationPacket *PopPacket() { return nullptr; };
	void Close() {};
	void ReleaseFromUse() {};
	void RemoveData() {};
	uint32 GetRemoteIP() const { return 0; }
	uint16 GetRemotePort() const { return 0; }
	bool CheckState(EQStreamState state) { return EQStreamState::ESTABLISHED; }
	std::string Describe() const { return ""; }
};

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

/*
		handleRequestClientSpawn
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleRequestClientSpawn_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleRequestClientSpawn(nullptr));
}

/*
	handleClientReady
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleClientReady_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleClientReady(nullptr));
}

/*
	handleRequestNewZoneData
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleRequestNewZoneData_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleRequestNewZoneData(nullptr));
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

/*
	handleGuildDelete
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildDelete_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleGuildDelete(nullptr));
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

/*
	handleGuildGetMOTD
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleGuildGetMOTD_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleGuildGetMOTD(nullptr));
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

/*
	handleSetGuildPublicNote
*/

TEST_F(ZoneConnectionHandlerSanityTest, handleSetGuildPublicNote_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mZoneConnection->handleSetGuildPublicNote(nullptr));
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