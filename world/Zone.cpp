#include "Zone.h"
#include "World.h"
#include "Character.h"
#include "ZoneClientConnection.h"
#include "../common/types.h"
#include "../common/EQStreamFactory.h"
#include "../common/EQStreamIdent.h"
#include "../common/patches/patches.h"
#include "LogSystem.h"

Zone::Zone(World* pWorld, DataStore* pDataStore, uint32 pPort, uint32 pZoneID, uint32 pInstanceID) :
	mWorld(pWorld),
	mDataStore(pDataStore),
	mPort(pPort),
	mID(pZoneID),
	mInstanceID(pInstanceID),
	mInitialised(false),
	mStreamFactory(nullptr),
	mStreamIdentifier(nullptr),
	mNextSpawnID(1)
{
}

Zone::~Zone() {
	safe_delete(mStreamFactory);
	safe_delete(mStreamIdentifier);
}

bool Zone::initialise() {
	// Prevent multiple initialisation.
	if (mInitialised) return false;

	// Create and initialise EQStreamFactory.
	mStreamFactory = new EQStreamFactory(ZoneStream);
	if (!mStreamFactory->Open(mPort)) {
		return false;
	}

	mStreamIdentifier = new EQStreamIdentifier;
	RegisterAllPatches(*mStreamIdentifier);

	mInitialised = true;
	return true;
}

void Zone::addAuthentication(ClientAuthentication& pAuthentication, std::string pCharacterName) {
	mAuthenticatedCharacters.insert(std::make_pair(pCharacterName, pAuthentication));
}

void Zone::removeAuthentication(std::string pCharacterName) {
	mAuthenticatedCharacters.erase(pCharacterName);
}

bool Zone::checkAuthentication(std::string pCharacterName) {
	for (auto i : mAuthenticatedCharacters) {
		if (i.first == pCharacterName) {
			Log::info("[Zone] Authentication Passed");
			return true;
		}
	}

	Log::error("[Zone] checkAuthentication is returning false. This is unexpected.");
	return false;
}

void Zone::update() {
	// Check if any new clients are connecting to this Zone.
	_handleIncomingConnections();

	for (auto i : mZoneClientConnections)
		i->update();

	for (auto i = mCharacters.begin(); i != mCharacters.end();) {
		// Remove Characters that logged out.
		if ((*i)->getLoggedOut()) {
			ZoneClientConnection* connection = (*i)->getConnection();
			mZoneClientConnections.remove(connection);
			//mZoneClientConnections.erase(connection);
			delete connection;
			delete *i;
			i = mCharacters.erase(i);
		}
		else {
			(*i)->update();
			i++;
		}
	}

	//for (auto i = mClientConnections.begin(); i != mClientConnections.end();) {
	//	if ((*i)->update()){
	//		i++;
	//	}
	//	else {
	//		delete *i;
	//		i = mClientConnections.erase(i);
	//	}
	//}
}

void Zone::shutdown()
{

}

void Zone::_handleIncomingConnections() {
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
		Log::info("[Zone] New Zone Client Connection");
		mZoneClientConnections.push_back(new ZoneClientConnection(incomingStreamInterface, mDataStore, this));
	}
}

void Zone::moveCharacter(Character* pCharacter, float pX, float pY, float pZ) {
	pCharacter->setPosition(pX, pY, pZ, 0);
	pCharacter->getConnection()->sendPosition();
}

void Zone::updateCharacterPosition(Character* pCharacter, float pX, float pY, float pZ, float pHeading){
	pCharacter->setPosition(pX, pY, pZ, pHeading);
}

void Zone::notifyCharacterLogOut(Character* pCharacter)
{
	//mWorld->notifyIncomingClient()
}

void Zone::notifyCharacterZoneOut(Character* pCharacter)
{

}

void Zone::notifyCharacterLinkDead(Character* pCharacter)
{

}
