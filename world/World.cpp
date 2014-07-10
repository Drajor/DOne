#include "World.h"
#include "Utility.h"
#include "ZoneManager.h"
#include "AccountManager.h"
#include "DataStore.h"
#include "LoginServerConnection.h"
#include "UCSConnection.h"
#include "worlddb.h"

#include "../common/EmuTCPServer.h"
#include "../common/EQStreamFactory.h"
#include "../common/EQStreamIdent.h"
#include "../common/patches/patches.h"

#include "WorldClientConnection.h"

extern WorldDatabase database;

World::World(DataStore* pDataStore) :
	mInitialised(false),
	mLocked(false),
	mStreamIdentifier(0),
	mStreamFactory(0),
	mZoneManager(0),
	mAccountManager(0),
	mTCPServer(0),
	mLoginServerConnection(0),
	mUCSConnection(0),
	mDataStore(pDataStore)
{

}

World::~World() {
	if (mStreamFactory) mStreamFactory->Close();
	if (mTCPServer) mTCPServer->Close();
	if (mUCSConnection) mUCSConnection->disconnect();
	// TODO: Close LoginServerConnection?

	safe_delete(mTCPServer);
	safe_delete(mLoginServerConnection);
	safe_delete(mStreamFactory);
	safe_delete(mStreamIdentifier);
	safe_delete(mZoneManager);
	safe_delete(mAccountManager);
	safe_delete(mUCSConnection);
}

bool World::initialise()
{
	// Prevent multiple initialisation.
	if (mInitialised) return false;

	// Create our connection to the Login Server
	mLoginServerConnection = new LoginServerConnection(this, mAccountManager, "127.0.0.1", 5998, "Admin", "Password");
	if (!mLoginServerConnection->initialise()) {
		Utility::criticalError("Unable to initialise Login Server Connection");
		return false;
	}

	// Create and initialise EQStreamFactory.
	mStreamFactory = new EQStreamFactory(WorldStream, 9000); // [Client Limitation] World must use port 9000
	if (!mStreamFactory->Open()) {
		Utility::criticalError("Unable to open World Stream");
		return false;
	}

	mStreamIdentifier = new EQStreamIdentifier;
	RegisterAllPatches(*mStreamIdentifier);

	mZoneManager = new ZoneManager(mDataStore);

	mAccountManager = new AccountManager(mDataStore);
	if (!mAccountManager->initialise()) {
		Utility::criticalError("Unable to initialise Account Manager");
		return false;
	}

	mTCPServer = new EmuTCPServer();
	char errbuf[TCPConnection_ErrorBufferSize];
	mTCPServer->Open(9000, errbuf);

	mUCSConnection = new UCSConnection();

	mInitialised = true;
	return true;
}

void World::update()
{
	_checkUCSConnection();
	mUCSConnection->update();

	mLoginServerConnection->update();

	// Check if any new clients are connecting.
	_handleIncomingClientConnections();

	// Update World Clients.
	for (auto i : mClients) {
		i->process();
	}

	mDataStore->update();
}

void World::_handleIncomingClientConnections() {
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
		Utility::print("World Incoming Connection");
		mClients.push_back(new WorldClientConnection(incomingStreamInterface, this));
	}
}

bool World::isLoginServerConnected() {
	return mLoginServerConnection->isConnected();
}

void World::_checkUCSConnection() {
	EmuTCPConnection* tcpConnection = 0;
	while ((tcpConnection = mTCPServer->NewQueuePop())) {
		if (tcpConnection->GetPacketMode() == EmuTCPConnection::packetModeUCS) {
			mUCSConnection->setConnection(tcpConnection);
		}
	}
}

void World::notifyIncomingClient(uint32 pLoginServerID, std::string pLoginServerAccountName, std::string pLoginServerKey, int16 pWorldAdmin /*= 0*/, uint32 pIP /*= 0*/, uint8 pLocal /*= 0*/)
{
	IncomingClient client;
	client.mAccountID = pLoginServerID;
	client.mAccountName = pLoginServerAccountName;
	client.mKey = pLoginServerKey;
	client.mWorldAdmin = pWorldAdmin;
	client.mIP = pIP;
	client.mLocal = pLocal;
	mIncomingClients.push_back(client);
}

bool World::tryIdentify(WorldClientConnection* pConnection, uint32 pLoginServerAccountID, std::string pLoginServerKey) {
	// Check Incoming Clients that match Account ID / Key
	for (auto& i : mIncomingClients) {
		if (pLoginServerAccountID == i.mAccountID && pLoginServerKey == i.mKey && !pConnection->getIdentified()) {
			// Configure the WorldClientConnection with details from the IncomingClient.
			pConnection->setIdentified(true);
			pConnection->setLoginServerAccountID(i.mAccountID);
			pConnection->setLoginServerAccountName(i.mAccountName);
			pConnection->setLoginServerKey(i.mKey);
			pConnection->setWorldAdmin(i.mWorldAdmin);
			pConnection->setWorldAccountID(database.GetAccountIDFromLSID(pLoginServerAccountID));
			// TODO: Do I need to set IP or Local here?
			// TODO Remove IncomingClient from list.
			return true;
		}
	}
	return false;
}

void World::setLocked(bool pLocked) {
	mLocked = pLocked;
	// Notify Login Server!
	mLoginServerConnection->sendWorldStatus();
}
