#include "World.h"
#include "ServiceLocator.h"
#include "EventDispatcher.h"
#include "Constants.h"
#include "Character.h"
#include "Inventory.h"
#include "Utility.h"
#include "GuildManager.h"
#include "ZoneManager.h"
#include "AccountManager.h"
#include "Account.h"
#include "IDataStore.h"
#include "Data.h"
#include "LoginServerConnection.h"
#include "UCS.h"
#include "Limits.h"
#include "Payload.h"
#include "CharacterFactory.h"

#include "../common/EQStreamFactory.h"
#include "../common/EQStreamIdent.h"
#include "../common/patches/Underfoot.h"
#include "../common/eq_packet_structs.h"
#include "../common/extprofile.h"

#include "WorldConnection.h"
#include "Settings.h"
#include "TimeUtility.h"

static const int StatusUpdateInterval = 15000;

World::World() : mStatusUpdateTimer(StatusUpdateInterval) { }

World::~World() {
	if (mStreamFactory) mStreamFactory->Close();
	// TODO: Close LoginServerConnection?

	safe_delete(mLoginServerConnection);
	safe_delete(mStreamFactory);
	safe_delete(mStreamIdentifier);
}

const bool World::initialise(IDataStore* pDataStore, ILogFactory* pLogFactory, CharacterFactory* pCharacterFactory, GuildManager* pGuildManager, ZoneManager* pZoneManager, AccountManager* pAccountManager) {
	if (mInitialised) return false;
	if (!pDataStore) return false;
	if (!pLogFactory) return false;
	if (!pCharacterFactory) return false;
	if (!pGuildManager) return false;
	if (!pZoneManager) return false;
	if (!pAccountManager) return false;

	mDataStore = pDataStore;
	mLogFactory = pLogFactory;
	mCharacterFactory = pCharacterFactory;
	mGuildManager = pGuildManager;
	mZoneManager = pZoneManager;
	mAccountManager = pAccountManager;

	// Create and configure World log.
	mLog = mLogFactory->make();
	mLog->setContext("[World]");
	mLog->status("Initialising.");

	mLocked = Settings::getLocked();

	// Create LoginServerConnection.
	mLoginServerConnection = new LoginServerConnection();
	if (!mLoginServerConnection->initialise(this,  mLogFactory->make(), Settings::getLSAddress(), Settings::getLSPort())) {
		mLog->error("LoginServerConnection failed to initialise.");
		return false;
	}
	mLoginServerConnection->sendWorldInformation(Settings::getLSAccountName(), Settings::getLSPassword(), Settings::getServerLongName(), Settings::getServerShortName());
	updateLoginServer();

	// Create EQStreamFactory.
	mStreamFactory = new EQStreamFactory(WorldStream, Limits::World::Port);
	if (!mStreamFactory->Open()) {
		mLog->error("Failed to start EQStreamFactory.");
		return false;
	}
	mLog->info("Listening on port " + std::to_string(Limits::World::Port));

	mStreamIdentifier = new EQStreamIdentifier;
	Underfoot::Register(*mStreamIdentifier);

	mInitialised = true;
	mLog->status("Finished initialising.");
	return true;
}

void World::update() {
	mLoginServerConnection->update();
	mZoneManager->update();
	//UCS::getInstance().update();

	// Check if any new clients are connecting.
	checkIncomingConnections();

	// Update World Clients.
	for (auto i = mWorldConnections.begin(); i != mWorldConnections.end();) {
		auto connection = *i;
		if (connection->update()){
			i++;
			continue;
		}

		// This basically detects when a user quits from the Character Select Screen.
		if (connection->isZoning() == false && connection->hasAccount()) {
			mLog->info("WorldConnection dropped from Character Select Screen. Cleaning up.");
			if (!mAccountManager->onDisconnect(connection->getAccount())) {
				mLog->error("AccountManager::onDisconnect failed.");
			}
		}

		delete connection;
		i = mWorldConnections.erase(i);
	}

	// Check: Time to update the Login Server with new numbers.
	if (mStatusUpdateTimer.Check()) {
		updateLoginServer();
		mStatusUpdateTimer.Start();
	}
}

void World::onConnectRequest(LoginServerConnection* pConnection, const u32 pLoginAccountID) {
	if (!pConnection) return;

	const auto loginServerID = pConnection->getID();
	auto account = mAccountManager->getAccount(pLoginAccountID, loginServerID);

	// Account does not exist yet.
	if (!account) {
		// Server is currently locked, do not let them in.
		if (isLocked()) {
			mLog->info("Unknown Account attempted connection to locked server.");
			pConnection->sendConnectResponse(pLoginAccountID, ResponseID::Denied);
			return;
		}

		// Server is not locked, let them in.
		mLog->info("Unknown Account connecting to server.");
		pConnection->sendConnectResponse(pLoginAccountID, ResponseID::Allowed);
		return;
	}

	// Check: Account is already logged in.
	if (account->hasAuthentication()) {
		mLog->info("Account is already logged in, denying connection.");
		pConnection->sendConnectResponse(pLoginAccountID, ResponseID::Full);
		return;
	}

	// Check: Suspended Account time may have expired.
	if (account->isSuspended()) {
		mAccountManager->checkSuspension(account);
	}
	
	// Account is banned.
	if (account->isBanned()) {
		mLog->info("Banned Account attempted connection.");
		pConnection->sendConnectResponse(pLoginAccountID, ResponseID::Banned);
		return;
	}

	// Account is suspended.
	if (account->isSuspended()) {
		mLog->info("Suspended Account attempted connection.");
		pConnection->sendConnectResponse(pLoginAccountID, ResponseID::Suspended);
		return;
	}

	// Server is currently locked.
	if (isLocked()) {
		// Account has enough status to bypass lock.
		if (account->getStatus() >= AccountStatus::BypassLock) {
			mLog->info("Server Locked: connection allowed.");
			pConnection->sendConnectResponse(pLoginAccountID, ResponseID::Allowed);
			return;
		}
		// Account does not have enough status to bypass lock.
		else {
			mLog->info("Server Locked: connection denied.");
			pConnection->sendConnectResponse(pLoginAccountID, ResponseID::Denied);
		}
	}

	// Speak friend and enter.
	mLog->info("Connection allowed.");
	pConnection->sendConnectResponse(pLoginAccountID, ResponseID::Allowed);
}

void World::checkIncomingConnections() {
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
		auto log = mLogFactory->make();
		auto connection = new WorldConnection();
		if (!connection->initialise(this, log, incomingStreamInterface)) {
			mLog->error("Failure: Initialising WorldConnection");
			delete log;
			delete connection;
			continue;
		}

		mLog->info("Added new WorldConnection.");
		mWorldConnections.push_back(connection);
	}
}

void World::setLocked(bool pLocked) {
	mLocked = pLocked;

	// Notify Login Server.
	updateLoginServer();
}

const bool World::_handleZoning(WorldConnection* pConnection, const String& pCharacterName) {
	if (!pConnection) return false;
	if (!pConnection->hasAccount()) return false;

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
	pConnection->sendChatServer(pCharacterName);
	pConnection->sendZoneServerInfo("127.0.0.1", mZoneManager->getZonePort(zoneID, instanceID));

	return true;
}

bool World::_handleEnterWorld(WorldConnection* pConnection, const String& pCharacterName) {
	if (!pConnection) return false;
	if (!pConnection->hasAccount()) return false;

	auto account = pConnection->getAccount();

	// Check: Account owns Character.
	if (!account->ownsCharacter(pCharacterName)) return false;

	// Create Character.
	auto character = mCharacterFactory->make(pCharacterName, account);
	if (!character) {
		mLog->error("Failed to make Character: " + pCharacterName);
		return false;
	}

	// Character Destination.
	auto data = character->getData();
	const u16 zoneID = data->mZoneID;
	const u16 instanceID = data->mInstanceID;

	// Check: Destination Zone is available to pCharacter.
	if (!mZoneManager->isZoneAvailable(zoneID, instanceID)) {
		mLog->error("Zone unavailable to Character: " + pCharacterName);
		delete character;
		return false;
	}

	if (character->getGuildID() != 0xFFFFFFFF)
		mGuildManager->onConnect(character, character->getGuildID());

	// Register Zone Change
	mZoneManager->onLeaveZone(character);
	character->setZoneAuthentication(zoneID, instanceID);

	// Send the client off to the Zone.
	pConnection->sendChatServer(pCharacterName);
	pConnection->sendZoneServerInfo("127.0.0.1", mZoneManager->getZonePort(zoneID, instanceID));

	return true;
}

void World::updateLoginServer() const {
	mLoginServerConnection->sendWorldStatus(getStatus(), numPlayers(), numZones());
}

void World::onAuthentication(LoginServerConnection* pConnection, const u32 pLoginAccountID, const String& pLoginAccountName, const String& pLoginKey, const u32 pIP) {
	const auto loginServerID = pConnection->getID();
	
	// Find existing Account.
	auto account = mAccountManager->getAccount(pLoginAccountID, loginServerID);

	// Account does not exist, make one.
	if (!account) {
		account = mAccountManager->createAccount(pLoginAccountID, pLoginAccountName, loginServerID);

		// Account creation failed.
		if (!account) {
			// LOG
			return;
		}
	}

	// Check: Account status is allowed.
	if (account->getStatus() < AccountStatus::Default) {
		// LOG
		return;
	}

	account->setAuthentication(pLoginKey, pIP);
}


const bool World::onConnect(WorldConnection* pConnection, const u32 pLoginAccountID, const String& pKey, const bool pZoning) {
	if (!pConnection) return false;
	if (pConnection->hasAccount()) return false;

	auto account = mAccountManager->getAuthenticatedAccount(pLoginAccountID, pKey, pConnection->getIP());

	if (!account) {
		struct Bypass {
			u32 mLoginAccountID;
			u32 mLoginServerID;
			String mKey;
		};
		std::list<Bypass> bypasses;
		bypasses.push_back({ 2, 1, "passwords" });
		bypasses.push_back({ 3, 1, "passwords" });

		// Find bypass.
		bool bypassFound = false;
		for (auto i : bypasses) {
			if (i.mLoginAccountID == pLoginAccountID && i.mKey == pKey) {
				bypassFound = true;
				account = mAccountManager->getAccount(i.mLoginAccountID, i.mLoginServerID);
				if (!account) {
					// LOG
					return false;
				}
			}
		}

		// No authenticated Account and no bypass found.
		if (!bypassFound) {
			return false;
		}
	}

	account->setAuthentication(pKey, pConnection->getIP());
	pConnection->setAccount(account);

	// Going to: Another Zone
	if (pZoning) {
		auto character = account->getActiveCharacter();
		if (!character) {
			// LOG. This is really bad.
			return false;
		}

		pConnection->sendLogServer();
		pConnection->sendApproveWorld();
		pConnection->sendEnterWorld(character->getName());
		pConnection->sendPostEnterWorld();
	}
	// Going to: Character Selection Screen.
	else {
		if (!account->isLoaded()) {
			if (!mAccountManager->onConnect(account)){
				// LOG/ This is really bad.
				return false;
			}
		}

		pConnection->sendGuildList(); // NOTE: Required. Character guild names do not work (on entering world) without it.
		pConnection->sendLogServer();
		pConnection->sendApproveWorld();
		pConnection->sendEnterWorld(""); // Empty character name when coming from Server Select. 
		pConnection->sendPostEnterWorld(); // Required.
		pConnection->sendExpansionInfo(); // Required.
		pConnection->sendCharacterSelectInfo(); // Required.
	}

	/*
	OP_GuildsList, OP_LogServer, OP_ApproveWorld
	All sent in EQEmu but not actually required to get to Character Select. More research on the effects of not sending are required.
	*/

	return true;
}

const bool World::onApproveName(WorldConnection* pConnection, const String& pCharacterName) {
	if (!pConnection) return false;
	if (!pConnection->hasAccount()) return false;

	auto account = pConnection->getAccount();

	// NOTE: Unfortunately I can not find a better place to prevent accounts from going over the maximum number of characters.
	// So we check here and just reject the name if the account is at max.
	// It would be better if I could figure out how the client limits it and duplicate that.
	if (account->numCharacters() >= Limits::Account::MAX_NUM_CHARACTERS) {
		pConnection->sendApproveNameResponse(false);
		return true;
	}

	const bool nameAllowed = mAccountManager->isCharacterNameAllowed(pCharacterName);

	// Reserve name for Account.
	if (nameAllowed) {
		account->setReservedCharacterName(pCharacterName);
	}

	pConnection->sendApproveNameResponse(nameAllowed);
	return true;
}

const bool World::onDeleteCharacter(WorldConnection* pConnection, const String& pCharacterName) {
	if (!pConnection) return false;
	if (!pConnection->hasAccount()) return false;

	auto account = pConnection->getAccount();

	// Check: Account owns Character.
	if (!account->ownsCharacter(pCharacterName)) return false;

	// Delete the Character.
	if (mAccountManager->deleteCharacter(account, pCharacterName)) {
		mLog->info("Success: Character deleted.");
		pConnection->sendCharacterSelectInfo();
		return true;
	}

	mLog->error("Failure: Character failed to be deleted.");
	return false;
}

const bool World::onCreateCharacter(WorldConnection* pConnection, Payload::World::CreateCharacter* pPayload) {
	if (!pConnection) return false;
	if (!pConnection->hasAccount()) return false;
	if (!pPayload) return false;

	auto account = pConnection->getAccount();

	// Create the Character.
	if (mAccountManager->createCharacter(account, pPayload)) {
		mLog->info("Success: Character created.");
		pConnection->sendCharacterSelectInfo();
		return true;
	}

	mLog->error("Failure: Character failed to be created.");
	return false;
}

const bool World::onEnterWorld(WorldConnection* pConnection, const String& pCharacterName, const bool pZoning) {
	if (!pConnection) return false;
	if (!pConnection->hasAccount()) return false;

	auto account = pConnection->getAccount();

	// Check: Account owns the Character.
	if (!account->ownsCharacter(pCharacterName)) return false;

	bool success = false;
	if (pZoning) {
		success = _handleZoning(pConnection, pCharacterName);
	}
	else {
		success = _handleEnterWorld(pConnection, pCharacterName);
	}

	// World Entry Failed
	if (!success) {
		// Character Select to World -OR- Character Create to World.
		if (!pZoning) {
			pConnection->sendZoneUnavailable();
			pConnection->sendCharacterSelectInfo();
			// NOTE: This just bumps the client to Character Select screen.
			return true;
		}
		// Zoning
		else {
			// TODO:
		}
		return true;
	}

	return success;
}

void World::onLeaveWorld(Character* pCharacter) {
	if (!pCharacter) return;

	auto account = pCharacter->getAccount();
	
	// LinkDead Character leaving World.
	if (pCharacter->isLinkDead()) {
		mLog->info("LinkDead Character (" + pCharacter->getName() + ") leaving world.");
		mAccountManager->onDisconnect(account);
	}
	// Camping Character leaving World.
	else if (pCharacter->isCampComplete()) {
		mLog->info("Camping Character (" + pCharacter->getName() + ") leaving world.");
		account->clearActiveCharacter();
	}
	else {
		mLog->error("Character leaving world under unknown circumstances.");
		mAccountManager->onDisconnect(account);
	}

	delete pCharacter;
}
