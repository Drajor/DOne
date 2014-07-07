#pragma once

#include <list>

class WorldClientConnection;
class Zone;
class DataStore;

class ZoneManager {
public:
	ZoneManager(DataStore* pDataStore);
	// Called when the Client clicks 'Enter World' at the Character Select screen.
	void clientConnect(WorldClientConnection* pClient);
	void update();
private:
	DataStore* mDataStore;
	std::list<Zone*> mZones;
};