#include "Zone.h"
#include "Character.h"
#include "ZoneClientConnection.h"
#include "../common/types.h"
#include "../common/EQStreamFactory.h"
#include "../common/EQStreamIdent.h"
#include "../common/patches/patches.h"
#include "LogSystem.h"

Zone::Zone(DataStore* pDataStore, uint32 pPort, uint32 pZoneID, uint32 pInstanceID) :
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

void Zone::update() {
	// Check if any new clients are connecting to this Zone.
	_handleIncomingConnections();

	for (auto i : mZoneClientConnections)
		i->update();
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
		//Character* character = new Character(incomingStreamInterface);
		//mCharacters.push_back(character);
		Log::info("[Zone] New Zone Client Connection");
		mZoneClientConnections.push_back(new ZoneClientConnection(incomingStreamInterface, mDataStore, this));
	}
}

bool Zone::isClientExpected(std::string pCharacterName) {
	return true;
	// TODO: Sort this out.
	//for (auto i : mExpectedCharacters) {
	//	if (i == pCharacterName)
	//		return true;
	//}
	//return false;
}

void Zone::addExpectedCharacter(std::string pCharacterName)
{
	mExpectedCharacters.push_back(pCharacterName);
}

void Zone::removeExpectedCharacter(std::string pCharacterName)
{
	// TODO: Check that pCharacterName is actually in mExpectedCharacters.
	mExpectedCharacters.remove(pCharacterName);
}

void Zone::moveCharacter(Character* pCharacter, float pX, float pY, float pZ)
{
	pCharacter->setPosition(pX, pY, pZ);
	pCharacter->getConnection()->sendPosition();
}
