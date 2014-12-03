#pragma once

#include "Constants.h"
#include "Vector3.h"
#include "../common/timer.h"

class NPC;
class SpawnPoint {
public:

	void start();
	const bool update();

	inline void setPosition(const Vector3& pPosition) { mPosition = pPosition; }
	inline const Vector3& getPosition() { return mPosition; }
	inline void setHeading(const float pHeading) { mHeading = pHeading; }
	inline const float getHeading() const { return mHeading; }
	inline const bool hasNPC() const { return mNPC != nullptr; }
	inline NPC* getNPC() const { return mNPC; }
	inline void setNPC(NPC* pNPC) { mNPC = pNPC; }
	inline void setRespawnTime(const uint32 pRespawnTime) { mRespawnTime = pRespawnTime; }
	inline void setSpawnGroup(uint32 pSpawnGroupID) { mSpawnGroupID = pSpawnGroupID; }
	inline const uint32 getSpawnGroup() const { return mSpawnGroupID; }
private:
	uint32 mNPCType = 0;
	uint32 mSpawnGroupID = 0;
	uint32 mRespawnTime = DEFAULT_RESPAWN_TIME;
	NPC* mNPC = nullptr;
	Vector3 mPosition;
	float mHeading = 0.0f;
	Timer mRespawnTimer;
};