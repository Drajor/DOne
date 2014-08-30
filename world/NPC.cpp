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

	setName("Bob");
	setRaceID(6);
	setClass(ClassIDs::Cleric);

	return true;
}

