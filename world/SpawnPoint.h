#pragma once

#include "Types.h"
#include "Vector3.h"
#include "../common/timer.h"

class NPC;
class SpawnGroup;

class SpawnPoint {
public:

	void start();
	const bool update();

	// Resetting will cause the respawn timer to be zero.
	void reset();

	// Sets the position of the SpawnPoint.
	inline void setPosition(const Vector3& pPosition) { mPosition = pPosition; }

	// Returns the position of the SpawnPoint.
	inline const Vector3& getPosition() { return mPosition; }

	// Sets the heading of the SpawnPoint.
	inline void setHeading(const float pHeading) { mHeading = pHeading; }

	// Returns the heading of the SpawnPoint.
	inline const float getHeading() const { return mHeading; }

	// Returns whether this SpawnPoint has an NPC assigned to it or not.
	inline const bool hasNPC() const { return mNPC != nullptr; }

	// Returns the NPC assigned to this SpawnPoint.
	inline NPC* getNPC() const { return mNPC; }

	// Sets the NPC assigned to this SpawnPoint.
	inline void setNPC(NPC* pNPC) { mNPC = pNPC; }

	// Sets the respawn time of this SpawnPoint.
	inline void setRespawnTime(const u32 pRespawnTime) { mRespawnTime = pRespawnTime; }

	// Sets the SpawnGroup assigned to this SpawnPoint.
	inline void setSpawnGroup(SpawnGroup* pSpawnGroup) { mSpawnGroupID = pSpawnGroup; }

	// Returns the SpawnGroup assigned to this SpawnPoint.
	inline SpawnGroup* getSpawnGroup() const { return mSpawnGroupID; }

private:

	SpawnGroup* mSpawnGroupID = nullptr;
	u32 mRespawnTime = 200;
	NPC* mNPC = nullptr;
	Vector3 mPosition;
	float mHeading = 0.0f;
	Timer mRespawnTimer;
};