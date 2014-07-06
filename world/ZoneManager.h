#pragma once

#include <list>

class Client;
class Zone;
class ZoneManager {
public:
	// Called when the Client clicks 'Enter World' at the Character Select screen.
	void clientConnect(Client* pClient);
	void update();
private:
	std::list<Zone*> mZones;
};