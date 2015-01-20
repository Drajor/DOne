#include "ZoneConnectionManager.h"
#include "ZoneConnection.h"
#include "LogSystem.h"
#include "Zone.h"
#include "Character.h"

#include "../common/EQStreamFactory.h"
#include "../common/EQStreamIdent.h"
#include "../common/patches/Underfoot.h"

ZoneConnectionManager::~ZoneConnectionManager() {
	if (mStreamFactory) {
		mStreamFactory->Close();
		mStreamFactory = nullptr;
		// NOTE: mStreamFactory is intentionally not deleted here.
	}

	if (mStreamIdentifier) {
		delete mStreamIdentifier;
		mStreamIdentifier = nullptr;
	}
}

const bool ZoneConnectionManager::initialise(const u16 pPort, Zone* pZone, ILogFactory* pLogFactory, GuildManager* pGuildManager) {
	if (mInitialised) return false;
	if (!pZone) return false;
	if (!pLogFactory) return false;
	if (!pGuildManager) return false;

	mPort = pPort;
	mZone = pZone;
	mLogFactory = pLogFactory;
	mGuildManager = pGuildManager;

	// Create and configure ZoneConnectionManager log.
	mLog = mLogFactory->make();
	mLog->setContext("[ZoneConnectionManager]");
	mLog->status("Initialising");
	
	// Create and initialise EQStreamFactory.
	mStreamFactory = new EQStreamFactory(ZoneStream);
	if (!mStreamFactory->Open(mPort)) {
		mLog->error("Failed to start EQStreamFactory.");
		return false;
	}

	mStreamIdentifier = new EQStreamIdentifier;
	Underfoot::Register(*mStreamIdentifier);

	mLog->status("Finished initialising.");
	mInitialised = true;
	return true;
}

void ZoneConnectionManager::update() {
	checkIncomingConnections();
	updatePreConnections();
	updateActiveConnections();
	checkLinkdeadConnections();
}

void ZoneConnectionManager::checkIncomingConnections() {
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
		auto connection = new ZoneConnection();
		if (!connection->initialise(incomingStreamInterface, mLogFactory->make(), mZone, mGuildManager)) {
			mLog->error("ZoneConnection::initialise failed.");
			delete connection;
			continue;
		}

		mLog->info("Added new ZoneConnection.");
		mPreConnections.push_back(connection);
	}
}

void ZoneConnectionManager::updatePreConnections() {
	for (auto i = mPreConnections.begin(); i != mPreConnections.end();) {
		auto connection = *i;
		const bool isConnected = connection->isConnected();
		// Connection is fine, proceed as normal.
		if (isConnected) {
			connection->update();

			// Check: Is the connection complete?
			if (connection->isConnectComplete()) {
				i = mPreConnections.erase(i);
				mActiveConnections.push_back(connection);

				auto character = connection->getCharacter();
				mZone->onEnterZone(character);

				continue;
			}
			i++;
			continue;
		}
		// Connection has been lost.
		else {
			Log::info("[Zone] Connection lost while zoning in.");
			// Disconnect while zoning or logging in.
			Character* character = connection->getCharacter();
			if (character) {

				// TODO: What do I do here?!

				//if (character->hasGroup())
				//	mGroupManager->onCamp(character);

				//if (character->hasRaid())
				//	mRaidManager->onCamp(character);

				//if (character->hasGuild())
				//	mGuildManager->onCamp(character);

				//mAccountManager->onCamp(character);
				//delete character;
			}

			delete connection;
			i = mPreConnections.erase(i);
		}
	}
}

void ZoneConnectionManager::updateActiveConnections() {
	// Update our connections.
	for (auto i = mActiveConnections.begin(); i != mActiveConnections.end();) {
		auto connection = *i;
		// Connection is fine, proceed as normal.
		if (connection->isConnected()) {
			connection->update();
			i++;
			continue;
		}

		// Connection has been lost.
		auto character = connection->getCharacter();

		// Check: Character camped out.
		if (character->isCampComplete()) {

			// Notify Zone.
			mZone->onCampComplete(character);

			// Clean up.
			i = mActiveConnections.erase(i);
			delete connection;

			continue;
		}

		// Expected: Player zoning out.
		if (character->isZoningOut()) {

			// Notify Zone.
			mZone->onLeaveZone(character);

			// Clean up.
			i = mActiveConnections.erase(i);
			delete connection;

			continue;
		}

		// Unexpected: Link Dead.

		// Notify Zone.
		mZone->onLinkdeadBegin(character);

		// Move connection into LD list.
		i = mActiveConnections.erase(i);
		mLinkDeadConnections.insert(std::make_pair(Utility::Time::now(), connection));
	}
}

void ZoneConnectionManager::checkLinkdeadConnections() {
	static u32 LDTime = 10;
	auto timeNow = Utility::Time::now();

	for (auto i = mLinkDeadConnections.begin(); i != mLinkDeadConnections.end();) {
		// Check: LD waiting time has expired.
		if (i->first + LDTime >= timeNow) {

			auto connection = i->second;

			// Notify Zone.
			mZone->onLinkdeadEnd(connection->getCharacter());

			// Clean up.
			i = mLinkDeadConnections.erase(i);
			delete connection;

			continue;
		}

		i++;
	}
}
