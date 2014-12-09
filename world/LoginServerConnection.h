#pragma once

#include "Types.h"

class EmuTCPConnection;
class ServerPacket;
class World;

class LoginServerConnection{
public:
	LoginServerConnection(World* pWorld);
	~LoginServerConnection();

	bool initialise(const String& pAddress, const u16 pPort);
	void update();

	bool connect(const String& pAddress, const u16 pPort);
	bool isConnected();

	void sendWorldInformation(const String& pAccount, const String& pPassword, const String& pLongName, const String& pShortName);
	void sendWorldStatus(const i32 pStatus, const i32 pPlayers, const i32 pZones);
	void sendConnectResponse(const u32 pAccountID, const u8 pResponse);
	
private:
	bool _isConnectReady();
	void _sendPacket(ServerPacket* pPacket);
	void _handleClientAuthentication(ServerPacket* pPacket);
	void _handleConnectRequest(ServerPacket * pPacket);

	EmuTCPConnection* mTCPConnection = nullptr;
	u32 mLoginServerIP = 0;
	World* mWorld = nullptr;
};
