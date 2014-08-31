#pragma once

#include "NPC.h"

class SpawnPoint {
public:
	inline const Vector3& getPosition() { return mPosition; }
	inline const bool hasNPC() const { return mNPC != nullptr; }
	inline const NPC* getNPC() const { return mNPC; }
	inline void setNPC(NPC* pNPC) { mNPC = pNPC; }
	inline void setRespawnTime(const uint32 pRespawnTime) { mRespawnTime = pRespawnTime; }
private:
	uint32 mRespawnTime = DEFAULT_RESPAWN_TIME;
	NPC* mNPC = nullptr;
	Vector3 mPosition;
};