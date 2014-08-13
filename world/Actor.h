#pragma once

class Zone;

#include "Constants.h"

class Actor {
public:
	Actor() : mSpawnID(0), mTarget(nullptr) {};
	virtual ~Actor() {};

	inline const SpawnID getSpawnID() { return mSpawnID; }
	inline void setSpawnID(SpawnID pSpawnID) { mSpawnID = pSpawnID; }
	inline virtual bool isCharacter() { return false; }
	inline virtual bool isNPC() { return false; }
	inline void setTarget(Actor* pActor) { mTarget = pActor; }
	inline Actor* getTarget() { return mTarget; }
	inline bool hasTarget() { return mTarget != nullptr; }
protected:
	SpawnID mSpawnID;
	Actor* mTarget;
private:
};