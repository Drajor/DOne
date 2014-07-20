#include "ZoneManager.h"
#include "Zone.h"
#include "World.h"
#include "DataStore.h"
#include "LogSystem.h"

ZoneManager::ZoneManager(World* pWorld, DataStore* pDataStore) :
mWorld(pWorld),
mDataStore(pDataStore)
{ }

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
	for (int i = 7000; i < 200; i++)
		mAvailableZonePorts.push_back(i);
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
	Zone* zone = new Zone(mWorld, mDataStore, _getNextZonePort(), pZoneID, pInstanceID);
	zone->initialise();
	mZones.push_back(zone);
	return zone;
}
