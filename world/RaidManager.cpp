#include "RaidManager.h"
#include "Character.h"
#include "Zone.h"
#include "Utility.h"

void RaidManager::onEnterZone(Character* pCharacter) {
	ARG_PTR_CHECK(pCharacter);
	EXPECTED(pCharacter->hasRaid());
}

void RaidManager::onLeaveZone(Character* pCharacter) {
	ARG_PTR_CHECK(pCharacter);
	EXPECTED(pCharacter->hasRaid());
}


void RaidManager::onCamp(Character* pCharacter) {
	ARG_PTR_CHECK(pCharacter);
	EXPECTED(pCharacter->hasRaid());
}

void RaidManager::onLinkdead(Character* pCharacter) {
	ARG_PTR_CHECK(pCharacter);
	EXPECTED(pCharacter->hasRaid());
}
