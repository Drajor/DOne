#include "RaidManager.h"
#include "Character.h"
#include "Zone.h"
#include "Utility.h"

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
