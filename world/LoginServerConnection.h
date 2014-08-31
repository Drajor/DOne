#pragma once

#include "Constants.h"
#include "../common/timer.h"

class EmuTCPConnection;
class ServerPacket;

class LoginServerConnection{
public:
	LoginServerConnection();
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

	EmuTCPConnection* mTCPConnection = nullptr;
	uint32 mLoginServerIP = 0;
	Timer mStatusUpdateTimer;
};
