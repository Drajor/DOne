#pragma once
#include "gtest/gtest.h"

#include "Test_Utility.h"

#include "World.h"
#include "WorldConnection.h"

class WorldConnectionHandlerSanityTest : public ::testing::Test {
protected:
	virtual void SetUp() {
		mLogFactory = new NullLogFactory();
		mWorldConnection = new WorldConnection();
		mZone = new World();
		mStreamInterface = new NullEQStreamInterface();
	}

	virtual void TearDown() {
		delete mLogFactory;
		mLogFactory = nullptr;

		delete mZone;
		mZone = nullptr;

		delete mWorldConnection;
		mWorldConnection = nullptr;

		delete mStreamInterface;
		mStreamInterface = nullptr;
	}

	bool initialise() {
		return mWorldConnection->initialise(mZone, mLogFactory, mStreamInterface);
	}

	EQApplicationPacket* makePacket(const u32 pSize) {
		return new EQApplicationPacket(EmuOpcode::OP_Unknown, nullptr, pSize);
	}

	NullEQStreamInterface* mStreamInterface = nullptr;
	NullLogFactory* mLogFactory = nullptr;
	World* mZone = nullptr;
	WorldConnection* mWorldConnection = nullptr;
};

/*
	handleGenerateRandomName
*/

TEST_F(WorldConnectionHandlerSanityTest, handleGenerateRandomName_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mWorldConnection->handleGenerateRandomName(nullptr));
}

TEST_F(WorldConnectionHandlerSanityTest, handleGenerateRandomName_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::World::NameGeneration::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mWorldConnection->hasSizeError());
	EXPECT_FALSE(mWorldConnection->handleGenerateRandomName(p));
	EXPECT_TRUE(mWorldConnection->hasSizeError());
	delete p;
}

TEST_F(WorldConnectionHandlerSanityTest, handleGenerateRandomName_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::World::NameGeneration::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mWorldConnection->hasSizeError());
	EXPECT_FALSE(mWorldConnection->handleGenerateRandomName(p));
	EXPECT_TRUE(mWorldConnection->hasSizeError());
	delete p;
}

TEST_F(WorldConnectionHandlerSanityTest, handleGenerateRandomName_Overflow_0) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::World::NameGeneration::size());
	auto payload = Payload::World::NameGeneration::convert(p);
	// Setup string with no null termination.
	memset(payload->mName, 1, sizeof(payload->mName));

	EXPECT_FALSE(mWorldConnection->hasStringError());
	EXPECT_FALSE(mWorldConnection->handleGenerateRandomName(p));
	EXPECT_TRUE(mWorldConnection->hasStringError());

	delete p;
}

/*
	handleCharacterCreate
*/

TEST_F(WorldConnectionHandlerSanityTest, handleCharacterCreate_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mWorldConnection->handleCharacterCreate(nullptr));
}

TEST_F(WorldConnectionHandlerSanityTest, handleCharacterCreate_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::World::CreateCharacter::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mWorldConnection->hasSizeError());
	EXPECT_FALSE(mWorldConnection->handleCharacterCreate(p));
	EXPECT_TRUE(mWorldConnection->hasSizeError());
	delete p;
}

TEST_F(WorldConnectionHandlerSanityTest, handleCharacterCreate_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::World::CreateCharacter::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mWorldConnection->hasSizeError());
	EXPECT_FALSE(mWorldConnection->handleCharacterCreate(p));
	EXPECT_TRUE(mWorldConnection->hasSizeError());
	delete p;
}

/*
	handleEnterWorld
*/

TEST_F(WorldConnectionHandlerSanityTest, handleEnterWorld_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mWorldConnection->handleEnterWorld(nullptr));
}

TEST_F(WorldConnectionHandlerSanityTest, handleEnterWorld_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::World::EnterWorld::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mWorldConnection->hasSizeError());
	EXPECT_FALSE(mWorldConnection->handleEnterWorld(p));
	EXPECT_TRUE(mWorldConnection->hasSizeError());
	delete p;
}

TEST_F(WorldConnectionHandlerSanityTest, handleEnterWorld_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::World::EnterWorld::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mWorldConnection->hasSizeError());
	EXPECT_FALSE(mWorldConnection->handleEnterWorld(p));
	EXPECT_TRUE(mWorldConnection->hasSizeError());
	delete p;
}

TEST_F(WorldConnectionHandlerSanityTest, handleEnterWorld_Overflow_0) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::World::EnterWorld::size());
	auto payload = Payload::World::EnterWorld::convert(p);
	// Setup string with no null termination.
	memset(payload->mCharacterName, 1, sizeof(payload->mCharacterName));

	EXPECT_FALSE(mWorldConnection->hasStringError());
	EXPECT_FALSE(mWorldConnection->handleEnterWorld(p));
	EXPECT_TRUE(mWorldConnection->hasStringError());

	delete p;
}

/*
	handleDeleteCharacter
*/

TEST_F(WorldConnectionHandlerSanityTest, handleDeleteCharacter_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mWorldConnection->handleDeleteCharacter(nullptr));
}

TEST_F(WorldConnectionHandlerSanityTest, handleDeleteCharacter_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Limits::Character::MAX_INPUT_LENGTH + 1);
	// Fail: Too big.
	EXPECT_FALSE(mWorldConnection->hasSizeError());
	EXPECT_FALSE(mWorldConnection->handleDeleteCharacter(p));
	EXPECT_TRUE(mWorldConnection->hasSizeError());
	delete p;
}

TEST_F(WorldConnectionHandlerSanityTest, handleDeleteCharacter_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Limits::Character::MIN_INPUT_LENGTH - 1);
	// Fail: Too small.
	EXPECT_FALSE(mWorldConnection->hasSizeError());
	EXPECT_FALSE(mWorldConnection->handleDeleteCharacter(p));
	EXPECT_TRUE(mWorldConnection->hasSizeError());
	delete p;
}

TEST_F(WorldConnectionHandlerSanityTest, handleDeleteCharacter_Overflow_0) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Limits::Character::MIN_INPUT_LENGTH + 1);
	// Setup string with no null termination.
	memset(p->pBuffer, 1, p->size);

	EXPECT_FALSE(mWorldConnection->hasStringError());
	EXPECT_FALSE(mWorldConnection->handleDeleteCharacter(p));
	EXPECT_TRUE(mWorldConnection->hasStringError());

	delete p;
}

/*
	handleConnect
*/

TEST_F(WorldConnectionHandlerSanityTest, handleConnect_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mWorldConnection->handleConnect(nullptr));
}

TEST_F(WorldConnectionHandlerSanityTest, handleConnect_IDNaN) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::World::Connect::size());
	auto payload = Payload::World::Connect::convert(p);
	// ID string is not convertible to a number
	memset(payload->mInformation, 0, sizeof(payload->mInformation));
	strcpy(payload->mInformation, "nan\0randomcharacters");

	EXPECT_FALSE(mWorldConnection->hasStringError());
	EXPECT_FALSE(mWorldConnection->hasSizeError());
	EXPECT_FALSE(mWorldConnection->handleConnect(p));
	EXPECT_FALSE(mWorldConnection->hasStringError());
	EXPECT_FALSE(mWorldConnection->hasSizeError());
}

TEST_F(WorldConnectionHandlerSanityTest, handleConnect_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::World::Connect::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mWorldConnection->hasSizeError());
	EXPECT_FALSE(mWorldConnection->handleConnect(p));
	EXPECT_TRUE(mWorldConnection->hasSizeError());
	delete p;
}

TEST_F(WorldConnectionHandlerSanityTest, handleConnect_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::World::Connect::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mWorldConnection->hasSizeError());
	EXPECT_FALSE(mWorldConnection->handleConnect(p));
	EXPECT_TRUE(mWorldConnection->hasSizeError());
	delete p;
}

TEST_F(WorldConnectionHandlerSanityTest, handleConnect_Overflow_0) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::World::Connect::size());
	auto payload = Payload::World::Connect::convert(p);
	// Setup string with no null termination.
	memset(payload->mInformation, 1, sizeof(payload->mInformation));

	EXPECT_FALSE(mWorldConnection->hasStringError());
	EXPECT_FALSE(mWorldConnection->handleConnect(p));
	EXPECT_TRUE(mWorldConnection->hasStringError());

	delete p;
}

TEST_F(WorldConnectionHandlerSanityTest, handleConnect_Overflow_1) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::World::Connect::size());
	auto payload = Payload::World::Connect::convert(p);
	// No null terminator before 19 is not allowed.
	memset(payload->mInformation, 1, 19);

	EXPECT_FALSE(mWorldConnection->hasStringError());
	EXPECT_FALSE(mWorldConnection->handleConnect(p));
	EXPECT_TRUE(mWorldConnection->hasStringError());

	delete p;
}

TEST_F(WorldConnectionHandlerSanityTest, handleConnect_Overflow_2) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::World::Connect::size());
	auto payload = Payload::World::Connect::convert(p);
	memset(payload->mInformation, 0, sizeof(payload->mInformation));
	memset(payload->mInformation, 1, 18); // Valid first 18 bytes.
	memset(payload->mInformation + 19, 1, sizeof(payload->mInformation) - 19); // no null terminator in the rest

	EXPECT_FALSE(mWorldConnection->hasStringError());
	EXPECT_FALSE(mWorldConnection->handleConnect(p));
	EXPECT_TRUE(mWorldConnection->hasStringError());

	delete p;
}

/*
	handleCharacterCreateRequest
*/

TEST_F(WorldConnectionHandlerSanityTest, handleCharacterCreateRequest_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mWorldConnection->handleCharacterCreateRequest(nullptr));
}

TEST_F(WorldConnectionHandlerSanityTest, handleCharacterCreateRequest_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(1);
	// Fail: Too big.
	EXPECT_FALSE(mWorldConnection->hasSizeError());
	EXPECT_FALSE(mWorldConnection->handleCharacterCreateRequest(p));
	EXPECT_TRUE(mWorldConnection->hasSizeError());
	delete p;
}

/*
	handleApproveName
*/

TEST_F(WorldConnectionHandlerSanityTest, handleApproveName_Null) {
	EXPECT_TRUE(initialise());

	// Fail: Null.
	EXPECT_FALSE(mWorldConnection->handleApproveName(nullptr));
}

TEST_F(WorldConnectionHandlerSanityTest, handleApproveName_Big) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::World::ApproveName::size() + 1);
	// Fail: Too big.
	EXPECT_FALSE(mWorldConnection->hasSizeError());
	EXPECT_FALSE(mWorldConnection->handleApproveName(p));
	EXPECT_TRUE(mWorldConnection->hasSizeError());
	delete p;
}

TEST_F(WorldConnectionHandlerSanityTest, handleApproveName_Small) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::World::ApproveName::size() - 1);
	// Fail: Too small.
	EXPECT_FALSE(mWorldConnection->hasSizeError());
	EXPECT_FALSE(mWorldConnection->handleApproveName(p));
	EXPECT_TRUE(mWorldConnection->hasSizeError());
	delete p;
}

TEST_F(WorldConnectionHandlerSanityTest, handleApproveName_Overflow_0) {
	EXPECT_TRUE(initialise());

	auto p = makePacket(Payload::World::ApproveName::size());
	auto payload = Payload::World::ApproveName::convert(p);
	// Setup string with no null termination.
	memset(payload->mName, 1, sizeof(payload->mName));

	EXPECT_FALSE(mWorldConnection->hasStringError());
	EXPECT_FALSE(mWorldConnection->handleApproveName(p));
	EXPECT_TRUE(mWorldConnection->hasStringError());

	delete p;
}