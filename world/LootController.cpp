#include "LootController.h"
#include "Utility.h"

void LootController::addLooter(Character* pLooter) {
	EXPECTED(pLooter);
	EXPECTED(isLooter(pLooter) == false); // Prevent double add.
	
	mLooters.push_back(pLooter);
}

void LootController::removeLooter(Character* pLooter) {
	EXPECTED(pLooter);
	EXPECTED(isLooter(pLooter));

	mLooters.remove(pLooter);
}

void LootController::clear() {
	mLooters.clear();
}

const bool LootController::isLooter(Character* pCharacter) const {
	for (auto i : mLooters) {
		if (i == pCharacter)
			return true;
	}
	return false;
}

void LootController::configure(DefenderCombatData* pData) {

}

const bool LootController::canLoot(Character* pLooter) const {
	EXPECTED_BOOL(pLooter);
	
	// Check: Already open.
	if (isOpen()) return true;

	// Otherwise, must be on the list.
	return isLooter(pLooter);
}
