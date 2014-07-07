#include "Zone.h"
#include "Character.h"
#include "../common/types.h"
#include "../common/EQStreamFactory.h"
#include "../common/EQStreamIdent.h"
#include "../common/patches/patches.h"

Zone::Zone(DataStore* pDataStore) :
	mDataStore(pDataStore),
	mInitialised(false),
	mStreamFactory(nullptr),
	mStreamIdentifier(nullptr)
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
	if (!mStreamFactory->Open(7000)) {
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
		Character* character = new Character(incomingStreamInterface);
		mCharacters.push_back(character);
	}
}
