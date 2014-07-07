#pragma once

class EmuTCPServer;
class LoginServerConnection;
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

	// Return whether World is connected to the Login Server.
	bool isLoginServerConnected();
private:
	void _handleIncomingConnections();

	bool mInitialised;
	EmuTCPServer* mTCPServer;
	LoginServerConnection* mLoginServerConnection;
	EQStreamFactory* mStreamFactory;
	EQStreamIdentifier* mStreamIdentifier;
	ZoneManager* mZoneManager;
	DataStore* mDataStore;
};