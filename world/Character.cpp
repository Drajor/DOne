#include "Character.h"
#include "Zone.h"
#include "Utility.h"
#include "LogSystem.h"
#include "../common/eq_packet_structs.h"
#include "../common/extprofile.h"
#include "ZoneClientConnection.h"

static const int AUTO_SAVE_FREQUENCY = 10000;

Character::Character(uint32 pCharacterID, ClientAuthentication& pAuthentication) :
mCharacterID(pCharacterID),
mAuthentication(pAuthentication),
mProfile(0),
mExtendedProfile(0),
mName("soandso"),
mLastName(""),
mTitle(""),
mSuffix(""),
mRace(0),
mOriginalRace(0),
mRunSpeed(0.7f),
mWalkSpeed(0.35f),
mClass(0),
mGender(0),
mLevel(1),
mDeity(394), // Agnostic
mSize(5.0f),
mStanding(true),
mAFK(false),
mCampComplete(false),
mTGB(false),
mStatus(0),
mZone(nullptr),
mCurrentHP(100),
mMaximumHP(100),
mCurrentMana(100),
mMaximumMana(100),
mCurrentEndurance(100),
mMaximumEndurance(100),
mX(0.0f),
mY(0.0f),
mZ(0.0f),
mHeading(0.0f),
mDeltaX(0),
mDeltaY(0),
mDeltaZ(0),
mDeltaHeading(0),
mAnimation(0),
mAppearance(SpawnAppearanceAnimation::Standing),
mGM(false),
mGuildID(0xFFFFFFFF),
mGuildRank(0xFF),
mExperience(0),
mCopper(0),
mSilver(0),
mGold(0),
mPlatinum(0),
mGroup(nullptr),
mGuild(nullptr),
mRaid(nullptr),
mIsZoning(false),
mIsLinkDead(false),
mIsZoningOut(false)
{ }
Character::~Character() {
	delete mProfile;
	delete mExtendedProfile;
}

void Character::update() {

	if (mGM) {
		if (mSuperGMPower.Check()) {
			mZone->notifyCharacterLevelIncrease(this);
		}
	}

	if (mAutoSave.Check()) {
		_updateForSave();
		mZone->requestSave(this);
	}
}

bool Character::initialise(PlayerProfile_Struct* pProfile, ExtendedProfile_Struct* pExtendedProfile) {
	mProfile = pProfile;
	mExtendedProfile = pExtendedProfile;

	mName = mProfile->name;
	mLastName = mProfile->last_name;
	mTitle = mProfile->title;
	mSuffix = mProfile->suffix;
	mOriginalRace = mRace = mProfile->race;
	mClass = mProfile->class_;
	mGender = mProfile->gender;
	mLevel = mProfile->level;
	mStatus = 255;
	mGM = (mProfile->gm == 1);
	mGM = true; // testing!

	mX = mProfile->x;
	mY = mProfile->y;
	mZ = mProfile->z;
	mHeading = mProfile->heading;
	mDeltaX = 0;
	mDeltaY = 0;
	mDeltaZ = 0;
	mDeltaHeading = 0;
	mSize = Character::getDefaultSize(mRace);

	mExperience = mProfile->exp;

	//mSuperGMPower.Start(2000);
	mAutoSave.Start(AUTO_SAVE_FREQUENCY);

	mCopper = mProfile->copper;
	mSilver = mProfile->silver;
	mGold = mProfile->gold;
	mPlatinum = mProfile->platinum;

	// Perform any profile patching that needs to be done.
	_initialiseProfile();

	return true;
}

void Character::_initialiseProfile() {
	// TODO:
}

bool Character::onZoneIn() {
	mIsZoning = false;
	_processMessageQueue();
	return true;
}

bool Character::onZoneOut() {
	mIsZoning = true;
	mZone = nullptr;
	mSpawnID = 0;
	mConnection = nullptr;
	return true;
}

void Character::setStanding(bool pStanding) {
	mStanding = pStanding;
	if (mStanding) {
		mCampTimer.Disable();
		_setAppearance(Standing);
	}
	else {
		_setAppearance(Sitting);
	}
		
}

void Character::startCamp() {
	mCampTimer.Start(29000, true);
}

void Character::setAFK(bool pAFK) {
	mAFK = pAFK;
}

bool Character::getAFK() {
	return mAFK;
}

void Character::setShowHelm(bool pShowHelm) {
	mProfile->showhelm = pShowHelm ? 1 : 0;
}
bool Character::getShowHelm() {
	return mProfile->showhelm == 1;
}

void Character::message(MessageType pType, String pMessage)
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

uint8 Character::getAnonymous() {
	return mProfile->mAnonymous;
}


void Character::setPositionDeltas(float pDeltaX, float pDeltaY, float pDeltaZ, int32 pDeltaHeading)
{
	mDeltaX = pDeltaX;
	mDeltaY = pDeltaY;
	mDeltaZ = pDeltaZ;
	mDeltaHeading = pDeltaHeading;
}

uint8 Character::getGM() {
	return mGM ? 1 : 0;
}

void Character::setGM(bool pGM) {
	mGM = pGM;
	mProfile->gm = getGM();
}


void Character::doAnimation(uint8 pAnimationID) {
	mZone->notifyCharacterAnimation(this, 10, pAnimationID, true);
}

void Character::addExperience(uint32 pExperience) {
	// Handle special case where Character is max level / experience.
	if (mLevel == Character::getMaxCharacterLevel()) {
		// Character is already at max exp - 1
		if (mExperience == getExperienceForNextLevel() - 1) {
			message(MessageType::LightBlue, "You can no longer gain experience.");
			return;
		}
		// This experience will take the Character over the limit.
		else if (mExperience + pExperience > getExperienceForNextLevel() - 1) {
			// Modify pExeperience so that pExperience + mExperience = getExperienceForNextLevel - 1 (Capped).
			pExperience = (getExperienceForNextLevel() - 1 - mExperience);
		}
	}


	mExperience += pExperience;
	mConnection->sendExperienceGain();
	_checkForLevelIncrease();
	mProfile->exp = mExperience;
	// Update user experience bar.
	mConnection->sendExperienceUpdate();
}

void Character::removeExperience(uint32 pExperience) {
	// No loss of level at this stage.
	if (mExperience == 0) return;

	// Prevent experience value wrapping.
	if (pExperience > mExperience) {
		pExperience = mExperience;
	}

	mExperience -= pExperience;
	mProfile->exp = mExperience;

	// Send user a message.
	mConnection->sendExperienceLoss();
	// Update user experience bar.
	mConnection->sendExperienceUpdate();
}

void Character::_checkForLevelIncrease() {
	while (mExperience >= getExperienceForNextLevel()) {
		mExperience -= getExperienceForNextLevel();
		setLevel(mLevel + 1);
	}
}

void Character::setLevel(uint8 pLevel) {
	// Ensure not going above maximum level.
	if (pLevel > Character::getMaxCharacterLevel()){
		pLevel = Character::getMaxCharacterLevel();
	}
	
	// Increasing.
	if (pLevel > mLevel) {
		mLevel = pLevel;
		// Notify user.
		mConnection->sendLevelUpdate();
		mConnection->sendLevelGain();
		// Notify zone.
		mZone->notifyCharacterLevelIncrease(this);
	}
	else if (pLevel < mLevel) {
		mExperience = 0; // to be safe.
		mLevel = pLevel;
		// Notify user.
		mConnection->sendLevelUpdate();
		mConnection->sendLevelLost();
		// Notify zone.
		mZone->notifyCharacterLevelDecrease(this);
	}
	
}

uint32 Character::getExperienceRatio() {
	// Protect against division by zero.
	uint32 next = getExperienceForNextLevel();
	if (next == 0) {
		Log::error("[Character] Prevented division by zero in getExperienceRatio");
		return 0;
	}

	return 330.0f * (mExperience / static_cast<float>(next));
}

uint32 Character::getExperienceForLevel(uint8 pLevel)
{
	return (pLevel * pLevel) * 20;
}

float Character::getDefaultSize(uint32 pRace) {
	switch (pRace) {
	case Ogre:
		return 9;
	case Troll:
		return 8;
	case Vahshir:
	case Barbarian:
		return 7;
	case Human:
	case HighElf:
	case Erudite:
	case Iksar:
	case Drakkin:
		return 6;
	case HalfElf:
		return 5.5;
	case WoodElf:
	case DarkElf:
	case Froglok:
		return 5;
	case Dwarf:
		return 4;
	case Halfling:
		return 3.5;
	case Gnome:
		return 3;
	default:
		return 0;
	}
}

void Character::_updateForSave() {
	// Ensure profile is updated for save.

	// TODO: Consider checking the ranges of these strings before saving.
	strcpy(mProfile->name, getName().c_str());
	strcpy(mProfile->last_name, getLastName().c_str());
	strcpy(mProfile->title, getTitle().c_str());
	strcpy(mProfile->suffix, getSuffix().c_str());

	mProfile->gm = getGM();
	mProfile->class_ = getClass();
	mProfile->race = getOriginalRace();
	mProfile->deity = getDeity();
	mProfile->gender = getGender();
	mProfile->mAnonymous = getAnonymous();
	mProfile->showhelm = getShowHelm();

	mProfile->copper = getCopper();
	mProfile->silver = getSilver();
	mProfile->gold = getGold();
	mProfile->platinum = getPlatinum();
	
	mProfile->x = getX();
	mProfile->y = getY();
	mProfile->z = getZ();
	mProfile->heading = getHeading();

	mProfile->level = getLevel();
	mProfile->exp = getExperience();

	mProfile->zone_id = mZone->getID();
	mProfile->zoneInstance = mZone->getInstanceID();

	mProfile->guild_id = getGuildID();
	mProfile->guildrank = getGuildID();
}

uint32 Character::getBaseStatistic(Statistic pStatistic) {
	switch (pStatistic)
	{
	case Statistic::Strength:
		return mProfile->STR;
		break;
	case Statistic::Stamina:
		return mProfile->STA;
		break;
	case Statistic::Charisma:
		return mProfile->CHA;
		break;
	case Statistic::Dexterity:
		return mProfile->DEX;
		break;
	case Statistic::Intelligence:
		return mProfile->INT;
		break;
	case Statistic::Agility:
		return mProfile->AGI;
		break;
	case Statistic::Wisdom:
		return mProfile->WIS;
		break;
	default:
		Log::error("[Character] Unknown Statistic in getBaseStatistic.");
		break;
	}

	return 0;
}

void Character::setBaseStatistic(Statistic pStatistic, uint32 pValue) {
	switch (pStatistic) {
	case Statistic::Strength:
		mProfile->STR = pValue;
		break;
	case Statistic::Stamina:
		mProfile->STA = pValue;
		break;
	case Statistic::Charisma:
		mProfile->CHA = pValue;
		break;
	case Statistic::Dexterity:
		mProfile->DEX = pValue;
		break;
	case Statistic::Intelligence:
		mProfile->INT = pValue;
		break;
	case Statistic::Agility:
		mProfile->AGI = pValue;
		break;
	case Statistic::Wisdom:
		mProfile->WIS = pValue;
		break;
	default:
		Log::error("[Character] Unknown Statistic in setBaseStatistic.");
		break;
	}
}

uint32 Character::getStatistic(Statistic pStatistic) {
	switch (pStatistic)
	{
	case Statistic::Strength:
		return getStrength();
	case Statistic::Stamina:
		return getStamina();
	case Statistic::Charisma:
		return getCharisma();
	case Statistic::Dexterity:
		return getDexterity();
	case Statistic::Intelligence:
		return getIntelligence();
	case Statistic::Agility:
		return getAgility();
	case Statistic::Wisdom:
		return getWisdom();
	default:
		Log::error("[Character] Unknown Statistic in getStatistic.");
		break;
	}

	return 0;
}

void Character::_processMessageQueue() {
	for (auto i : mMessageQueue)
		mConnection->sendChannelMessage(i.mChannelID, i.mSenderName, i.mMessage);
	mMessageQueue.clear();
}

void Character::addQueuedMessage(ChannelID pChannel, const String& pSenderName, const String& pMessage) {
	mMessageQueue.push_back({ pChannel, pSenderName, pMessage });
}

void Character::setLinkDead() {
	mIsLinkDead = true;
}
