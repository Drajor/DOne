#include "LootController.h"
#include "Utility.h"
#include "Character.h"
#include "HateController.h"

const bool LootController::canLoot(Character* pLooter) const {
	EXPECTED_BOOL(pLooter);
	
	// Check: Already open.
	if (isOpen()) return true;

	if (mCharacter == pLooter) return true;
	if (mCharacter->getGroup() == mGroup) return true;
	if (mCharacter->getRaid() == mRaid) return true; // TODO: This will have to change with raid loot rules.

	return false;
}
