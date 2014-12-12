#include "SpawnPoint.h"

void SpawnPoint::start() {
	mRespawnTimer.Enable();
	mRespawnTimer.Start(mRespawnTime);
}

const bool SpawnPoint::update() {
	if (mRespawnTimer.Check()){
		mRespawnTimer.Disable();
		return false;
	}
	return true;
}

void SpawnPoint::reset() {
	mRespawnTimer.Start(0);
}
