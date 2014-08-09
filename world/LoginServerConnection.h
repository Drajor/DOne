#pragma once

#include "Constants.h"
#include "../common/timer.h"

class EmuTCPConnection;
class ServerPacket;
class World;
class AccountManager;

class LoginServerConnection{
public:
	LoginServerConnection(World* pWorld, String pAddress, uint16 pPort, String pAccountName, String pPassword);
	~LoginServerConnection();

	bool initialise();
	void update();

	bool connect();
	bool isConnected();
	// Tells the Login Server whether work is locked, how many players / zones World has.
	void sendWorldStatus();

private:
	bool _isConnectReady();
	void _sendPacket(ServerPacket* pPacket);
	void _sendWorldInformation();
	void _handleLoginServerClientAuth(ServerPacket* pPacket);
	void _handleUserToWorldRequest(ServerPacket * pPacket);

	World* mWorld;
	EmuTCPConnection* mTCPConnection;
	String mLoginServerAddress;
	uint32 mLoginServerIP;
	uint16 mLoginServerPort;
	String mLoginAccount;
	String mLoginPassword;
	Timer mStatusUpdateTimer;
};
