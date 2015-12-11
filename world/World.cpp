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

const bool World::saveCharacter(Character* pCharacter) {
	if (!pCharacter) return false;

	//// Save Character.
	//if (!mDataStore->saveCharacter(pCharacter->getName(), pCharacter->getData())) {
	//	mLog->error("Failed to save Character: " + pCharacter->getName());
	//	return false;
	//}

	return true;
}

void World::onConnectRequest(LoginServerConnection* pConnection, const u32 pLSAccountID) {
	if (!pConnection) return;

	const auto response = mAccountManager->onConnectRequest(pLSAccountID, pConnection->getID(), isLocked());
	pConnection->sendConnectResponse(pLSAccountID, response);
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
		auto connection = new WorldConnection();
		if (!connection->initialise(this, mLogFactory, incomingStreamInterface)) {
			mLog->error("Failure: Initialising WorldConnection");
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

	// Check: Account is valid.
	auto account = pConnection->getAccount();
	if (!account) {
		mLog->error("Null Account in " + String(__FUNCTION__));
		return false;
	}

	// Check: Account Character is valid.
	auto character = account->getActiveCharacter();
	if (!character) {
		mLog->error("Null Character in " + String(__FUNCTION__));
		return false;
	}

	// Check: Character names match.
	if (character->getName() != pCharacterName) {
		mLog->error("Character name mismatch in " + String(__FUNCTION__) + " got: " + pCharacterName + " expected: " + character->getName());
		return false;
	}

	auto zoneChange = character->getZoneChange();
	const u16 zoneID = zoneChange.mZoneID;
	const u16 instanceID = zoneChange.mInstanceID;

	// Check: Zone Authentication.
	if (!character->checkZoneAuthentication(zoneID, instanceID)) {
		mLog->error("Character not authenticated to enter zone.");
		return false;
	}

	// Check: Destination Zone is available to Character.
	if (!mZoneManager->isZoneAvailable(zoneID, instanceID)) {
		mLog->error("Zone unavailable.");
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
	auto ownership = false;
	if (!mDataStore->accountOwnsCharacter(account->getAccountID(), pCharacterName, ownership)) {
		mLog->error("Failed to check Character ownership!");
		return false;
	}
	if (!ownership) {
		mLog->error("Ownership check failed for Character: " + pCharacterName);
		return false;
	}

	// Load Character.
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
		return false;
	}

	if (character->getGuildID() != 0xFFFFFFFF)
		mGuildManager->onConnect(character.get(), character->getGuildID());

	// Register Zone Change
	mZoneManager->onLeaveZone(character.get());
	character->setZoneAuthentication(zoneID, instanceID);

	// Send the client off to the Zone.
	pConnection->sendChatServer(pCharacterName);
	pConnection->sendZoneServerInfo("127.0.0.1", mZoneManager->getZonePort(zoneID, instanceID));

	// Associated Account and Character.
	account->setActiveCharacter(character);
	return true;
}

void World::updateLoginServer() const {
	mLoginServerConnection->sendWorldStatus(getStatus(), numPlayers(), numZones());
}

void World::onAuthentication(LoginServerConnection* pConnection, const u32 pLSAccountID, const String& pLSAccountName, const String& pKey, const u32 pIP) {
	const auto loginServerID = pConnection->getID();

	// Check: Has this Account been created yet?
	if (!mAccountManager->isCreated(pLSAccountID, pConnection->getID())) {
		// Account does not exist, create it.
		auto account = mAccountManager->create(pLSAccountID, pLSAccountName, loginServerID);

		// Account creation failed.
		if (!account) {
			// NOTE: This failure is logged within AccountManager::createAccount
			return;
		}

		// Set authentication on newly created Account.
		account->setAuthentication(pKey, pIP);
		return;
	}


	// Check: Is this Account already connected?
	else if(mAccountManager->isConnected(pLSAccountID, pLSAccountID)) {
		// This should never occur as an Account that is already connect is blocked in onConnectRequest().
		return;
	}
	
	auto account = mAccountManager->load(pLSAccountID, pConnection->getID());
	if (!account) {
		// TODO: LOG
		return;
	}

	// Check: Account status is allowed.
	if (account->getStatus() < AccountStatus::Default) {
		// TODO: LOG
		return;
	}

	account->setAuthentication(pKey, pIP);
}


const bool World::onConnect(WorldConnection* pConnection, const u32 pLSAccountID, const String& pKey, const bool pZoning) {
	if (!pConnection) return false;
	if (pConnection->hasAccount()) return false;
	if (!mAccountManager) return false;

	// Handle: Going to Character Selection.
	if (!pZoning) {
		
		// Check: Account loaded successfully.
		auto account = std::make_shared<Account>(mAccountManager);
		if (!mDataStore->accountLoad(account.get(), pLSAccountID, 1)) {
			// TODO: Log.
			return false;
		}

		// Check: Account authentication matches.
		if (account->getKey() != pKey || account->getIP() != pConnection->getIP()) {
			// TODO: Log.
			return false;
		}

		// Notify AccountManager.
		if (!mAccountManager->onConnect(account)) {
			// TODO: Log.
			return false;
		}

		pConnection->setAccount(account);

		// NOTE: Required. Character guild names do not work (on entering world) without it.
		auto packet = Payload::makeGuildNameList(mGuildManager->getGuilds());
		pConnection->sendPacket(packet);
		delete packet;

		pConnection->sendLogServer();
		pConnection->sendApproveWorld();
		pConnection->sendEnterWorld(""); // Empty character name when coming from Server Select. 
		pConnection->sendPostEnterWorld(); // Required.
		pConnection->sendExpansionInfo(); // Required.
		sendCharacterSelection(pConnection);

		return true;
	}
	// Handle: Zoning.
	else {

		// Check: Account is currently connected.
		auto account = mAccountManager->_find(pLSAccountID, 1);
		if (!account) {
			// TODO: Log.
			return false;
		}

		// Check: Account has an active Character.
		auto character = account->getActiveCharacter();
		if (!character) {
			mLog->error("Got null Character while zoning");
			return false;
		}

		pConnection->sendLogServer();
		pConnection->sendApproveWorld();
		pConnection->sendEnterWorld(character->getName());
		pConnection->sendPostEnterWorld();

		return true;
	}
}

const bool World::onApproveName(WorldConnection* pConnection, const String& pCharacterName) {
	if (!pConnection) return false;
	if (!pConnection->hasAccount()) return false;

	auto account = pConnection->getAccount();

	// NOTE: Unfortunately I can not find a better place to prevent accounts from going over the maximum number of characters.
	// So we check here and just reject the name if the account is at max.
	// It would be better if I could figure out how the client limits it and duplicate that.
	//if (account->numCharacters() >= Limits::Account::MAX_NUM_CHARACTERS) {
	//	pConnection->sendApproveNameResponse(false);
	//	return true;
	//}

	const bool allowed = mAccountManager->isCharacterNameAllowed(pCharacterName);

	// Reserve name for Account.
	if (allowed) {
		account->setReservedCharacterName(pCharacterName);
	}

	pConnection->sendApproveNameResponse(allowed);
	return true;
}

const bool World::onDeleteCharacter(WorldConnection* pConnection, const String& pCharacterName) {
	// A WorldConnection is attempting to delete a Character.
	// This only occurs when the delete button is pressed at the Character Selection Screen.

	// Check: WorldConnection is valid.
	if (!pConnection) {
		mLog->error("Null WorldConnection in " + String(__FUNCTION__));
		return false;
	}

	auto account = pConnection->getAccount();

	// Check: WorldConnection has an Account.
	if (!account) {
		mLog->error("Attempt to delete Character when WorldConnection has no Account.");
		return false;
	}
	
	// Check: Account owns Character.
	auto ownership = false;
	if (!mDataStore->accountOwnsCharacter(account->getAccountID(), pCharacterName, ownership)) {
		mLog->error("Failed to check Character ownership!");
		return false;
	}
	if (!ownership) {
		mLog->error("Ownership check failed for Character: " + pCharacterName);
		return false;
	}

	// Delete Character.
	if (!mDataStore->characterDelete(pCharacterName)) {
		mLog->error("Failed to delete " + pCharacterName);
		return false;
	}
	mLog->info("Deleted " + pCharacterName);

	// Notify GuildManager
	mGuildManager->onCharacterDelete(pCharacterName);

	sendCharacterSelection(pConnection);
	return true;
}

const bool World::onCreateCharacter(WorldConnection* pConnection, Payload::World::CreateCharacter* pPayload) {
	if (!pConnection) return false;
	if (!pConnection->hasAccount()) return false;
	if (!pPayload) return false;

	auto account = pConnection->getAccount();

	// Create the Character.
	if (mAccountManager->createCharacter(account, pPayload)) {
		mLog->info("Success: Character created.");
		sendCharacterSelection(pConnection);
		return true;
	}

	mLog->error("Failure: Character failed to be created.");
	return false;
}

const bool World::onEnterWorld(WorldConnection* pConnection, const String& pCharacterName, const bool pZoning) {
	if (!pConnection) return false;
	if (!pConnection->hasAccount()) return false;

	auto account = pConnection->getAccount();

	auto success = false;
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
			// NOTE: This just bumps the client to Character Select screen.
			pConnection->sendZoneUnavailable();
			sendCharacterSelection(pConnection);
		}
		// Zoning
		else {
			// TODO:
		}
	}

	return success;
}

void World::onLeaveWorld(Character* pCharacter) {
	if (!pCharacter) return;

	auto account = pCharacter->getAccount();
	
	// LinkDead Character leaving World.
	if (pCharacter->isLinkDead()) {
		mLog->info("LinkDead Character (" + pCharacter->getName() + ") leaving world.");

		// Notify AccountManager.
		mAccountManager->onDisconnect(account);
	}
	// Camping Character leaving World.
	else if (pCharacter->isCampComplete()) {
		mLog->info("Camping Character (" + pCharacter->getName() + ") leaving world.");
		account->clearActiveCharacter();
	}
	else {
		mLog->error("Character leaving world under unknown circumstances.");

		// Notify AccountManager.
		mAccountManager->onDisconnect(account);
	}

	delete pCharacter;
}

const bool World::sendCharacterSelection(WorldConnection* pConnection) {
	if (!pConnection) return false;
	if (!pConnection->getAccount()) return false;

	// Check: AccountCharacter(s) loaded successfully.
	SharedPtrList<AccountCharacter> characters;
	if (!mDataStore->accountLoadCharacters(pConnection->getAccount()->getAccountID(), characters)) {
		// TODO: Log.
		return false;
	}

	auto packet = Payload::makeCharacterSelection(characters);
	pConnection->sendPacket(packet);
	delete packet;

	return true;
}
