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
	inline const u32 getIP() const { return mIP; }
	inline const u16 getPort() const { return mPort; }

	inline const u32 getAccountID() const { return mAccountID; }
	void _sendChatServer(const String& pCharacterName);
	void sendZoneServerInfo(const String& pIP, const u16 pPort);

	void sendPacket(const EQApplicationPacket* pPacket);
private:

	inline void setAccountID(const u32 pAccountID) { mAccountID = pAccountID; }
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
	bool _handleEnterWorld(const EQApplicationPacket* pPacket);
	bool _handleDeleteCharacterPacket(const EQApplicationPacket* pPacket);
	bool _handleConnect(const EQApplicationPacket* packet);
	bool _handleCharacterCreateRequestPacket(const EQApplicationPacket* packet);
	bool _handleNameApprovalPacket(const EQApplicationPacket* packet);

	inline void dropConnection() { mConnectionDropped = true; }

	bool mConnectionDropped = false;
	bool mZoning = false;
	bool mAuthenticated = false;
	u16 mPort = 0;
	u32 mIP = 0;
	u32 mAccountID = 0;
	String mReservedCharacterName = "";
	u32 ClientVersionBit = 0;

	EQStreamInterface* const mStreamInterface;
	World* mWorld = nullptr;
};