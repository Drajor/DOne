#include "ZoneManager.h"
#include "Zone.h"
#include "GroupManager.h"
#include "GuildManager.h"
#include "RaidManager.h"
#include "World.h"
#include "DataStore.h"
#include "Character.h"
#include "ZoneClientConnection.h"
#include "LogSystem.h"
#include "Utility.h"

ZoneManager::ZoneManager(World* pWorld, DataStore* pDataStore) :
mWorld(pWorld),
mDataStore(pDataStore),
mGroupManager(nullptr),
mGuildManager(nullptr),
mRaidManager(nullptr)
{ }

ZoneManager::~ZoneManager() {
	ZoneClientConnection::deinitialise();
	safe_delete(mGroupManager);
}


void ZoneManager::update() {
	for (auto& i : mZones) {
		i->update();
	}
}

uint16 ZoneManager::getZonePort(uint32 pZoneID, uint32 pInstanceID) {
	// Search existing zones
	for (auto i : mZones) {
		if (i->getID() == pZoneID && i->getInstanceID() == pInstanceID) {
			return i->getPort();
		}
	}

	// Zone was not found, create it.
	Zone* zone = _makeZone(pZoneID, pInstanceID);
	return zone->getPort();
}

void ZoneManager::initialise() {
	for (int i = 0; i < 200; i++)
		mAvailableZonePorts.push_back(7000+i);
	ZoneClientConnection::initalise();
	mGroupManager = new GroupManager();
	mGuildManager = new GuildManager();
	mRaidManager = new RaidManager();
}

uint32 ZoneManager::_getNextZonePort() {
	uint32 port = *mAvailableZonePorts.begin();
	mAvailableZonePorts.pop_front();
	return port;
	// TODO: Error check this ;)
}

void ZoneManager::addAuthentication(ClientAuthentication& pAuthentication, std::string pCharacterName, uint32 pZoneID, uint32 pInstanceID) {
	bool zoneFound = false;
	for (auto i : mZones) {
		if (i->getID() == pZoneID && i->getInstanceID() == pInstanceID) {
			i->addAuthentication(pAuthentication, pCharacterName);
			return;
		}
	}

	// Zone was not found, create it.
	Zone* zone = _makeZone(pZoneID, pInstanceID);
	zone->addAuthentication(pAuthentication, pCharacterName);
}

Zone* ZoneManager::_makeZone(uint32 pZoneID, uint32 pInstanceID) {
	Log::info("[Zone Manager] : Zone Request");
	Zone* zone = new Zone(mWorld, this, mGroupManager, mGuildManager, mRaidManager, mDataStore, _getNextZonePort(), pZoneID, pInstanceID);
	zone->initialise();
	mZones.push_back(zone);
	return zone;
}

void ZoneManager::whoAllRequest(Character* pCharacter, WhoFilter& pFilter) {
	std::list<Character*> matches;
	for (auto i : mZones) {
		i->getWhoMatches(matches, pFilter);
	}
	pCharacter->getConnection()->sendWhoResults(matches);
}

void ZoneManager::notifyCharacterChatTell(Character* pCharacter, const std::string& pTargetName, const std::string& pMessage) {
	// Search Zones
	for (auto i : mZones) {
		if (i->trySendTell(pCharacter->getName(), pTargetName, pMessage)) {
			// Send echo "You told Player, 'Message'
			pCharacter->getConnection()->sendSimpleMessage(MessageType::TellEcho, StringID::TELL_ECHO, pTargetName, pMessage);
			return;
		}
	}

	// Check zoning Characters
	for (auto i : mZoningCharacters) {
		if (i->getName() == pTargetName) {
			// Send queued echo "You told Player, '[queued] Message'
			pCharacter->getConnection()->sendSimpleMessage(MessageType::TellEcho, StringID::TELL_QUEUED, pTargetName, Utility::StringIDString(StringID::QUEUED), pMessage);
			// Store queued message.
			i->addQueuedMessage(ChannelID::CH_TELL, pCharacter->getName(), pMessage);
		}
	}

	// pTargetName is not online at this time.
	pCharacter->getConnection()->sendSimpleMessage(MessageType::White, StringID::PLAYER_NOT_ONLINE, pTargetName);
}

Character* ZoneManager::findCharacter(const std::string pCharacterName, bool pIncludeZoning, Zone* pExcludeZone) {
	Character* character = nullptr;

	// Search Zones.
	for (auto i : mZones) {
		if (i != pExcludeZone) {
			character = i->findCharacter(pCharacterName);
			if (character)
				return character;
		}
	}

	// Search zoning characters.
	if (pIncludeZoning) {
		for (auto i : mZoningCharacters) {
			if (i->getName() == pCharacterName)
				return i;
		}
	}

	return nullptr;
}

void ZoneManager::notifyCharacterZoneOut(Character* pCharacter) {
	mZoningCharacters.push_back(pCharacter);
}
