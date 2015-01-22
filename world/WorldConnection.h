#pragma once

#include "Types.h"

class EQApplicationPacket;
class EQStreamInterface;
class World;
class Account;
class ILog;
class ILogFactory;

class WorldConnection {
public:
	~WorldConnection();

	const bool initialise(World* pWorld, ILogFactory* pLogFactory, EQStreamInterface* pStreamInterface);
	bool update();

	inline Account* getAccount() const { return mAccount; }
	inline void setAccount(Account* pAccount) { mAccount = pAccount; }
	inline const bool hasAccount() const { return mAccount != nullptr; }
	inline const bool isZoning() const { return mZoning; }

	const u32 getIP() const;

	void sendPacket(const EQApplicationPacket* pPacket);

	void sendCharacterSelection();
	void sendGuildList();
	void sendEnterWorld(const String& pCharacterName);
	void sendExpansionInfo();
	void sendLogServer();
	void sendApproveWorld();
	void sendPostEnterWorld();
	void sendZoneUnavailable();
	void sendZoneServerInfo(const String& pIP, const u16 pPort);
	void sendChatServer(const String& pCharacterName);
	void sendApproveNameResponse(const bool pResponse);

	const bool handlePacket(const EQApplicationPacket* pPacket);

	const bool handleGenerateRandomName(const EQApplicationPacket* pPacket);
	const bool handleCharacterCreate(const EQApplicationPacket* pPacket);
	const bool handleEnterWorld(const EQApplicationPacket* pPacket);
	const bool handleDeleteCharacter(const EQApplicationPacket* pPacket);
	const bool handleConnect(const EQApplicationPacket* packet);
	const bool handleCharacterCreateRequest(const EQApplicationPacket* packet);
	const bool handleApproveName(const EQApplicationPacket* packet);

	inline const bool hasSizeError() const { return mSizeError; } // For unit testing.
	inline const bool hasStringError() const { return mStringError; } // For unit testing.

private:

	bool mInitialised = false;
	ILog* mLog = nullptr;
	Account* mAccount = nullptr;
	EQStreamInterface* mStreamInterface = nullptr;
	World* mWorld = nullptr;

	void updateLogContext();

	inline void dropConnection() { mConnectionDropped = true; }

	bool mConnectionDropped = false;
	bool mZoning = false;

	bool mSizeError = false; // For unit testing.
	bool mStringError = false; // For unit testing.
};