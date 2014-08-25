#pragma once

class Zone;

#include "Constants.h"
#include "Vector3.h"

class Actor {
public:
	Actor() : mSpawnID(0), mTarget(nullptr) {};
	virtual ~Actor() {};

	inline const SpawnID getSpawnID() const { return mSpawnID; }
	inline void setSpawnID(const SpawnID pSpawnID) { mSpawnID = pSpawnID; }
	inline virtual const bool isCharacter() const { return false; }
	inline virtual bool isNPC() const { return false; }
	inline void setTarget(Actor* pActor) { mTarget = pActor; }
	inline Actor* getTarget() const { return mTarget; }
	inline const bool hasTarget() const { return mTarget != nullptr; }
	template <typename T>
	inline static T cast(Actor* pActor) {
		return static_cast<T>(pActor);
	}
	inline const Vector3& getPosition() const { return mPosition; }
	inline const float distanceTo(const Actor* pActor) {
		return mPosition.distance(pActor->mPosition);
	}
	inline const float squareDistanceTo(const Actor* pActor) {
		return mPosition.squareDistance(pActor->mPosition);
	}
protected:
	SpawnID mSpawnID;
	Vector3 mPosition;
	Actor* mTarget;
private:
};