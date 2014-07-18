#pragma once

#include <list>
#include "../common/types.h"

class WorldClientConnection;
class Zone;
class DataStore;

class ZoneManager {
public:
	ZoneManager(DataStore* pDataStore);
	void initialise();
	// Called when the Client clicks 'Enter World' at the Character Select screen.
	void clientConnect(WorldClientConnection* pClient);
	void update();
	uint16 getZonePort(uint32 pZoneID, uint32 pInstanceID = 0);
private:
	uint32 _getNextZonePort();
	DataStore* mDataStore;
	std::list<uint32> mAvailableZonePorts;
	std::list<Zone*> mZones;
};