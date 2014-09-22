#include "NPC.h"
#include "Zone.h"
#include "Utility.h"

NPC::NPC() {
	setRunSpeed(0.7f);
	setWalkSpeed(0.35f);
	setBodyType(BT_Humanoid);
	setActorType(AT_NPC);
	setIsNPC(true);
}

const bool NPC::initialise() {
	EXPECTED_BOOL(mZone);
	EXPECTED_BOOL(getSpawnID() == 0);
	setSpawnID(mZone->getNextSpawnID());

	String baseName = "Bob";
	setName(baseName+std::to_string(getSpawnID()));
	setRaceID(6);
	setClass(ClassIDs::Cleric);
	//setIsGM(true);

	return true;
}

const bool NPC::onDeath() {
	setActorType(AT_NPC_CORPSE);
	setName(getName() + "'s corpse");

	return true;
}

