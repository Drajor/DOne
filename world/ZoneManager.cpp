#include "ZoneManager.h"
#include "Zone.h"
#include "DataStore.h"
#include "LogSystem.h"

ZoneManager::ZoneManager(DataStore* pDataStore) :mDataStore(pDataStore) { }

void ZoneManager::update() {
	for (auto& i : mZones) {
		i->update();
	}
}

void ZoneManager::clientConnect(WorldClientConnection* pClient)
{
}

uint16 ZoneManager::getZonePort(uint32 pZoneID, uint32 pInstanceID /*= 0*/) {
	// Search existing zones
	for (auto i : mZones) {
		if (i->getID() == pZoneID && i->getInstanceID() == pInstanceID) {
			return i->getPort();
		}
	}

	// Zone not currently up so lets create it.
	Log::info("[Zone Manager] : Zone Request");
	Zone* zone = new Zone(mDataStore, _getNextZonePort(), pZoneID, pInstanceID);
	zone->initialise();
	mZones.push_back(zone);

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
