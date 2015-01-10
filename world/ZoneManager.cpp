#include "ZoneManager.h"
#include "ServiceLocator.h"
#include "ZoneData.h"
#include "Zone.h"
#include "World.h"
#include "Character.h"
#include "ZoneConnection.h"
#include "LogSystem.h"
#include "Utility.h"
#include "ExperienceController.h"
#include "ExperienceCalculator.h"
#include "GroupManager.h"
#include "RaidManager.h"
#include "GuildManager.h"

ZoneManager::~ZoneManager() {
	ZoneConnection::_deinitialise();
}

void ZoneManager::update() {
	for (auto i = mZones.begin(); i != mZones.end();) {
		auto zone = *i;
		if (zone->update()) {
			i++;
			continue;
		}
		Log::status("[Zone Manager] Deleting Zone." + Utility::zoneLogDetails(zone));
		mAvailableZonePorts.push_back(zone->getPort());
		delete zone;
		i = mZones.erase(i);
	}
}

const u16 ZoneManager::getZonePort(const u16 pZoneID, const u16 pInstanceID) const {
	auto zone = _search(pZoneID, pInstanceID);
	EXPECTED_VAR(zone, 0); // NOTE: If this occurs then something is happening out of order.
	return zone->getPort();
}

const bool ZoneManager::initialise(ZoneDataManager* pZoneDataManager, GroupManager* pGroupManager, RaidManager* pRaidManager, GuildManager* pGuildManager, CommandHandler* pCommandHandler, ItemFactory* pItemFactory) {
	EXPECTED_BOOL(mInitialised == false);
	EXPECTED_BOOL(pZoneDataManager);
	EXPECTED_BOOL(pGroupManager);
	EXPECTED_BOOL(pRaidManager);
	EXPECTED_BOOL(pGuildManager);
	EXPECTED_BOOL(pCommandHandler);
	EXPECTED_BOOL(pItemFactory);

	mZoneDataManager = pZoneDataManager;
	mGroupManager = pGroupManager;
	mRaidManager = pRaidManager;
	mGuildManager = pGuildManager;
	mCommandHandler = pCommandHandler;
	mItemFactory = pItemFactory;

	Log::status("[Zone Manager] Initialising.");

	for (int i = 0; i < 3000; i++)
		mAvailableZonePorts.push_back(7000+i);
	ZoneConnection::_initalise();
	Experience::Controller::_initialise();

	mExperienceCalculator = new Experience::Calculator();

	mInitialised = true;
	return true;
}

const bool ZoneManager::isZoneRunning(const u16 pZoneID, const u16 pInstanceID) const {
	return _search(pZoneID, pInstanceID) != nullptr;
}

const bool ZoneManager::isZoneAvailable(const u16 pZoneID, const u16 pInstanceID) {
	auto zone = _search(pZoneID, pInstanceID);
	if (zone) {
		// Where Zone is shutting down, do not allow entry.
		return !zone->isShuttingDown();
	}

	return _makeZone(pZoneID, pInstanceID);
}

const bool ZoneManager::canZoneShutdown(const u16 pZoneID, const u16 pInstanceID) const {
	// Check: Zone exists.
	auto zone = _search(pZoneID, pInstanceID);
	if (!zone) return false;

	return canZoneShutdown(zone);
}

const bool ZoneManager::canZoneShutdown(Zone* pZone) const {
	EXPECTED_BOOL(pZone);

	// Check: Are any Characters currently zoning into Zone.
	for (auto i : mZoningCharacters) {
		auto zoneChange = i->getZoneChange();
		if (zoneChange.mZoneID == pZone->getID() && zoneChange.mInstanceID == pZone->getInstanceID())
			return false;
	}

	// Finally check with Zone.
	return pZone->canShutdown();
}

const bool ZoneManager::requestZoneBoot(const u16 pZoneID, const u16 pInstanceID) {
	// Check: Zone already running.
	if (isZoneRunning(pZoneID, pInstanceID)) return false;

	return _makeZone(pZoneID, pInstanceID);
}

const bool ZoneManager::requestZoneShutdown(const u16 pZoneID, const u16 pInstanceID) {
	// Check: Zone exists.
	auto zone = _search(pZoneID, pInstanceID);
	if (!zone) return false;

	if (!canZoneShutdown(zone)) return false;

	return zone->shutdown();
}

const u16 ZoneManager::_getNextZonePort() {
	EXPECTED_VAR(mAvailableZonePorts.empty() == false, 0);

	auto port = *mAvailableZonePorts.begin();
	mAvailableZonePorts.pop_front();
	return port;
}

const bool ZoneManager::_makeZone(const u16 pZoneID, const u16 pInstanceID) {
	auto port = _getNextZonePort();
	EXPECTED_BOOL(port != 0);

	StringStream ss; ss << "[Zone Manager] Starting new Zone on port " << port << ", ZoneID: " << pZoneID << " InstanceID: " << pInstanceID;
	Log::info(ss.str());
	
	
	// Check: Zone initialises correctly.
	auto zoneData = mZoneDataManager->getZoneData(pZoneID);
	EXPECTED_BOOL(zoneData);

	auto zone = new Zone(port, pZoneID, pInstanceID);
	if (!zone->initialise(zoneData, mExperienceCalculator, mGroupManager, mRaidManager, mGuildManager, mCommandHandler, mItemFactory)) {
		// Restore port to available list.
		mAvailableZonePorts.push_front(port);
		delete zone;
		return false;
	}
	
	mZones.push_back(zone);
	return true;
}

void ZoneManager::handleWhoRequest(Character* pCharacter, const WhoFilter& pFilter, std::list<Character*>& pResults) {
	for (auto i : mZones)
		i->getWhoMatches(pResults, pFilter);
}

void ZoneManager::handleTell(Character* pCharacter, const String& pTargetName, const String& pMessage) {
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
			i->addQueuedMessage(ChannelID::Tell, pCharacter->getName(), pMessage);
		}
	}

	// pTargetName is not online at this time.
	pCharacter->getConnection()->sendSimpleMessage(MessageType::White, StringID::PLAYER_NOT_ONLINE, pTargetName);
}

Character* ZoneManager::findCharacter(const String pCharacterName, bool pIncludeZoning, Zone* pExcludeZone) const {
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

void ZoneManager::addZoningCharacter(Character* pCharacter) {
	EXPECTED(pCharacter);

	mZoningCharacters.push_back(pCharacter);
}

const bool ZoneManager::removeZoningCharacter(const String& pCharacterName) {
	for (auto i : mZoningCharacters) {
		if (i->getName() == pCharacterName) {
			mZoningCharacters.remove(i);
			return true;
		}
	}

	// NOTE: This should only occur if there is a bug.
	Log::error("[Zone Manager] removeZoningCharacter failed for " + pCharacterName);
	return false;
}

const bool ZoneManager::hasZoningCharacter(const u32 pAccountID) const {
	for (auto i : mZoningCharacters) {
		if (i->getAccountID() == pAccountID) {
			return true;
		}
	}
	return false;
}

String ZoneManager::getZoningCharacterName(const u32 pAccountID) {
	for (auto i : mZoningCharacters) {
		if (i->getAccountID() == pAccountID) {
			return i->getName();
		}
	}

	return "";
}

Character* ZoneManager::getZoningCharacter(const String& pCharacterName) {
	for (auto i : mZoningCharacters) {
		if (i->getName() == pCharacterName)
			return i;
	}

	return nullptr;
}

ZoneSearchResult ZoneManager::getAllZones() {
	ZoneSearchResult result;
	for (auto i : mZones) {
		ZoneSearchEntry entry;
		entry.mName = i->getLongName();
		entry.mID = i->getID();
		entry.mInstanceID = i->getInstanceID();
		entry.mNumCharacters = i->getNumCharacters();
		result.push_back(entry);
	}

	return result;
}

Zone* ZoneManager::_search(const u16 pZoneID, const u16 pInstanceID) const {
	for (auto i : mZones) {
		if (i->getID() == pZoneID && i->getInstanceID() == pInstanceID)
			return i;
	}

	return nullptr;
}
