#include "World.h"
#include "ServiceLocator.h"
#include "Constants.h"
#include "Character.h"
#include "Utility.h"
#include "ZoneManager.h"
#include "AccountManager.h"
#include "DataStore.h"
#include "Data.h"
#include "LoginServerConnection.h"
#include "UCS.h"

#include "../common/EQStreamFactory.h"
#include "../common/EQStreamIdent.h"
#include "../common/patches/Underfoot.h"
#include "../common/eq_packet_structs.h"
#include "../common/extprofile.h"

#include "WorldClientConnection.h"
#include "Settings.h"
#include "TimeUtility.h"

static const int StatusUpdateInterval = 15000;

World::World() : mLog("[World]"), mStatusUpdateTimer(StatusUpdateInterval) { }

World::~World() {
	if (mStreamFactory) mStreamFactory->Close();
	// TODO: Close LoginServerConnection?

	safe_delete(mLoginServerConnection);
	safe_delete(mStreamFactory);
	safe_delete(mStreamIdentifier);
}

const bool World::initialise(DataStore* pDataStore, ZoneManager* pZoneManager, AccountManager* pAccountManager) {
	mLog.status("Initialising.");
	EXPECTED_BOOL(mInitialised == false);
	EXPECTED_BOOL(pDataStore);
	EXPECTED_BOOL(pZoneManager);
	EXPECTED_BOOL(pAccountManager);

	mDataStore = pDataStore;
	mZoneManager = pZoneManager;
	mAccountManager = pAccountManager;

	mLocked = Settings::getLocked();

	EXPECTED_BOOL(_initialiseLoginServerConnection());

	// Create and initialise EQStreamFactory.
	mStreamFactory = new EQStreamFactory(WorldStream, Limits::World::Port);
	EXPECTED_BOOL(mStreamFactory->Open());
	mLog.info("Listening on port " + std::to_string(Limits::World::Port));

	mStreamIdentifier = new EQStreamIdentifier;
	Underfoot::Register(*mStreamIdentifier);

	mInitialised = true;
	mLog.status("Initialised.");
	return true;
}

bool World::_initialiseLoginServerConnection() {
	// Create our connection to the Login Server
	mLoginServerConnection = new LoginServerConnection(this);
	EXPECTED_BOOL(mLoginServerConnection->initialise(Settings::getLSAddress(), Settings::getLSPort()));
	mLoginServerConnection->sendWorldInformation(Settings::getLSAccountName(), Settings::getLSPassword(), Settings::getServerLongName(), Settings::getServerShortName());
	_updateLoginServer();

	return true;
}

void World::update() {
	mLoginServerConnection->update();
	mZoneManager->update();
	//UCS::getInstance().update();

	// Check if any new clients are connecting.
	_handleIncomingClientConnections();

	// Update World Clients.
	for (auto i = mClientConnections.begin(); i != mClientConnections.end();) {
		if ((*i)->update()){
			i++;
		}
		else {
			delete *i;
			i = mClientConnections.erase(i);
		}
	}

	// Check: Time to update the Login Server with new numbers.
	if (mStatusUpdateTimer.Check()) {
		_updateLoginServer();
		mStatusUpdateTimer.Start();
	}
}

void World::handleConnectRequest(const u32 pAccountID) {
	mLoginServerConnection->sendConnectResponse(pAccountID, getConnectResponse(pAccountID));
}

void World::_handleIncomingClientConnections() {
	// Check for incoming connections.
	EQStream* incomingStream = nullptr;
	while (incomingStream = mStreamFactory->Pop()) {
		// Hand over to the EQStreamIdentifier. (Determine which client the user has)
		mStreamIdentifier->AddStream(incomingStream);
	}

	mStreamIdentifier->Process();

	// Check for identified streams.
	EQStreamInterface* incomingStreamInterface = nullptr;
	while (incomingStreamInterface = mStreamIdentifier->PopIdentified()) {
		mLog.error("TODO: Check Bans.");
		//mLog.info("Connection from " + incomingStreamInterface->GetRemoteIP());
		mClientConnections.push_back(new WorldConnection(this, incomingStreamInterface));
	}
}

bool World::isLoginServerConnected() {
	return mLoginServerConnection->isConnected();
}

void World::removeAuthentication(const u32 pAccountID) {
	for (auto i = mAuthentations.begin(); i != mAuthentations.end(); i++) {
		if ((*i)->mAccountID == pAccountID) {
			mAuthentations.erase(i);
			return;
		}
	}
}

const bool World::checkAuthentication(const u32 pAccountID, const String& pKey) const {
	for (auto i : mAuthentations) {
		if (i->mAccountID == pAccountID && i->mKey == pKey)
			return true;
	}
	return false;
}

const bool World::authenticationExists(const u32 pAccountID) const {
	for (auto i : mAuthentations) {
		if (i->mAccountID == pAccountID)
			return true;
	}
	return false;
}

void World::setLocked(bool pLocked) {
	mLocked = pLocked;

	// Notify Login Server.
	_updateLoginServer();
}

const u8 World::getConnectResponse(const u32 pAccountID) {
	// Fetch the Account Status.
	auto accountStatus = mAccountManager->getStatus(pAccountID);

	// Account Suspended.
	if (accountStatus == ResponseID::SUSPENDED) return ResponseID::SUSPENDED;
	// Account Banned.
	if (accountStatus == ResponseID::BANNED) return ResponseID::BANNED;

	// Check for existing authentication, this prevents same account sign in.
	if (authenticationExists(pAccountID)) return ResponseID::FULL;

	// Server is Locked (Only GM/Admin may enter)
	if (mLocked && accountStatus >= LOCK_BYPASS_STATUS) return ResponseID::ALLOWED;
	// Server is Locked and user is not a GM/Admin.
	else if (mLocked && accountStatus < LOCK_BYPASS_STATUS) return ResponseID::DENIED;

	return ResponseID::ALLOWED; // Speak friend and enter.
}

bool World::isCharacterNameUnique(String pCharacterName) { return mAccountManager->isCharacterNameUnique(pCharacterName); }

bool World::isCharacterNameReserved(String pCharacterName) {
	for (auto i : mReservedCharacterNames)
		if (i.second == pCharacterName)
			return true;
	return false;
}

void World::reserveCharacterName(const uint32 pWorldAccountID, const String pCharacterName) {
	mReservedCharacterNames.insert(std::make_pair(pWorldAccountID, pCharacterName));
}

bool World::deleteCharacter(const uint32 pAccountID, const String& pCharacterName) {
	mLog.info("Delete Character request from Account(" + std::to_string(pAccountID) + ") Name(" + pCharacterName + ")");

	// Check: Character to Account.
	const bool isOwner = mAccountManager->checkOwnership(pAccountID, pCharacterName);
	if (!isOwner) {
		mLog.error("Ownership test failed!");
		return false;
	}
	mLog.info("Ownership test passed!");

	// Check: Delete succeeds.
	const bool isDeleted = mAccountManager->deleteCharacter(pAccountID, pCharacterName);
	if (!isDeleted) {
		mLog.error("Delete Failed!");
		return false;
	}

	mLog.info("Delete Success!");
	return true;
}

const bool World::handleEnterWorld(WorldConnection* pConnection, const String& pCharacterName, const bool pZoning) {
	EXPECTED_BOOL(pConnection);

	if (pZoning)
		return _handleZoning(pConnection, pCharacterName);

	return _handleEnterWorld(pConnection, pCharacterName);
}

bool World::_handleZoning(WorldConnection* pConnection, const String& pCharacterName) {
	EXPECTED_BOOL(pConnection);

	auto character = mZoneManager->getZoningCharacter(pCharacterName);
	EXPECTED_BOOL(character);
	EXPECTED_BOOL(character->getName() == pCharacterName);

	auto zoneChange = character->getZoneChange();
	const uint16 zoneID = zoneChange.mZoneID;
	const uint16 instanceID = zoneChange.mInstanceID;

	// Check: Zone Authentication.
	EXPECTED_BOOL(character->checkZoneAuthentication(zoneID, instanceID));

	// Check: Destination Zone is available to Character.
	if (!mZoneManager->isZoneAvailable(zoneID, instanceID)) {
		return false;
	}

	// Send the client off to the Zone.
	pConnection->_sendChatServer(pCharacterName);
	pConnection->sendZoneServerInfo("127.0.0.1", mZoneManager->getZonePort(zoneID, instanceID));

	return true;
}

bool World::_handleEnterWorld(WorldConnection* pConnection, const String& pCharacterName) {
	EXPECTED_BOOL(pConnection);

	// Check: CharacterData could be loaded.
	auto characterData = new Data::Character();
	if (!mDataStore->loadCharacter(pCharacterName, characterData)) {
		delete characterData;
		return false;
	}

	// Character Destination.
	const uint16 zoneID = characterData->mZoneID;
	const uint16 instanceID = characterData->mInstanceID;

	// Check: Destination Zone is available to pCharacter.
	if (!mZoneManager->isZoneAvailable(zoneID, instanceID)) {
		delete characterData;
		return false;
	}

	// Create and initialise Character.
	auto character = new Character(pConnection->getAccountID(), characterData);
	if (!character->initialise()) {
		delete character;
		return false;
	}

	// Register Zone Change
	mZoneManager->addZoningCharacter(character);
	character->setZoneAuthentication(zoneID, instanceID);

	// Send the client off to the Zone.
	pConnection->_sendChatServer(pCharacterName);
	pConnection->sendZoneServerInfo("127.0.0.1", mZoneManager->getZonePort(zoneID, instanceID));

	return true;
}

void World::_updateLoginServer() const {
	mLoginServerConnection->sendWorldStatus(getStatus(), getPlayers(), getZones());
}

void World::handleClientAuthentication(const u32 pAccountID, const String& pAccountName, const String& pKey, const u32 pIP) {
	// Check: Account exists.
	const bool accountExists = mAccountManager->exists(pAccountID);

	// Make a new Account.
	if (!accountExists) {
		EXPECTED(mAccountManager->createAccount(pAccountID, pAccountName));
	}

	// Check: Account status.
	auto accountStatus = mAccountManager->getStatus(pAccountID);
	if (accountStatus < ResponseID::ALLOWED)
		return;

	addAuthentication(pAccountID, pAccountName, pKey, pIP);
}

void World::addAuthentication(const u32 pAccountID, const String& pAccountName, const String& pKey, const u32 pIP) {
	// Add Authentication.
	auto a = new Authentication();
	a->mAccountID = pAccountID;
	a->mAccountName = pAccountName;
	a->mKey = pKey;
	a->mIP = pIP;
	a->mExpiryTime = Time::nowSeconds() + 10000;
	mAuthentations.push_back(a);
}
