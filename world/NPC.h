#pragma once

#include "Actor.h"

class SpawnPoint;
class NPC : public Actor {
public:
	NPC();
	inline const bool isNPC() const { return true; }
	const bool isNPCCorpse() const { return getActorType() == AT_NPC_CORPSE; }
	const bool initialise();
	const bool onDeath();
	void onDestroy();
	const bool update();
	inline SpawnPoint* getSpawnPoint() const { return mSpawnPoint; }
	inline void setSpawnPoint(SpawnPoint* pSpawnPoint) { mSpawnPoint = pSpawnPoint; }
private:

	SpawnPoint* mSpawnPoint = nullptr;
};