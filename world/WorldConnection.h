#pragma once

#include "Types.h"

class EQApplicationPacket;
class EQStreamInterface;
class World;
class Account;
class ILog;

class WorldConnection {
public:
	~WorldConnection();

	const bool initialise(World* pWorld, ILog* pLog, EQStreamInterface* pStreamInterface);
	bool update();

	inline Account* getAccount() const { return mAccount; }
	inline void setAccount(Account* pAccount) { mAccount = pAccount; }
	inline const bool hasAccount() const { return mAccount != nullptr; }

	const u32 getIP() const;

	void sendPacket(const EQApplicationPacket* pPacket);

	void sendCharacterSelectInfo();
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

private:

	bool mInitialised = false;
	ILog* mLog = nullptr;
	Account* mAccount = nullptr;
	EQStreamInterface* mStreamInterface = nullptr;
	World* mWorld = nullptr;

	void updateLogContext();

	bool _handlePacket(const EQApplicationPacket* pPacket);
	bool _handleGenerateRandomName(const EQApplicationPacket* pPacket);
	bool _handleCharacterCreate(const EQApplicationPacket* pPacket);
	bool _handleEnterWorld(const EQApplicationPacket* pPacket);
	bool _handleDeleteCharacter(const EQApplicationPacket* pPacket);
	bool _handleConnect(const EQApplicationPacket* packet);
	bool _handleCharacterCreateRequest(const EQApplicationPacket* packet);
	bool _handleApproveName(const EQApplicationPacket* packet);

	inline void dropConnection() { mConnectionDropped = true; }

	bool mConnectionDropped = false;
	bool mZoning = false;
};