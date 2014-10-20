#include "Character.h"
#include "Data.h"
#include "SpellDataStore.h"
#include "GuildManager.h"
#include "Zone.h"
#include "Utility.h"
#include "LogSystem.h"
#include "ZoneClientConnection.h"
#include "Limits.h"
#include "NPC.h"
#include "Inventory.h"
#include "AccountManager.h"
#include "CombatSystem.h"

static const int AUTO_SAVE_FREQUENCY = 10000;

Character::Character(const uint32 pAccountID, CharacterData* pCharacterData) : mAccountID(pAccountID), mData(pCharacterData) {
	EXPECTED(mData);
	setName(pCharacterData->mName); // NOTE: This is required for ID before initialise has been called.

	setRunSpeed(0.7f);
	setWalkSpeed(0.35f);
	setBodyType(BT_Humanoid);
	setActorType(AT_PLAYER);
	//setIsNPC(false);

	mFilters.fill(0);
	mSkills.fill(0);
	mLanguages.fill(0);

	memset(mCurrency, 0, sizeof(mCurrency));

	mInventory = new Inventoryy(this);
}

Character::~Character() {
}

void Character::update() {

	if (isGM()) {
		if (mSuperGMPower.Check()) {
			mZone->handleLevelIncrease(this);
		}
	}

	if (isCasting()) {
		if (mCastingTimer.Check()) {
			finishCasting();
		}
	}

	if (mAutoSave.Check()) {
		_updateForSave();
		mZone->requestSave(this);
	}

	if (mAutoAttacking) {
		// TODO: Check range.
		// TODO: Check PvP
		// TODO: Check stunned.

		// Check: Primary Hand
		if (mPrimaryAttackTimer.check() && hasTarget()) {
			// Attacking an NPC.
			if (getTarget()->isNPC())
				CombatSystem::primaryMeleeAttack(this, Actor::cast<NPC*>(getTarget()));
			// Attacking a Character.
			else if (getTarget()->isCharacter())
				CombatSystem::primaryMeleeAttack(this, Actor::cast<Character*>(getTarget()));
		}
	}
}

bool Character::initialise() {
	EXPECTED_BOOL(mInitialised == false);

	setLastName(mData->mLastName);
	setTitle(mData->mTitle);
	setSuffix(mData->mSuffix);
	setRace(mData->mRace);
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
	setFaceStyle(mData->mFaceStyle);
	setDrakkinHeritage(mData->mDrakkinHeritage);
	setDrakkinTattoo(mData->mDrakkinTattoo);
	setDrakkinDetails(mData->mDrakkinDetails);

	setSize(Character::getDefaultSize(getRace()));

	setExperience(mData->mExperience);

	// Personal Currency
	setCurrency(MoneySlotID::PERSONAL, MoneyType::PLATINUM, mData->mPlatinumCharacter);
	setCurrency(MoneySlotID::PERSONAL, MoneyType::GOLD, mData->mGoldCharacter);
	setCurrency(MoneySlotID::PERSONAL, MoneyType::SILVER, mData->mSilverCharacter);
	setCurrency(MoneySlotID::PERSONAL, MoneyType::COPPER, mData->mCopperCharacter);

	// Cursor Currency
	setCurrency(MoneySlotID::CURSOR, MoneyType::PLATINUM, mData->mPlatinumCursor);
	setCurrency(MoneySlotID::CURSOR, MoneyType::GOLD, mData->mGoldCursor);
	setCurrency(MoneySlotID::CURSOR, MoneyType::SILVER, mData->mSilverCursor);
	setCurrency(MoneySlotID::CURSOR, MoneyType::COPPER, mData->mCopperCursor);

	// Bank Currency
	setCurrency(MoneySlotID::BANK, MoneyType::PLATINUM, mData->mPlatinumBank);
	setCurrency(MoneySlotID::BANK, MoneyType::GOLD, mData->mGoldBank);
	setCurrency(MoneySlotID::BANK, MoneyType::SILVER, mData->mSilverBank);
	setCurrency(MoneySlotID::BANK, MoneyType::COPPER, mData->mCopperBank);

	// Shared Bank Currency
	setCurrency(MoneySlotID::SHARED_BANK, MoneyType::PLATINUM, AccountManager::getInstance().getSharedPlatinum(mAccountID));

	mBaseStrength = mData->mStrength;
	mBaseStamina = mData->mStamina;
	mBaseCharisma = mData->mCharisma;
	mBaseDexterity = mData->mDexterity;
	mBaseIntelligence = mData->mIntelligence;
	mBaseAgility = mData->mAgility;
	mBaseWisdom = mData->mWisdom;

	if (mData->mGuildID != NO_GUILD) {
		GuildManager::getInstance().onConnect(this, mData->mGuildID);
	}

	// Skills
	for (int i = 0; i < Limits::Skills::MAX_ID; i++) {
		setSkill(i, mData->mSkills[i]);
	}

	// Languages
	for (int i = 0; i < Limits::Languages::MAX_ID; i++) {
		setLanguage(i, mData->mLanguages[i]);
	}

	// Armor Dye
	for (int i = 0; i < MAX_ARMOR_DYE_SLOTS; i++) {
		setColour(i, mData->mDyes[i]);
	}

	setAutoConsentGroup(mData->mAutoConsentGroup);
	setAutoConsentRaid(mData->mAutoConsentRaid);
	setAutoConsentGuild(mData->mAutoConsentGuild);

	_setRadiantCrystals(mData->mRadiantCrystals, mData->mTotalRadiantCrystals);
	_setEbonCrystals(mData->mEbonCrystals, mData->mTotalEbonCrystals);

	if (isCaster()) {
		// Create and initialise SpellBook.
		mSpellBook = new SpellBook();
		for (auto i = 0; i < Limits::SpellBook::MAX_SLOTS; i++) {
			if (mData->mSpellBook[i] != 0)
				mSpellBook->setSpell(i, mData->mSpellBook[i]);

			mSpellBook->setSpell(0, 1000); // Testing.
		}
		// Create and initialise SpellBar.
		mSpellBar = new SpellBar();
		for (auto i = 0; i < Limits::SpellBar::MAX_SLOTS; i++) {
			if (mData->mSpellBar[i] != 0)
				mSpellBar->setSpell(i, mData->mSpellBar[i]);
		}
	}

	mAutoSave.Start(AUTO_SAVE_FREQUENCY);
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

void Character::message(MessageType pType, String pMessage) {
	mConnection->sendMessage(pType, pMessage);
}

void Character::healPercentage(int pPercent) {
	mConnection->sendHPUpdate();
}

//void Character::damage(uint32 pAmount) {
//	mCurrentHP -= pAmount;
//	mConnection->sendHPUpdate();
//}

void Character::doAnimation(uint8 pAnimationID) {
	mZone->handleAnimation(this, 10, pAnimationID, true);
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

void Character::setCharacterLevel(uint8 pLevel) {
	// Ensure not going above maximum level.
	if (pLevel > Character::getMaxCharacterLevel()){
		pLevel = Character::getMaxCharacterLevel();
	}
	
	// Increasing.
	if (pLevel > getLevel()) {
		setLevel(pLevel);
		// Notify user.
		mConnection->sendLevelUpdate();
		mConnection->sendLevelGain();
		// Notify zone.
		mZone->handleLevelIncrease(this);
	}
	else if (pLevel < getLevel()) {
		mExperience = 0; // to be safe.
		setLevel(pLevel);
		// Notify user.
		mConnection->sendLevelUpdate();
		mConnection->sendLevelLost();
		// Notify zone.
		mZone->handleLevelDecrease(this);
	}
	
}

const uint32 Character::getExperienceRatio() const {
	// Protect against division by zero.
	uint32 next = getExperienceForNextLevel();
	if (next == 0) {
		Log::error("[Character] Prevented division by zero in getExperienceRatio");
		return 0;
	}

	return 330.0f * (mExperience / static_cast<float>(next));
}

const uint32 Character::getExperienceForLevel(const uint8 pLevel) {
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

	mData->mGM = isGM();
	mData->mName = getName();
	mData->mLastName = getLastName();
	mData->mTitle = getTitle();
	mData->mSuffix = getSuffix();

	mData->mLevel = getLevel();
	mData->mExperience = getExperience();

	mData->mRace = getRace();
	mData->mClass = getClass();

	mData->mBeardStyle = getBeardStyle();
	mData->mBeardColour = getBeardColour();
	mData->mHairStyle = getHairStyle();
	mData->mHairColour = getHairColour();
	mData->mEyeColourLeft = getLeftEyeColour();
	mData->mEyeColourRight = getRightEyeColour();
	mData->mFaceStyle = getFaceStyle();
	mData->mDrakkinHeritage = getDrakkinHeritage();
	mData->mDrakkinTattoo = getDrakkinTattoo();
	mData->mDrakkinDetails = getDrakkinDetails();

	mData->mGender = getGender();

	// Personal Currency
	mData->mPlatinumCharacter = getPersonalPlatinum();
	mData->mGoldCharacter = getPersonalGold();
	mData->mSilverCharacter = getPersonalSilver();
	mData->mCopperCharacter = getPersonalCopper();

	// Cursor Currency
	mData->mPlatinumCursor = getCursorPlatinum();
	mData->mGoldCursor = getCursorGold();
	mData->mSilverCursor = getCursorSilver();
	mData->mCopperCursor = getCursorCopper();

	// Bank Currency
	mData->mPlatinumBank = getBankPlatinum();
	mData->mGoldBank = getBankGold();
	mData->mSilverBank = getBankSilver();
	mData->mCopperBank = getBankCopper();

	// Shared Bank Currency
	AccountManager::getInstance().setSharedPlatinum(mAccountID, getSharedBankPlatinum());

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

	// Skills
	for (int i = 0; i < Limits::Skills::MAX_ID; i++)
		mData->mSkills[i] = getSkill(i);

	// Languages
	for (int i = 0; i < Limits::Languages::MAX_ID; i++)
		mData->mLanguages[i] = getLanguage(i);

	mData->mGuildID = getGuildID();
	mData->mGuildRank = getGuildRank();

	// Armor Dye
	for (int i = 0; i < MAX_ARMOR_DYE_SLOTS; i++) {
		//mData->mDyes[i] = getColour(i).mColour;
		//TODO!
	}

	mData->mAutoConsentGroup = getAutoConsentGroup();
	mData->mAutoConsentRaid = getAutoConsentRaid();
	mData->mAutoConsentGuild = getAutoConsentGuild();

	mData->mRadiantCrystals = getRadiantCrystals();
	mData->mTotalRadiantCrystals = getTotalRadiantCrystals();
	mData->mEbonCrystals = getEbonCrystals();
	mData->mTotalEbonCrystals = getTotalRadiantCrystals();

	// Spell Book
	if (mSpellBook) {
		const std::vector<uint32> spellBook = mSpellBook->getData();
		for (auto i = 0; i < Limits::SpellBook::MAX_SLOTS; i++)
			mData->mSpellBook[i] = spellBook[i];
	}

	// Spell Bar
	if (mSpellBar) {
		const std::vector<uint32> spellBar = mSpellBar->getData();
		for (auto i = 0; i < Limits::SpellBar::MAX_SLOTS; i++)
			mData->mSpellBar[i] = spellBar[i];
	}
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

void Character::notify(const String& pMessage) {
	mConnection->sendMessage(MessageType::Yellow, pMessage);
}

const bool Character::handleDeleteSpell(const int16 pSlot) {
	EXPECTED_BOOL(isCaster());
	EXPECTED_BOOL(mSpellBook);
	EXPECTED_BOOL(Limits::SpellBook::slotValid(pSlot));
	
	return mSpellBook->deleteSpell(pSlot);
}

const std::vector<uint32> Character::getSpellBookData() const {
	return mSpellBook->getData();
}

const std::vector<uint32> Character::getSpellBarData() const {
	return mSpellBar->getData();
}

const bool Character::handleSwapSpells(const uint16 pFrom, const uint16 pTo) {
	EXPECTED_BOOL(isCaster());
	EXPECTED_BOOL(mSpellBook);
	EXPECTED_BOOL(Limits::SpellBook::slotValid(pFrom));
	EXPECTED_BOOL(Limits::SpellBook::slotValid(pTo));

	return mSpellBook->swapSpells(pFrom, pTo);
}

const bool Character::handleMemoriseSpell(const uint16 pSlot, const uint32 pSpellID) {
	EXPECTED_BOOL(isCaster());
	EXPECTED_BOOL(mSpellBook && mSpellBar);
	EXPECTED_BOOL(Limits::SpellBar::slotValid(pSlot));
	EXPECTED_BOOL(mSpellBook->hasSpell(pSpellID));

	// Update Spell Bar.
	mSpellBar->setSpell(pSlot, pSpellID);

	// Notify Client (required).
	mConnection->sendMemoriseSpell(pSlot, pSpellID);
	return true;
}

const bool Character::handleUnmemoriseSpell(const uint16 pSlot) {
	EXPECTED_BOOL(isCaster());
	EXPECTED_BOOL(mSpellBook && mSpellBar);
	EXPECTED_BOOL(Limits::SpellBar::slotValid(pSlot));

	// Update Spell Bar.
	mSpellBar->setSpell(pSlot, 0);

	// Notify Client (required).
	mConnection->sendUnmemoriseSpell(pSlot);
	return true;
}

const bool Character::handleScribeSpell(const uint16 pSlot, const uint32 pSpellID) {
	// TODO: Items
	return true;
}

const bool Character::hasSpell(const uint16 pSlot, const uint32 pSpellID) const {
	EXPECTED_BOOL(isCaster());
	EXPECTED_BOOL(mSpellBar);

	const bool matches = mSpellBar->getSpellID(pSlot) == pSpellID;
	return matches;
}

const bool Character::canCast(const uint32 pSpellID) const {
	// TODO: Check spell level / class etc.
	return true;
}

const bool Character::beginCasting(const uint16 pSlot, const uint32 pSpellID) {
	EXPECTED_BOOL(isCaster());
	EXPECTED_BOOL(isCasting() == false);

	mIsCasting = true;
	mCastingSlot = pSlot;
	mCastingSpellID = pSpellID;
	mCastingTimer.Start(1000);

	return true;
}

const bool Character::finishCasting() {
	EXPECTED_BOOL(isCaster());
	EXPECTED_BOOL(isCasting());

	// Update Zone.
	mZone->handleCastingFinished(this);

	mConnection->sendRefreshSpellBar(mCastingSlot, mCastingSpellID);
	mConnection->sendEnableSpellBar(mCastingSpellID);
	//mConnection->sendSpellCastOn(); // temp

	mIsCasting = false;
	mCastingSlot = 0;
	mCastingSpellID = 0;
	mCastingTimer.Disable();

	return true;
}

const bool Character::preCastingChecks(const SpellData* pSpell) {
	EXPECTED_BOOL(pSpell);

	// Check: Caster has enough mana.
	// TODO: Adjusted mana cost for focus effects.
	if (getCurrentMana() < pSpell->mManaCost) {
		return false;
	}

	// Check: Caster class and level.
	if (Spell::canClassUse(pSpell, getClass(), getLevel()) == false) {
		return false;
	}

	// Check: Spell is allowed to be cast in Zone.
	if (Spell::zoneAllowed(pSpell, mZone)) {
		return false;
	}

	// Target is valid.

	return true;
}

const bool Character::postCastingChecks(const SpellData* pSpell) {
	return true;
}

const bool Character::send(EQApplicationPacket* pPacket) {
	EXPECTED_BOOL(mConnection);
	EXPECTED_BOOL(mIsLinkDead == false);
	mConnection->sendPacket(pPacket);
	return true;
}

const uint32 Character::getSkill(const uint32 pSkillID) const {
	if (!Limits::Skills::validID(pSkillID)) {
		Log::error("Skill ID out of range: " + std::to_string(pSkillID));
		return 0;
	}

	return mSkills[pSkillID];
}

const bool Character::setSkill(const uint32 pSkillID, const uint32 pValue) {
	EXPECTED_BOOL(Limits::Skills::validID(pSkillID));

	mSkills[pSkillID] = pValue;
	return true;
}

const uint32 Character::getAdjustedSkill(const uint32 pSkillID) const {
	// TODO: For now defer to base skill level.
	return getSkill(pSkillID);
}

const uint32 Character::getLanguage(const uint32 pLanguageID) const {
	if (!Limits::Languages::validID(pLanguageID)) {
		Log::error("Language ID out of range: " + std::to_string(pLanguageID));
		return 0;
	}

	return mLanguages[pLanguageID];
}

const bool Character::setLanguage(const uint32 pLanguageID, const uint32 pValue) {
	EXPECTED_BOOL(Limits::Languages::validID(pLanguageID));

	mLanguages[pLanguageID] = pValue;
	return true;
}

Actor* Character::findVisible(const uint32 pSpawnID) {
	// Search NPCs currently visible to this character.
	for (auto i : mVisibleNPCs) {
		if (i->getSpawnID() == pSpawnID)
			return i;
	}
	// Search Characters currently visible to this character.
	for (auto i : getVisibleTo()) {
		if (i->getSpawnID() == pSpawnID)
			return i;
	}

	return nullptr;
}

void Character::setZoneChange(const uint16 pZoneID, const uint16 pInstanceID) {
	EXPECTED(mZoneChange.mZoneID == 0);
	EXPECTED(mZoneChange.mInstanceID == 0);

	mZoneChange.mZoneID = pZoneID;
	mZoneChange.mInstanceID = pInstanceID;
}

const bool Character::checkZoneChange(const uint16 pZoneID, const uint16 pInstanceID) const {
	return mZoneChange.mZoneID == pZoneID && mZoneChange.mInstanceID == pInstanceID;
}

void Character::clearZoneChange() {
	mZoneChange.mZoneID = 0;
	mZoneChange.mInstanceID = 0;
}

const bool Character::removeRadiantCrystals(const uint32 pCrystals) {
	EXPECTED_BOOL(mRadiantCrystals >= pCrystals);
	
	mRadiantCrystals -= pCrystals;
	return true;
}

const bool Character::removeEbonCrystals(const uint32 pCrystals) {
	EXPECTED_BOOL(mEbonCrystals >= pCrystals);

	mEbonCrystals -= pCrystals;
	return true;
}

const uint64 Character::getTotalCurrency() const {
	uint64 total = 0;
	for (auto i = 0; i < MoneySlotID::MAX; i++) {
		for (auto j = 0; j < MoneyType::MAX; j++) {
			total += getCurrency(i, j) * std::pow(10, j);
		}
	}
	return total;
}

const bool Character::currencyValid() const {
	for (auto i = 0; i < MoneySlotID::MAX; i++) {
		for (auto j = 0; j < MoneyType::MAX; j++) {
			if (getCurrency(i, j) < 0)
				return false;
		}
	}

	return true;
}

void Character::setAutoAttack(const bool pAttacking) {
	mAutoAttacking = pAttacking;

	if (mAutoAttacking) {
		mPrimaryAttackTimer.setStep(2000);
		mPrimaryAttackTimer.start();
	}
}

const bool Character::SpellBook::deleteSpell(const uint16 pSlot) {
	EXPECTED_BOOL(Limits::SpellBook::slotValid(pSlot));
	EXPECTED_BOOL(mSpellIDs[pSlot] != 0);

	mSpellIDs[pSlot] = 0;
	return true;
}

void Character::SpellBook::setSpell(const uint16 pSlot, const uint32 pSpellID) {
	EXPECTED(Limits::SpellBook::slotValid(pSlot));
	EXPECTED(Limits::SpellBook::spellIDValid(pSpellID));

	mSpellIDs[pSlot] = pSpellID;
}

const bool Character::SpellBook::swapSpells(const uint16 pFrom, const uint16 pTo) {
	EXPECTED_BOOL(Limits::SpellBook::slotValid(pFrom));
	EXPECTED_BOOL(Limits::SpellBook::slotValid(pTo));

	const uint32 temp = mSpellIDs[pTo];
	mSpellIDs[pTo] = mSpellIDs[pFrom];
	mSpellIDs[pFrom] = temp;

	return true;
}

const bool Character::SpellBook::hasSpell(const uint32 pSpellID) {
	for (auto i : mSpellIDs){
		if (i == pSpellID)
			return true;
	}

	return false;
}

void Character::SpellBar::setSpell(const uint16 pSlot, const uint32 pSpellID) {
	EXPECTED(Limits::SpellBar::slotValid(pSlot));
	EXPECTED(Limits::SpellBar::spellIDValid(pSpellID));

	mSpellIDs[pSlot] = pSpellID;
}

const uint32 Character::SpellBar::getSpellID(const uint16 pSlot) const {
	if (Limits::SpellBar::slotValid(pSlot) == false) {
		return 0;
	}
	return mSpellIDs[pSlot];
}
