#include "Character.h"
#include "Data.h"
#include "GuildManager.h"
#include "Zone.h"
#include "Utility.h"
#include "LogSystem.h"
#include "../common/eq_packet_structs.h"
#include "../common/extprofile.h"
#include "ZoneClientConnection.h"

static const int AUTO_SAVE_FREQUENCY = 10000;

Character::Character(const uint32 pAccountID, CharacterData* pCharacterData) : mAccountID(pAccountID), mData(pCharacterData) {
	EXPECTED(mData);
	setName(pCharacterData->mName); // NOTE: This is required for ID before initialise has been called.

	setRunSpeed(0.7f);
	setWalkSpeed(0.35f);
	setBodyType(BT_Humanoid);
	setActorType(AT_PLAYER);
	setIsNPC(false);
}

Character::~Character() {
}

void Character::update() {

	if (getIsGM()) {
		if (mSuperGMPower.Check()) {
			mZone->notifyCharacterLevelIncrease(this);
		}
	}

	if (mAutoSave.Check()) {
		_updateForSave();
		mZone->requestSave(this);
	}
}

bool Character::initialise() {
	EXPECTED_BOOL(mInitialised == false);

	setLastName(mData->mLastName);
	setTitle(mData->mTitle);
	setSuffix(mData->mSuffix);
	setRaceID(mData->mRace);
	setIsGM(mData->mGM);
	setClass(mData->mClass);
	setGender(mData->mGender);
	setLevel(mData->mLevel);
	setStatus(mData->mStatus);

	mPosition.x = mData->mX;
	mPosition.y = mData->mY;
	mPosition.z = mData->mZ;
	mHeading = mData->mHeading;
	_syncPosition();

	setBeardStyle(mData->mBeardStyle);
	setBeardColour(mData->mBeardColour);
	setHairStyle(mData->mHairStyle);
	setHairColour(mData->mHairColour);
	setFaceStyle(mData->mFace);
	setDrakkinHeritage(mData->mDrakkinHeritage);
	setDrakkinTattoo(mData->mDrakkinTattoo);
	setDrakkinDetails(mData->mDrakkinDetails);

	setSize(Character::getDefaultSize(getRaceID()));

	mExperience = mData->mExperience;

	mPlatinum = mData->mPlatinumCharacter;
	mGold = mData->mGoldCharacter;
	mSilver = mData->mSilverCharacter;
	mCopper = mData->mCopperCharacter;

	mBaseStrength = mData->mStrength;
	mBaseStamina = mData->mStamina;
	mBaseCharisma = mData->mCharisma;
	mBaseDexterity = mData->mDexterity;
	mBaseIntelligence = mData->mIntelligence;
	mBaseAgility = mData->mAgility;
	mBaseWisdom = mData->mWisdom;

	mAutoSave.Start(AUTO_SAVE_FREQUENCY);

	if (mData->mGuildID != NO_GUILD) {
		GuildManager::getInstance().onConnect(this, mData->mGuildID);
	}

	mInitialised = true;
	return true;
}

bool Character::onZoneIn() {
	setZoning(false);
	_processMessageQueue();
	return true;
}

bool Character::onZoneOut() {
	setZoning(true);
	setZone(nullptr);
	setSpawnID(0);
	setConnection(nullptr);

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

void Character::message(MessageType pType, String pMessage)
{
	mConnection->sendMessage(pType, pMessage);
}

void Character::healPercentage(int pPercent) {
	mConnection->sendHPUpdate();
}

void Character::damage(uint32 pAmount) {
	mCurrentHP -= pAmount;
	mConnection->sendHPUpdate();
}

void Character::doAnimation(uint8 pAnimationID) {
	mZone->notifyCharacterAnimation(this, 10, pAnimationID, true);
}

void Character::addExperience(uint32 pExperience) {
	// Handle special case where Character is max level / experience.
	if (getLevel() == Character::getMaxCharacterLevel()) {
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

	// Send user a message.
	mConnection->sendExperienceLoss();
	// Update user experience bar.
	mConnection->sendExperienceUpdate();
}

void Character::_checkForLevelIncrease() {
	while (mExperience >= getExperienceForNextLevel()) {
		mExperience -= getExperienceForNextLevel();
		setLevel(getLevel() + 1);

		if (hasGuild())
			GuildManager::getInstance().onLevelChange(this);
	}
}

//void Character::setLevel(uint8 pLevel) {
//	// Ensure not going above maximum level.
//	if (pLevel > Character::getMaxCharacterLevel()){
//		pLevel = Character::getMaxCharacterLevel();
//	}
//	
//	// Increasing.
//	if (pLevel > getLevel()) {
//		setLevel(pLevel);
//		// Notify user.
//		mConnection->sendLevelUpdate();
//		mConnection->sendLevelGain();
//		// Notify zone.
//		mZone->notifyCharacterLevelIncrease(this);
//	}
//	else if (pLevel < getLevel()) {
//		mExperience = 0; // to be safe.
//		setLevel(pLevel);
//		// Notify user.
//		mConnection->sendLevelUpdate();
//		mConnection->sendLevelLost();
//		// Notify zone.
//		mZone->notifyCharacterLevelDecrease(this);
//	}
//	
//}

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
	EXPECTED(mData);
	EXPECTED(mZone);

	mData->mGM = getIsGM();
	mData->mName = getName();
	mData->mLastName = getLastName();
	mData->mTitle = getTitle();
	mData->mSuffix = getSuffix();

	mData->mLevel = getLevel();
	mData->mExperience = mExperience;

	mData->mRace = getRaceID();
	mData->mClass = getClass();

	mData->mBeardStyle = getBeardStyle();
	mData->mBeardColour = getBeardColour();
	mData->mHairStyle = getHairStyle();
	mData->mHairColour = getHairColour();
	mData->mEyeColourLeft = getLeftEyeColour();
	mData->mEyeColourRight = getRightEyeColour();
	mData->mFace = getFaceStyle();
	mData->mDrakkinHeritage = getDrakkinHeritage();
	mData->mDrakkinTattoo = getDrakkinTattoo();
	mData->mDrakkinDetails = getDrakkinDetails();

	mData->mGender = getGender();

	mData->mPlatinumCharacter = mPlatinum;
	mData->mGoldCharacter = mGold;
	mData->mSilverCharacter = mSilver;
	mData->mCopperCharacter = mCopper;

	// TODO: Bank / Cursor currency

	mData->mZoneID = mZone->getID();
	mData->mInstanceID = mZone->getInstanceID();
	mData->mX = mPosition.x;
	mData->mY = mPosition.y;
	mData->mZ = mPosition.z;
	mData->mHeading = mHeading;

	mData->mStrength = mBaseStrength;
	mData->mStamina = mBaseStamina;
	mData->mCharisma = mBaseCharisma;
	mData->mDexterity = mBaseDexterity;
	mData->mIntelligence = mBaseIntelligence;
	mData->mAgility = mBaseAgility;
	mData->mWisdom = mBaseWisdom;

	mData->mGuildID = getGuildID();
	mData->mGuildRank = getGuildRank();
}

uint32 Character::getBaseStatistic(Statistic pStatistic) {
	switch (pStatistic)
	{
	case Statistic::Strength:
		return mBaseStrength;
		break;
	case Statistic::Stamina:
		return mBaseStamina;
		break;
	case Statistic::Charisma:
		return mBaseCharisma;
		break;
	case Statistic::Dexterity:
		return mBaseDexterity;
		break;
	case Statistic::Intelligence:
		return mBaseIntelligence;
		break;
	case Statistic::Agility:
		return mBaseAgility;
		break;
	case Statistic::Wisdom:
		return mBaseWisdom;
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
		mBaseStrength = pValue;
		break;
	case Statistic::Stamina:
		mBaseStamina = pValue;
		break;
	case Statistic::Charisma:
		mBaseCharisma = pValue;
		break;
	case Statistic::Dexterity:
		mBaseDexterity = pValue;
		break;
	case Statistic::Intelligence:
		mBaseIntelligence = pValue;
		break;
	case Statistic::Agility:
		mBaseAgility = pValue;
		break;
	case Statistic::Wisdom:
		mBaseWisdom = pValue;
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

void Character::addVisibleTo(Character* pCharacter)
{
	EXPECTED(pCharacter); EXPECTED(pCharacter != this); mVisibleTo.push_back(pCharacter);
}

void Character::removeVisibleTo(Character* pCharacter) {
	EXPECTED(pCharacter);
	EXPECTED(mVisibleTo.empty() == false);
	std::size_t oSize = mVisibleTo.size();
	mVisibleTo.remove(pCharacter);
	EXPECTED(mVisibleTo.size() == oSize - 1);
}
