#include "NPC.h"
#include "Character.h"
#include "Zone.h"
#include "Utility.h"

NPC::NPC() {
	setRunSpeed(0.7f);
	setWalkSpeed(0.35f);
	setBodyType(BT_Humanoid);
	setActorType(AT_NPC);
	//setIsNPC(true);

	setCurrency(4, 5, 6, 7);
}

const bool NPC::initialise() {
	EXPECTED_BOOL(mZone);
	EXPECTED_BOOL(getSpawnID() == 0);
	setSpawnID(mZone->getNextSpawnID());

	//String baseName = "Bob";
	//setName(baseName+std::to_string(getSpawnID()));
	//setRaceID(6);
	//setClass(ClassIDs::Cleric);
	//setClass(40);

	return true;
}

const bool NPC::onDeath() {
	setHPPercent(0);
	setActorType(AT_NPC_CORPSE);
	setName(getName() + "'s corpse");
	mDecayTimer.Start(DEFAULT_CORPSE_ROT_TIME * 1000);
	return true;
}

const bool NPC::update() {
	// Check: Is NPC flagged for destruction?
	if (getDestroy()) return false;

	// Check: Is NPC dead?
	if (isCorpse()) {
		// Corpse is about to decay, clean up if someone is looting.
		if (mDecayTimer.Check()) {
			return false;
		}
		return true;
	}
	return true;
}

void NPC::onDestroy() {
	// Remove target references.
	clearTargeters();

	// Remove looting reference.
	Character* looter = getLooter();
	if (looter) {
		looter->setLootingCorpse(nullptr);
		setLooter(nullptr);
	}
}

void NPC::onLootBegin() {
	// Remove any null items from mLootItems
	// These occur from previous looting.
	mLootItems.erase(std::remove(begin(mLootItems), end(mLootItems), nullptr), end(mLootItems));
}
