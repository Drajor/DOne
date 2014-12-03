#pragma once

#include <list>

class Zone;
class SpawnPoint;
class NPC;

namespace Data {
	struct SpawnPoint;
}

class SpawnPointManager {
public:
	const bool initialise(Zone* pZone, std::list<Data::SpawnPoint*>& pSpawnPointData);
	void update();
	void onDeath(NPC* pNPC);
private:

	void _spawn(SpawnPoint* pSpawnPoint);

	bool mInitialised = false;
	Zone* mZone = nullptr;

	std::list<SpawnPoint*> mActiveSpawnPoints;
	std::list<SpawnPoint*> mRespawnSpawnPoints;
};