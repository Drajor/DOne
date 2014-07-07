#pragma once

class EmuTCPServer;
class LoginServerConnection;
class UCSConnection;
class EQStreamFactory;
class EQStreamIdentifier;
class ZoneManager;
class DataStore;

class World {
public:
	World(DataStore* pDataStore);
	~World();
	bool initialise();
	void update();

	void _checkUCSConnection();

	// Return whether World is connected to the Login Server.
	bool isLoginServerConnected();
private:
	void _handleIncomingConnections();

	bool mInitialised;
	EmuTCPServer* mTCPServer;
	LoginServerConnection* mLoginServerConnection;
	UCSConnection* mUCSConnection;
	EQStreamFactory* mStreamFactory;
	EQStreamIdentifier* mStreamIdentifier;
	ZoneManager* mZoneManager;
	DataStore* mDataStore;
};