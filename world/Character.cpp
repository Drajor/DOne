#include "Character.h"
#include "../common/eq_packet_structs.h"
#include "../common/extprofile.h"

Character::Character(uint32 pCharacterID) :
mCharacterID(pCharacterID),
mStanding(true),
mAFK(false),
mStatus(0),
mZone(0)
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
	mStatus = 255;

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

void Character::setAFK(bool pAFK) {
	mAFK = pAFK;
}

void Character::setShowHelm(bool pShowHelm) {
	mProfile->showhelm = pShowHelm ? 1 : 0;
}

void Character::message(uint32 pType, std::string pMessage)
{

}

void Character::setPosition(float pX, float pY, float pZ)
{
	mProfile->x = pX;
	mProfile->y = pY;
	mProfile->z = pZ;
}
