#include "LootController.h"
#include "CombatData.h"
#include "Utility.h"
#include "Character.h"

void LootController::configure(DefenderCombatData* pData) {
	EXPECTED(pData);

	auto attackers = pData->getAttackers();

	// TEMP 
	for (auto i : attackers) {
		auto actor = i.first;
		if (actor->isCharacter()) {
			addLooter(Actor::cast<Character*>(actor));
		}
	}

	// TODO: Group / Raid / Individual + pets.
}

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

const bool LootController::canLoot(Character* pLooter) const {
	EXPECTED_BOOL(pLooter);
	
	// Check: Already open.
	if (isOpen()) return true;

	// Otherwise, must be on the list.
	return isLooter(pLooter);
}
