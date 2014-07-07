#include "World.h"
#include "Utility.h"
#include "ZoneManager.h"
#include "LoginServerConnection.h"

#include "../common/EmuTCPServer.h"
#include "../common/EQStreamFactory.h"
#include "../common/EQStreamIdent.h"
#include "../common/patches/patches.h"

#include "clientlist.h"
#include "Client.h"
extern ClientList client_list;

World::World(DataStore* pDataStore) :
	mInitialised(false),
	mStreamIdentifier(0),
	mStreamFactory(0),
	mZoneManager(0),
	mTCPServer(0),
	mLoginServerConnection(0),
	mDataStore(pDataStore)
{

}

World::~World() {
	if (mStreamFactory) mStreamFactory->Close();
	if (mTCPServer) mTCPServer->Close();

	safe_delete(mTCPServer);
	safe_delete(mLoginServerConnection);
	safe_delete(mStreamFactory);
	safe_delete(mStreamIdentifier);
	safe_delete(mZoneManager);
}

bool World::initialise()
{
	// Prevent multiple initialisation.
	if (mInitialised) return false;

	// Create our connection to the Login Server
	mLoginServerConnection = new LoginServerConnection("127.0.0.1", 5998, "Admin", "Password");
	mLoginServerConnection->InitLoginServer();

	// Create and initialise EQStreamFactory.
	mStreamFactory = new EQStreamFactory(WorldStream, 9000); // [Client Limitation] World must use port 9000
	if (!mStreamFactory->Open()) {
		Utility::criticalError("Unable to open World Stream");
		return false;
	}

	mStreamIdentifier = new EQStreamIdentifier;
	RegisterAllPatches(*mStreamIdentifier);

	mZoneManager = new ZoneManager(mDataStore);

	mInitialised = true;
	return true;
}

void World::update()
{
	mLoginServerConnection->Process();

	// Check if any new clients are connecting.
	_handleIncomingConnections();
}

void World::_handleIncomingConnections() {
	// Check for incoming connections.
	EQStream* incomingStream = nullptr;
	while (incomingStream = mStreamFactory->Pop()) {
		// Hand over to the EQStreamIdentifier. (Determine which client to user has)
		mStreamIdentifier->AddStream(incomingStream);
	}

	mStreamIdentifier->Process();

	// Check for identified streams.
	EQStreamInterface* incomingStreamInterface = nullptr;
	while (incomingStreamInterface = mStreamIdentifier->PopIdentified()) {
		// TODO: Add Banned IPs check.
		// TEMP HACK
		Client* client = new Client(incomingStreamInterface, this);
		client_list.Add(client);
	}
}

bool World::isLoginServerConnected()
{
	return mLoginServerConnection->Connected();
}
