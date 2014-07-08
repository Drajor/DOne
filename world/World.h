#pragma once

#include "../common/types.h"
#include <list>
#include <string>

class EmuTCPServer;
class LoginServerConnection;
class UCSConnection;
class EQStreamFactory;
class EQStreamIdentifier;
class ZoneManager;
class DataStore;
class WorldClientConnection;

class World {
public:
	World(DataStore* pDataStore);
	~World();
	bool initialise();
	void update();

	void notifyIncomingClient(uint32 pLoginServerID, std::string pLoginServerAccountName, std::string pLoginServerKey, int16 pWorldAdmin = 0, uint32 pIP = 0, uint8 pLocal = 0);
	bool tryIdentify(WorldClientConnection* pConnection, uint32 pLoginServerAccountID, std::string pLoginServerKey);

	void _checkUCSConnection();

	// Return whether World is connected to the Login Server.
	bool isLoginServerConnected();
private:
	struct IncomingClient {
		uint32 mAccountID; // Login Server Account
		std::string mAccountName; // Login Server Account
		std::string mKey;
		int16 mWorldAdmin;
		uint32 mIP;
		uint8 mLocal;
	};
	std::list<IncomingClient> mIncomingClients; // These are Clients the Login Server has told us about but have not yet fully connected to the World.

	void _handleIncomingClientConnections();

	bool mInitialised;
	EmuTCPServer* mTCPServer;
	LoginServerConnection* mLoginServerConnection;
	UCSConnection* mUCSConnection;
	EQStreamFactory* mStreamFactory;
	EQStreamIdentifier* mStreamIdentifier;
	ZoneManager* mZoneManager;
	DataStore* mDataStore;

	std::list<WorldClientConnection*> mClients;
};