#include "Character.h"
#include "../common/eq_packet_structs.h"
#include "../common/extprofile.h"
#include "ZoneClientConnection.h"

Character::Character(uint32 pCharacterID) :
mCharacterID(pCharacterID),
mProfile(0),
mExtendedProfile(0),
mName("soandso"),
mLastName(""),
mRace(0),
mRunSpeed(1.0f),
mWalkSpeed(0.5f),
mClass(0),
mGender(0),
mLevel(1),
mDeity(394), // Agnostic
mSize(5.0f),
mStanding(true),
mAFK(false),
mLoggedOut(false),
mTGB(false),
mStatus(0),
mZone(0),
mCurrentHP(100),
mMaximumHP(100),
mCurrentMana(100),
mMaximumMana(100),
mCurrentEndurance(100),
mMaximumEndurance(100)
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
	mRace = mProfile->race;
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
	mConnection->sendMessage(pType, pMessage);
}

void Character::setPosition(float pX, float pY, float pZ, float pHeading) {
	mX = pX;
	mY = pY;
	mZ = pZ;
	mHeading = pHeading;
	_updateProfilePosition();
}

void Character::setPosition(Vector3& pPosition) {
	mX = pPosition.x;
	mY = pPosition.y;
	mZ = pPosition.z;
	_updateProfilePosition();
}

void Character::setPosition(Vector4& pPosition) {
	mX = pPosition.x;
	mY = pPosition.y;
	mZ = pPosition.z;
	mHeading = pPosition.h;
	_updateProfilePosition();
}

void Character::setHeading(float pHeading) {
	mHeading = pHeading;
	_updateProfilePosition();
}

void Character::_updateProfilePosition() {
	mProfile->x = mX;
	mProfile->y = mY;
	mProfile->z = mZ;
	mProfile->heading = mHeading;
}

void Character::healPercentage(int pPercent) {
	mConnection->sendHPUpdate();
}

void Character::damage(uint32 pAmount) {
	mCurrentHP -= pAmount;
	mConnection->sendHPUpdate();
}

void Character::setAnonymous(uint8 pAnonymous) {
	mProfile->mAnonymous = pAnonymous;
}
