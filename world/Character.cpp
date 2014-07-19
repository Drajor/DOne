#include "Character.h"
#include "../common/eq_packet_structs.h"
#include "../common/extprofile.h"

Character::Character() :
mAnimation(0),
mStanding(true)
{ }
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

	return true;
}

void Character::setStanding(bool pStanding) {
	mStanding = pStanding;
	if (mStanding)
		mCampTimer.Disable();
}

void Character::startCamp() {
	mCampTimer.Start(29000, true);
}
