#pragma once

#include <list>

class Client;
class Zone;
class DataStore;

class ZoneManager {
public:
	ZoneManager(DataStore* pDataStore);
	// Called when the Client clicks 'Enter World' at the Character Select screen.
	void clientConnect(Client* pClient);
	void update();
private:
	DataStore* mDataStore;
	std::list<Zone*> mZones;
};