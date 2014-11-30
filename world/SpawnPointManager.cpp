#include "SpawnPointManager.h"
#include "SpawnPoint.h"
#include "Data.h"
#include "Zone.h"
#include "NPC.h"
#include "NPCFactory.h"
#include "LogSystem.h"

const bool SpawnPointManager::initialise(Zone* pZone, std::list<SpawnPointData*>& pSpawnPointData) {
	if (mInitialised) { return false; }
	if (!pZone) { return false; }

	Log::status("[SpawnPointManager] Initialising.");

	mZone = pZone;

	// Create SpawnPoints.
	for (auto i : pSpawnPointData) {
		auto spawnPoint = new SpawnPoint();
		spawnPoint->start(); // Start the respawn timer.
		mRespawnSpawnPoints.push_back(spawnPoint);

		spawnPoint->setPosition(i->mPosition);
		spawnPoint->setHeading(i->mHeading);
		spawnPoint->setRespawnTime(i->mRespawnTime);
		spawnPoint->setType(i->mType);
		spawnPoint->setNPCType(i->mNPCType);
		spawnPoint->setSpawnGroup(i->mSpawnGroupID);
	}

	Log::info("[SpawnPointManager] Finished initialising. " + std::to_string(pSpawnPointData.size()) + " Spawn Points.");
	mInitialised = true;
	return true;
}

void SpawnPointManager::update() {
	// Check each despawned SpawnPoint 
	for (auto i = mRespawnSpawnPoints.begin(); i != mRespawnSpawnPoints.end();) {
		SpawnPoint* spawnPoint = *i;
		if (spawnPoint->update()) {
			i++;
			continue;
		}
		i = mRespawnSpawnPoints.erase(i);
		mActiveSpawnPoints.push_back(spawnPoint);
		_spawn(spawnPoint);
	}
}

void SpawnPointManager::onDeath(NPC* pNPC) {
	if (!pNPC) return;
	auto spawnPoint = pNPC->getSpawnPoint();
	if (!spawnPoint) return;
	
	// Disassociate NPC and SpawnPoint.
	pNPC->setSpawnPoint(nullptr);
	spawnPoint->setNPC(nullptr);

	// Add to respawn list.
	mRespawnSpawnPoints.push_back(spawnPoint);
}

void SpawnPointManager::_spawn(SpawnPoint* pSpawnPoint) {
	
	auto npc = NPCFactory::getInstance().create(1);
	npc->setZone(mZone);
	npc->initialise();
	npc->setPosition(pSpawnPoint->getPosition());
	npc->setHeading(pSpawnPoint->getHeading());

	// Associate NPC and SpawnPoint.
	pSpawnPoint->setNPC(npc);
	npc->setSpawnPoint(pSpawnPoint);

	// Add NPC to Zone.
	mZone->addActor(npc);
}
