#pragma once

#include "Types.h"
#include <list>

class NPCFactory;
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

	const bool populate();
	const bool depopulate();

	void onDeath(NPC* pNPC);

	// Finds a SpawnGroup by ID.
	SpawnGroup* findSpawnGroup(const u32 pID) const;

private:

	bool mInitialised = false;
	NPCFactory* mNPCFactory = nullptr;

	void _spawn(SpawnPoint* pSpawnPoint);
	void _associate(SpawnPoint* pSpawnPoint, NPC* pNPC);
	void _disassociate(SpawnPoint* pSpawnPoint, NPC* pNPC);
	inline void _addRespawn(SpawnPoint* pSpawnPoint) { mRespawnSpawnPoints.push_back(pSpawnPoint); }
	bool mPopulated = false;
	Zone* mZone = nullptr;

	std::list<SpawnGroup*> mSpawnGroups;
	std::list<SpawnPoint*> mActiveSpawnPoints;
	std::list<SpawnPoint*> mRespawnSpawnPoints;
};