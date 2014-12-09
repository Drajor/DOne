#pragma once

#include "Constants.h"
#include "ClientAuthentication.h"

class EQApplicationPacket;
class EQStreamInterface;
class World;

class WorldClientConnection {
public:
	WorldClientConnection(World* pWorld, EQStreamInterface* pStreamInterface);
	~WorldClientConnection();

	bool update();

	inline const bool getAuthenticated() const { return mAuthenticated; }
	inline const uint32 getIP() const { return mIP; }
	inline const uint16 getPort() const { return mPort; }

	inline const u32 getAccountID() const { return mAccountID; }
	void _sendChatServer(const String& pCharacterName);
	void _sendZoneServerInfo(const uint16 pPort);
private:

	inline void setAccountID(const uint32 pAccountID) { mAccountID = pAccountID; mAuthentication.mLoginServerAccountID = pAccountID; }
	void _setAuthenticated(const bool pAuthenticated) { mAuthenticated = pAuthenticated; }

	void _sendCharacterSelectInfo();
	void _sendGuildList();
	void _sendEnterWorld(String pCharacterName);
	void _sendExpansionInfo();
	void _sendLogServer();
	void _sendApproveWorld();
	void _sendPostEnterWorld();
	void _sendZoneUnavailable();

	bool _handlePacket(const EQApplicationPacket* pPacket);
	bool _handleGenerateRandomNamePacket(const EQApplicationPacket* pPacket);
	bool _handleCharacterCreatePacket(const EQApplicationPacket* pPacket);
	bool _handleEnterWorldPacket(const EQApplicationPacket* pPacket);
	bool _handleDeleteCharacterPacket(const EQApplicationPacket* pPacket);
	bool _handleSendLoginInfoPacket(const EQApplicationPacket* packet);
	bool _handleCharacterCreateRequestPacket(const EQApplicationPacket* packet);
	bool _handleNameApprovalPacket(const EQApplicationPacket* packet);

	inline void dropConnection() { mConnectionDropped = true; }

	bool mConnectionDropped = false;
	bool mZoning = false;
	bool mAuthenticated = false;
	ClientAuthentication mAuthentication;
	uint16 mPort = 0;
	uint32 mIP = 0;
	uint32 mAccountID = 0;
	String mReservedCharacterName = "";
	uint32 ClientVersionBit = 0;

	EQStreamInterface* const mStreamInterface;
	World* mWorld = nullptr;
};