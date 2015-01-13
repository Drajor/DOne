#pragma once

#include "Types.h"

class EmuTCPConnection;
class ServerPacket;
class World;
class ILog;

class LoginServerConnection{
public:
	~LoginServerConnection();

	const bool initialise(World* pWorld, ILog* pLog, const String& pAddress, const u16 pPort);
	void update();
	const bool connect(const String& pAddress, const u16 pPort);

	void sendWorldInformation(const String& pAccount, const String& pPassword, const String& pLongName, const String& pShortName);
	void sendWorldStatus(const i32 pStatus, const i32 pPlayers, const i32 pZones);
	void sendConnectResponse(const u32 pAccountID, const u8 pResponse);

	// Login Server ID
	inline const u32 getID() { return 1; }
	
private:

	bool mInitialised = false;
	ILog* mLog = nullptr;

	void sendPacket(ServerPacket* pPacket);
	void _handleClientAuthentication(ServerPacket* pPacket);
	void _handleConnectRequest(ServerPacket * pPacket);

	EmuTCPConnection* mConnection = nullptr;
	u32 mLoginServerIP = 0;
	World* mWorld = nullptr;
};
