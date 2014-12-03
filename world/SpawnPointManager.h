#pragma once

#include "Types.h"
#include <list>

class Zone;
class SpawnGroup;
class SpawnPoint;
class NPC;

namespace Data {
	struct SpawnGroup;
	struct SpawnPoint;
}

class SpawnPointManager {
public:
	const bool initialise(Zone* pZone, std::list<Data::SpawnGroup*>& pSpawnGroupData, std::list<Data::SpawnPoint*>& pSpawnPointData);
	void update();
	void onDeath(NPC* pNPC);

	// Finds a SpawnGroup by ID.
	SpawnGroup* findSpawnGroup(const u32 pID) const;

private:

	void _spawn(SpawnPoint* pSpawnPoint);

	bool mInitialised = false;
	Zone* mZone = nullptr;

	std::list<SpawnGroup*> mSpawnGroups;
	std::list<SpawnPoint*> mActiveSpawnPoints;
	std::list<SpawnPoint*> mRespawnSpawnPoints;
};