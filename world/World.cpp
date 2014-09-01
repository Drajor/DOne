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
#include "LogSystem.h"
#include "Settings.h"

World::~World() {
	if (mStreamFactory) mStreamFactory->Close();
	// TODO: Close LoginServerConnection?

	safe_delete(mLoginServerConnection);
	safe_delete(mStreamFactory);
	safe_delete(mStreamIdentifier);
}

bool World::initialise() {
	Log::status("[World] Initialising.");
	EXPECTED_BOOL(mInitialised == false);

	mLocked = Settings::getLocked();

	// Create our connection to the Login Server
	mLoginServerConnection = new LoginServerConnection();
	EXPECTED_BOOL(mLoginServerConnection->initialise());

	// Create and initialise EQStreamFactory.
	mStreamFactory = new EQStreamFactory(WorldStream, Limits::World::Port);
	EXPECTED_BOOL(mStreamFactory->Open());
	Log::info("[World] Listening on port " + std::to_string(Limits::World::Port));

	mStreamIdentifier = new EQStreamIdentifier;
	RegisterAllPatches(*mStreamIdentifier);

	EXPECTED_BOOL(ZoneManager::getInstance().initialise());

	mInitialised = true;
	Log::status("[World] Initialised.");
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
		// TODO: Add Banned IPs check.
		Log::info("[World] Incoming Connection");
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
			Log::info("[World] Removing Client Authentication for TODO");
			mAuthenticatedClients.remove(i);
			return;
		}
	}
}

bool World::checkAuthentication(WorldClientConnection* pConnection, const uint32 pAccountID, const String& pKey) {
	EXPECTED_BOOL(pConnection);

	// Check Incoming Clients that match Account ID / Key
	for (auto i = mAuthenticatedClients.begin(); i != mAuthenticatedClients.end(); i++) {
		ClientAuthentication* incClient = *i;
		if (pAccountID == incClient->mLoginServerAccountID && pKey == incClient->mKey && !pConnection->getAuthenticated()) {
			// Configure the WorldClientConnection with details from the IncomingClient.
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

void World::reserveCharacterName(uint32 pWorldAccountID, String pCharacterName) {
	mReservedCharacterNames.insert(std::make_pair(pWorldAccountID, pCharacterName));
}

bool World::deleteCharacter(const uint32 pAccountID, const String& pCharacterName) {
	// Verify that character with pCharacterName belongs to pWorldAccountID.
	const bool isOwner = AccountManager::getInstance().checkOwnership(pAccountID, pCharacterName);
	EXPECTED_BOOL(isOwner);
	return AccountManager::getInstance().deleteCharacter(pCharacterName);
}

//bool World::createCharacter(uint32 pWorldAccountID, String pCharacterName, CharCreate_Struct* pData) {
//	// Find reserved name.
//	auto i = mReservedCharacterNames.find(pWorldAccountID);
//	if (i == mReservedCharacterNames.end()) {
//		Log::error("Reserved character name not found during character creation!");
//		return false;
//	} else {
//		mReservedCharacterNames.erase(pWorldAccountID);
//	}
//
//	// Create and configure profile.
//	PlayerProfile_Struct profile;
//	memset(&profile, 0, sizeof(PlayerProfile_Struct));
//	strcpy(profile.name, pCharacterName.c_str());
//
//	ExtendedProfile_Struct extendedProfile;
//	memset(&extendedProfile, 0, sizeof(ExtendedProfile_Struct));
//	// TODO: Check Validity!
//	
//	profile.race = pData->race;
//	profile.class_ = pData->class_;
//	profile.gender = pData->gender;
//	profile.deity = pData->deity;
//	profile.STR = pData->STR;
//	profile.STA = pData->STA;
//	profile.AGI = pData->AGI;
//	profile.DEX = pData->DEX;
//	profile.WIS = pData->WIS;
//	profile.INT = pData->INT;
//	profile.CHA = pData->CHA;
//	profile.face = pData->face;
//	profile.eyecolor1 = pData->eyecolor1;
//	profile.eyecolor2 = pData->eyecolor2;
//	profile.hairstyle = pData->hairstyle;
//	profile.haircolor = pData->haircolor;
//	profile.beard = pData->beard;
//	profile.beardcolor = pData->beardcolor;
//	profile.drakkin_heritage = pData->drakkin_heritage;
//	profile.drakkin_tattoo = pData->drakkin_tattoo;
//	profile.drakkin_details = pData->drakkin_details;
//	profile.birthday = 0; // TODO:
//	profile.lastlogin = 0; // TODO:
//	profile.level = 1;
//	profile.points = 5;
//	profile.cur_hp = 1000;
//	profile.hunger_level = 6000;
//	profile.thirst_level = 6000;
//	profile.zone_id = ZoneIDs::NorthQeynos;
//	profile.x = 0;
//	profile.y = 0;
//	profile.z = 0;
//
//	for (int i = 0; i < MAX_PP_SPELLBOOK; i++)
//		profile.spell_book[i] = 0xFFFFFFFF;
//
//	for (int i = 0; i < MAX_PP_MEMSPELL; i++)
//		profile.mem_spells[i] = 0xFFFFFFFF;
//
//	for (int i = 0; i < BUFF_COUNT; i++)
//		profile.buffs[i].spellid = 0xFFFF;
//
//	profile.binds[0].zoneId = profile.zone_id;
//	profile.binds[0].x = profile.x;
//	profile.binds[0].y = profile.y;
//	profile.binds[0].z = profile.z;
//	profile.binds[0].heading = profile.heading;
//
//	//if (!DataStore::getInstance().createCharacter(pWorldAccountID, pCharacterName, &profile, &extendedProfile)) {
//	//	Log::error("Could not create character!"); // pCharacterName
//	//	return false;
//	//}
//
//	return true;
//}

uint16 World::getZonePort(ZoneID pZoneID, uint16 pInstanceID) {
	return ZoneManager::getInstance().getZonePort(pZoneID, pInstanceID);
}

void World::addZoneAuthentication(ClientAuthentication& pAuthentication, String pCharacterName, ZoneID pZoneID, uint32 pInstanceID) {
	ZoneManager::getInstance().addAuthentication(pAuthentication, pCharacterName, pZoneID, pInstanceID);
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

bool World::handleEnterWorld(WorldClientConnection* pConnection, const String& pCharacterName, const bool pZoning) {
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

	// Load Character data
	CharacterData* characterData = new CharacterData();
	if (!DataStore::getInstance().loadCharacter(pCharacterName, characterData)) {
		delete characterData;
		return false;
	}

	// Create Character
	Character* character = new Character(pConnection->getAccountID(), characterData);

	// Register ZoneTransfer
	ZoneManager::getInstance().registerZoneTransfer(character, characterData->mZoneID, characterData->mInstanceID);
	addZoneAuthentication(*findAuthentication(pConnection->getAccountID()), pCharacterName, characterData->mZoneID, characterData->mInstanceID);
	ZoneManager::getInstance().onLeaveZone(character);

	// Send the client off to the Zone.
	pConnection->_sendChatServer(pCharacterName);;
	pConnection->_sendZoneServerInfo(ZoneManager::getInstance().getZonePort(characterData->mZoneID, characterData->mInstanceID));

	return true;
}
