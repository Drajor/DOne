#include "World.h"
#include "Utility.h"
#include "ZoneManager.h"
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
	mDataStore(pDataStore)
{

}

World::~World() {
	if (mStreamFactory) mStreamFactory->Close();

	safe_delete(mStreamFactory);
	safe_delete(mStreamIdentifier);
	safe_delete(mZoneManager);
}

bool World::initialise()
{
	// Prevent multiple initialisation.
	if (mInitialised) return false;

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
		Client* client = new Client(incomingStreamInterface);
		client_list.Add(client);
	}
}