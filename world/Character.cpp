#include "Character.h"
#include "../common/eq_packet_structs.h"
#include "../common/extprofile.h"

Character::Character() { }
Character::~Character() {
	delete mProfile;
	delete mExtendedProfile;
}

void Character::update() {
}

bool Character::initialise(PlayerProfile_Struct* pProfile, ExtendedProfile_Struct* pExtendedProfile) {
	mProfile = pProfile;
	mExtendedProfile = pExtendedProfile;

	mName = mProfile->name;
	//mID = mProfile->

	return true;
}
