#include "World.h"
#include "Character.h"
#include "Utility.h"
#include "ZoneManager.h"
#include "AccountManager.h"
#include "DataStore.h"
#include "LoginServerConnection.h"
#include "UCS.h"

#include "../common/EQStreamFactory.h"
#include "../common/EQStreamIdent.h"
#include "../common/patches/patches.h"
#include "../common/eq_packet_structs.h"
#include "../common/extprofile.h"

#include "WorldClientConnection.h"
#include "Settings.h"

World::~World() {
	if (mStreamFactory) mStreamFactory->Close();
	// TODO: Close LoginServerConnection?

	safe_delete(mLoginServerConnection);
	safe_delete(mStreamFactory);
	safe_delete(mStreamIdentifier);
}

bool World::initialise() {
	mLog.status("Initialising.");
	EXPECTED_BOOL(mInitialised == false);

	mLocked = Settings::getLocked();

	// Create our connection to the Login Server
	mLoginServerConnection = new LoginServerConnection();
	EXPECTED_BOOL(mLoginServerConnection->initialise());

	// Create and initialise EQStreamFactory.
	mStreamFactory = new EQStreamFactory(WorldStream, Limits::World::Port);
	EXPECTED_BOOL(mStreamFactory->Open());
	mLog.info("Listening on port " + std::to_string(Limits::World::Port));

	mStreamIdentifier = new EQStreamIdentifier;
	RegisterAllPatches(*mStreamIdentifier);

	EXPECTED_BOOL(ZoneManager::getInstance().initialise());

	mInitialised = true;
	mLog.status("Initialised.");
	return true;
}

void World::update() {
	mLoginServerConnection->update();
	ZoneManager::getInstance().update();
	UCS::getInstance().update();

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
		mLog.info("Incoming Connection");
		mClientConnections.push_back(new WorldClientConnection(incomingStreamInterface));
	}
}

bool World::isLoginServerConnected() {
	return mLoginServerConnection->isConnected();
}

void World::addAuthentication(ClientAuthentication& pAuthentication) {
	// Save our copy
	ClientAuthentication* authentication = new ClientAuthentication(pAuthentication);
	mAuthenticatedClients.push_back(authentication);
}

void World::removeAuthentication(ClientAuthentication& pAuthentication) {
	for (auto i : mAuthenticatedClients) {
		// Only need to match on Account IDs
		if (i->mLoginServerAccountID == pAuthentication.mLoginServerAccountID) {
			mLog.info("Removing authentication for account " + std::to_string(i->mLoginServerAccountID));
			mAuthenticatedClients.remove(i);
			return;
		}
	}
}

bool World::checkAuthentication(WorldClientConnection* pConnection, const uint32 pAccountID, const String& pKey) {
	EXPECTED_BOOL(pConnection);
	EXPECTED_BOOL(!pConnection->getAuthenticated());

	for (auto i = mAuthenticatedClients.begin(); i != mAuthenticatedClients.end(); i++) {
		ClientAuthentication* incClient = *i;
		if (pAccountID == incClient->mLoginServerAccountID && pKey == incClient->mKey) {
			pConnection->_setAuthenticated(true);
			pConnection->setAccountID(incClient->mLoginServerAccountID);
			pConnection->setAccountName(incClient->mLoginServerAccountName);
			pConnection->setKey(incClient->mKey);
			return true;
		}
	}
	return false;
}

bool World::authenticationExists(uint32 pLoginServerID) {
	for (auto i : mAuthenticatedClients) {
		if (i->mLoginServerAccountID == pLoginServerID)
			return true;
	}

	return false;
}

void World::setLocked(bool pLocked) {
	mLocked = pLocked;
	// Notify Login Server!
	mLoginServerConnection->sendWorldStatus();
}

ResponseID World::getConnectResponse(uint32 pLoginServerAccountID) {
	// Fetch the Account Status.
	uint32 accountStatus = AccountManager::getInstance().getStatus(pLoginServerAccountID);

	// Account Suspended.
	if (accountStatus == ResponseID::SUSPENDED) return ResponseID::SUSPENDED;
	// Account Banned.
	if (accountStatus == ResponseID::BANNED) return ResponseID::BANNED;

	// Check for existing authentication
	// This prevents same account sign in.
	if (authenticationExists(pLoginServerAccountID)) return ResponseID::FULL;

	// Server is Locked (Only GM/Admin may enter)
	if (mLocked && accountStatus >= LOCK_BYPASS_STATUS) return ResponseID::ALLOWED;
	// Server is Locked and user is not a GM/Admin.
	else if (mLocked && accountStatus < LOCK_BYPASS_STATUS) return ResponseID::DENIED;

	return ResponseID::ALLOWED; // Speak friend and enter.
}

bool World::isCharacterNameUnique(String pCharacterName) { return AccountManager::getInstance().isCharacterNameUnique(pCharacterName); }

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
	// Verify that character with pCharacterName belongs to pWorldAccountID.
	const bool isOwner = AccountManager::getInstance().checkOwnership(pAccountID, pCharacterName);
	EXPECTED_BOOL(isOwner);
	return AccountManager::getInstance().deleteCharacter(pAccountID, pCharacterName);
}

const bool World::addZoneAuthentication(ClientAuthentication& pAuthentication, String pCharacterName, ZoneID pZoneID, uint32 pInstanceID) {
	return ZoneManager::getInstance().addAuthentication(pAuthentication, pCharacterName, pZoneID, pInstanceID);
}

ClientAuthentication* World::findAuthentication(uint32 pLoginServerAccountID){
	for (auto i : mAuthenticatedClients) {
		if (i->mLoginServerAccountID == pLoginServerAccountID)
			return i;
	}

	return 0;
}

bool World::ensureAccountExists(const uint32 pAccountID, const String& pAccountName) {
	if (AccountManager::getInstance().exists(pAccountID))
		return true; // Account already exists.

	// Create a new Account.
	EXPECTED_BOOL(AccountManager::getInstance().createAccount(pAccountID, pAccountName));
	return true;
}

bool World::getCharacterZoneTransfer(const String& pCharacterName, ZoneTransfer& pZoneTransfer) {
	for (auto i : mZoneTransfers) {
		if (i.mCharacterName == pCharacterName) {
			pZoneTransfer = i;
			return true;
		}
	}

	return false;
}

void World::removeZoneTransfer(const String& pCharacterName) {
	for (auto i = mZoneTransfers.begin(); i != mZoneTransfers.end(); i++){
		if (i->mCharacterName == pCharacterName) {
			mZoneTransfers.erase(i);
			return;
		}
	}
}

const bool World::handleEnterWorld(WorldClientConnection* pConnection, const String& pCharacterName, const bool pZoning) {
	EXPECTED_BOOL(pConnection);

	if (pZoning)
		return _handleZoning(pConnection, pCharacterName);

	return _handleEnterWorld(pConnection, pCharacterName);
}

bool World::_handleZoning(WorldClientConnection* pConnection, const String& pCharacterName) {
	EXPECTED_BOOL(pConnection);

	ZoneTransfer zoneTransfer;
	EXPECTED_BOOL(getCharacterZoneTransfer(pCharacterName, zoneTransfer));
	removeZoneTransfer(pCharacterName);
	//addZoneAuthentication()
	// Add Zone Authentication.

	return true;
}

bool World::_handleEnterWorld(WorldClientConnection* pConnection, const String& pCharacterName) {
	EXPECTED_BOOL(pConnection);

	// Check: CharacterData could be loaded.
	CharacterData* characterData = new CharacterData();
	if (!DataStore::getInstance().loadCharacter(pCharacterName, characterData)) {
		delete characterData;
		return false;
	}

	// Character Destination.
	const ZoneID zoneID = characterData->mZoneID;
	const InstanceID instanceID = characterData->mInstanceID;

	// Check: Destination Zone is available to pCharacter.
	if (!ZoneManager::getInstance().isZoneAvailable(zoneID, instanceID)) {
		delete characterData;
		return false;
	}

	// Create Character
	Character* character = new Character(pConnection->getAccountID(), characterData);

	// Register ZoneTransfer
	ZoneManager::getInstance().registerZoneTransfer(character, zoneID, instanceID);
	if (!addZoneAuthentication(*findAuthentication(pConnection->getAccountID()), pCharacterName, zoneID, instanceID)){
		
		return false;
	}
	ZoneManager::getInstance().addZoningCharacter(character);

	// Send the client off to the Zone.
	pConnection->_sendChatServer(pCharacterName);;
	pConnection->_sendZoneServerInfo(ZoneManager::getInstance().getZonePort(zoneID, instanceID));

	return true;
}
