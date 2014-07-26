#pragma once

#include "ClientAuthentication.h"
#include <list>
#include "../common/types.h"

class WorldClientConnection;
class Zone;
class DataStore;
class World;
class Character;
struct WhoFilter;

class ZoneManager {
public:
	ZoneManager(World* pWorld, DataStore* pDataStore);
	void addAuthentication(ClientAuthentication& pAuthentication, std::string pCharacterName, uint32 pZoneID, uint32 pInstanceID = 0);
	void initialise();
	void update();
	uint16 getZonePort(uint32 pZoneID, uint32 pInstanceID = 0);
	void whoAllRequest(Character* pCharacter, WhoFilter& pFilter);
private:
	Zone* _makeZone(uint32 pZoneID, uint32 pInstanceID = 0);
	uint32 _getNextZonePort();
	

	World* mWorld;
	DataStore* mDataStore;
	std::list<uint32> mAvailableZonePorts;
	std::list<Zone*> mZones;
};