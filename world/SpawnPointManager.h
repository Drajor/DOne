#pragma once

#include <list>

class Zone;
class SpawnPoint;
class NPC;
struct SpawnPointData;

class SpawnPointManager {
public:
	const bool initialise(Zone* pZone, std::list<SpawnPointData*>& pSpawnPointData);
	void update();
	void onDeath(NPC* pNPC);
private:

	void _spawn(SpawnPoint* pSpawnPoint);

	bool mInitialised = false;
	Zone* mZone = nullptr;

	std::list<SpawnPoint*> mActiveSpawnPoints;
	std::list<SpawnPoint*> mRespawnSpawnPoints;
};