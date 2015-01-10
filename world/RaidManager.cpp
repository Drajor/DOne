#include "RaidManager.h"
#include "Character.h"
#include "Zone.h"
#include "Utility.h"

const bool RaidManager::initialise(ZoneManager* pZoneManager) {
	EXPECTED_BOOL(mInitialised == false);
	EXPECTED_BOOL(pZoneManager);

	mZoneManager = pZoneManager;
	mInitialised = true;
	return true;
}

void RaidManager::onEnterZone(Character* pCharacter) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->hasRaid());
}

void RaidManager::onLeaveZone(Character* pCharacter) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->hasRaid());
}


void RaidManager::onCamp(Character* pCharacter) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->hasRaid());
}

void RaidManager::onLinkdead(Character* pCharacter) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->hasRaid());
}
