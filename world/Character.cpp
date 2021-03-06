#include "Character.h"
#include "Account.h"
#include "ServiceLocator.h"
#include "Data.h"
#include "SpellDataStore.h"
#include "GuildManager.h"
#include "Zone.h"
#include "Utility.h"
#include "LogSystem.h"
#include "ZoneConnection.h"
#include "Limits.h"
#include "NPC.h"
#include "Inventory.h"
#include "AccountManager.h"
#include "CombatSystem.h"
#include "Item.h"
#include "ExtendedTargetController.h"
#include "HateController.h"
#include "RespawnOptions.h"
#include "ExperienceController.h"
#include "ActorBonuses.h"
#include "BuffController.h"
#include "TaskController.h"

#include "Group.h"
#include "Guild.h"

static const int AUTO_SAVE_FREQUENCY = 10000;

Character::Character() {

}

Character::Character(Data::Character* pCharacterData) : mData(pCharacterData) {
	EXPECTED(mData);
	setName(pCharacterData->mName); // NOTE: This is required for ID before initialise has been called.

	setRunSpeed(0.7f);
	setWalkSpeed(0.35f);
	setBodyType(BT_Humanoid);
	setActorType(AT_PLAYER);
	
	mFilters.fill(0);
	mSkills.fill(0);
	mLanguages.fill(0);

	mXTargetController = new ExtendedTargetController();
	mRespawnOptions = new RespawnOptions();

	RespawnOption option;
	option.mID = 0;
	option.mZoneID = 2;
	option.mName = "Tits";
	mRespawnOptions->add(option);

	option.mID = 1;
	option.mZoneID = 13;
	option.mName = "Boobs";
	mRespawnOptions->add(option);

	option.mID = 2;
	option.mZoneID = 52;
	option.mName = "Jugs";
	mRespawnOptions->add(option);
}



Character::~Character() {
	mAccount = nullptr;
	mExperienceController = nullptr;
	mInventory = nullptr;
	mTaskController = nullptr;

	delete mXTargetController;
	delete mRespawnOptions;
}

void Character::update() {

	//if (isCasting()) {
	//	if (mCastingTimer.Check()) {
	//		finishCasting();
	//	}
	//}

	if (mAutoSave.Check()) {
		mZone->saveCharacter(this);
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

const bool Character::initialise(SharedPtr<Account> pAccount, Inventory* pInventory, Experience::Controller* pExperienceController, TaskController* pTaskController) {
	if (mInitialised) return false;
	if (!pAccount) return false;
	if (!pInventory) return false;
	if (!pExperienceController) return false;
	if (!pTaskController) return false;

	//mAccount = pAccount;
	//mInventory = pInventory;
	//mExperienceController = pExperienceController;
	//mTaskController = pTaskController;

	getActorBonuses()->add(pInventory);

	// Add base stats.
	auto bonuses = getBaseBonuses();
	bonuses->_addStrength(mData->mStrength);
	bonuses->_addStamina(mData->mStamina);
	bonuses->_addIntelligence(mData->mIntelligence);
	bonuses->_addWisdom(mData->mWisdom);
	bonuses->_addAgility(mData->mAgility);
	bonuses->_addDexterity(mData->mDexterity);
	bonuses->_addCharisma(mData->mCharisma);

	// Buffs.
	mBuffController->onLoad(mData->mBuffs);

	setSurname(mData->mLastName);
	setTitle(mData->mTitle);
	setSuffix(mData->mSuffix);
	setInspectMessage(mData->mInspectMessage);
	setRace(mData->mRace);
	setIsGM(mData->mGM);
	setMuted(mData->mMuted);
	setNew(mData->mNew);
	setClass(mData->mClass);
	setGender(mData->mGender);
	setLevel(mData->mExperience.mLevel);
	setStatus(mData->mStatus);

	// Guild.
	setGuildID(mData->mGuildID);
	setGuildRank(mData->mGuildRank);

	mPosition = mData->mPosition;
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

	// Personal Currency
	mInventory->setCurrency(CurrencySlot::Personal, CurrencyType::Platinum, mData->mPlatinumCharacter);
	mInventory->setCurrency(CurrencySlot::Personal, CurrencyType::Gold, mData->mGoldCharacter);
	mInventory->setCurrency(CurrencySlot::Personal, CurrencyType::Silver, mData->mSilverCharacter);
	mInventory->setCurrency(CurrencySlot::Personal, CurrencyType::Copper, mData->mCopperCharacter);

	// Cursor Currency
	mInventory->setCurrency(CurrencySlot::Cursor, CurrencyType::Platinum, mData->mPlatinumCursor);
	mInventory->setCurrency(CurrencySlot::Cursor, CurrencyType::Gold, mData->mGoldCursor);
	mInventory->setCurrency(CurrencySlot::Cursor, CurrencyType::Silver, mData->mSilverCursor);
	mInventory->setCurrency(CurrencySlot::Cursor, CurrencyType::Copper, mData->mCopperCursor);

	// Bank Currency
	mInventory->setCurrency(CurrencySlot::Bank, CurrencyType::Platinum, mData->mPlatinumBank);
	mInventory->setCurrency(CurrencySlot::Bank, CurrencyType::Gold, mData->mGoldBank);
	mInventory->setCurrency(CurrencySlot::Bank, CurrencyType::Silver, mData->mSilverBank);
	mInventory->setCurrency(CurrencySlot::Bank, CurrencyType::Copper, mData->mCopperBank);

	// Shared Bank Currency
	mInventory->setCurrency(CurrencySlot::SharedBank, CurrencyType::Platinum, mAccount->getSharedPlatinum());

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

	mInventory->_setRadiantCrystals(mData->mRadiantCrystals, mData->mTotalRadiantCrystals);
	mInventory->_setEbonCrystals(mData->mEbonCrystals, mData->mTotalEbonCrystals);

	// Alternate Currency.
	for (auto i : mData->mAlternateCurrency) {
		mInventory->setAlternateCurrencyQuantity(i.first, i.second);
	}

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

	// Bind Locations.
	for (auto i = 0; i < 5; i++) {
		mBindLocations[i].setZoneID(mData->mBindLocations[i].mZoneID);
		mBindLocations[i].setPosition(mData->mBindLocations[i].mPosition);
		mBindLocations[i].setHeading(mData->mBindLocations[i].mHeading);
	}

	mAutoSave.Start(AUTO_SAVE_FREQUENCY);
	mInitialised = true;
	return true;
}

bool Character::onEnterZone() {
	setZoning(false);
	_processMessageQueue();
	return true;
}

void Character::onLeaveZone() {
	setZoning(true);
	setZone(nullptr);
	setConnection(nullptr);
}

void Character::setStanding(bool pStanding) {
	mStanding = pStanding;
	if (mStanding) {
		mIsCamping = false;
		mCampTimer.Disable();
	}
}

void Character::startCamp() {
	mIsCamping = true;
	mCampTimer.Start(29000, true);
}

void Character::message(const u32 pType, String pMessage) {
	mConnection->sendMessage(pType, pMessage);
}

void Character::healPercentage(int pPercent) {
	mConnection->sendHealthUpdate();
}

void Character::doAnimation(uint8 pAnimationID) {
	mZone->onAnimationChange(this, 10, pAnimationID, true);
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

const bool Character::_updateForSave() {
	EXPECTED_BOOL(mData);
	EXPECTED_BOOL(mZone);

	mData->mGM = isGM();
	mData->mMuted = isMuted();
	mData->mNew = isNew();
	mData->mName = getName();
	mData->mLastName = getSurname();
	mData->mTitle = getTitle();
	mData->mSuffix = getSuffix();
	mData->mInspectMessage = getInspectMessage();
	
	// Buffs.
	for (auto i : mData->mBuffs)
		delete i;
	mData->mBuffs.clear();
	mBuffController->onSave(mData->mBuffs);

	// Experience.
	mExperienceController->onSave(&mData->mExperience);

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
	mData->mPlatinumCharacter = mInventory->getPersonalPlatinum();
	mData->mGoldCharacter = mInventory->getPersonalGold();
	mData->mSilverCharacter = mInventory->getPersonalSilver();
	mData->mCopperCharacter = mInventory->getPersonalCopper();

	// Cursor Currency
	mData->mPlatinumCursor = mInventory->getCursorPlatinum();
	mData->mGoldCursor = mInventory->getCursorGold();
	mData->mSilverCursor = mInventory->getCursorSilver();
	mData->mCopperCursor = mInventory->getCursorCopper();

	// Bank Currency
	mData->mPlatinumBank = mInventory->getBankPlatinum();
	mData->mGoldBank = mInventory->getBankGold();
	mData->mSilverBank = mInventory->getBankSilver();
	mData->mCopperBank = mInventory->getBankCopper();

	// Shared Bank Currency
	mAccount->setSharedPlatinum(mInventory->getSharedBankPlatinum());

	mData->mZoneID = mZone->getID();
	mData->mInstanceID = mZone->getInstanceID();
	mData->mPosition = mPosition;
	mData->mHeading = mHeading;

	auto bonuses = getBaseBonuses();
	mData->mStrength = bonuses->getStrength();
	mData->mStamina = bonuses->getStamina();
	mData->mIntelligence = bonuses->getIntelligence();
	mData->mWisdom = bonuses->getWisdom();
	mData->mAgility = bonuses->getAgility();
	mData->mDexterity = bonuses->getDexterity();
	mData->mCharisma = bonuses->getCharisma();

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

	mData->mRadiantCrystals = mInventory->getRadiantCrystals();
	mData->mTotalRadiantCrystals = mInventory->getTotalRadiantCrystals();
	mData->mEbonCrystals = mInventory->getEbonCrystals();
	mData->mTotalEbonCrystals = mInventory->getTotalEbonCrystals();

	// Alternate Currency
	mData->mAlternateCurrency = mInventory->getAlternateCurrency(); // Copy.

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

	// Inventory.
	mData->mInventory.mItems.clear(); // TODO: Need to copy before clearing. Otherwise failure will wipe inventory.
	EXPECTED_BOOL(mInventory->updateForSave(mData->mInventory));

	// Bind Locations
	for (auto i = 0; i < 5; i++) {
		auto bindLocation = getBindLocation(i);
		mData->mBindLocations[i].mZoneID = bindLocation.getZoneID();
		mData->mBindLocations[i].mPosition = bindLocation.getPosition();
		mData->mBindLocations[i].mHeading = bindLocation.getHeading();
	}

	// Tasks.
	mTaskController->onSave(mData->mCompletedTasks);

	return true;
}

void Character::_processMessageQueue() {
	for (auto i : mMessageQueue)
		mConnection->sendChannelMessage(i.mChannelID, i.mSenderName, i.mMessage);
	mMessageQueue.clear();
}

void Character::addQueuedMessage(const u32 pChannel, const String& pSenderName, const String& pMessage) {
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

const bool Character::handleScribeSpell(const u16 pSlot, const u32 pSpellID) {
	EXPECTED_BOOL(isCaster());
	EXPECTED_BOOL(mSpellBook);

	// Check: Character does not already have the spell.
	EXPECTED_BOOL(hasSpell(pSpellID) == false);

	// Check: The SpellBook slot is free.
	EXPECTED_BOOL(mSpellBook->isSlotFree(pSlot));

	mSpellBook->setSpell(pSlot, pSpellID);
	mConnection->sendScribeSpell(pSlot, pSpellID);

	return true;
}

const bool Character::hasSpell(const uint16 pSlot, const uint32 pSpellID) const {
	EXPECTED_BOOL(isCaster());
	EXPECTED_BOOL(mSpellBar);

	const bool matches = mSpellBar->getSpellID(pSlot) == pSpellID;
	return matches;
}

const bool Character::hasSpell(const u32 pSpellID) const {
	EXPECTED_BOOL(isCaster());
	EXPECTED_BOOL(mSpellBook);

	return mSpellBook->hasSpell(pSpellID);
}

const bool Character::canCast(const uint32 pSpellID) const {
	// TODO: Check spell level / class etc.
	return true;
}

//const bool Character::beginCasting(const uint16 pSlot, const uint32 pSpellID) {
//	EXPECTED_BOOL(isCaster());
//	EXPECTED_BOOL(isCasting() == false);
//
//	mIsCasting = true;
//	mCastingSlot = pSlot;
//	mCastingSpellID = pSpellID;
//	mCastingTimer.Start(1000);
//
//	return true;
//}
//
//const bool Character::finishCasting() {
//	EXPECTED_BOOL(isCaster());
//	EXPECTED_BOOL(isCasting());
//
//	// Update Zone.
//	mZone->onFinishCast(this);
//
//	mConnection->sendRefreshSpellBar(mCastingSlot, mCastingSpellID);
//	mConnection->sendEnableSpellBar(mCastingSpellID);
//	//mConnection->sendSpellCastOn(); // temp
//
//	mIsCasting = false;
//	mCastingSlot = 0;
//	mCastingSpellID = 0;
//	mCastingTimer.Disable();
//
//	return true;
//}

const bool Character::preCastingChecks(const Data::Spell* pSpell) {
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

const bool Character::postCastingChecks(const Data::Spell* pSpell) {
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

void Character::setAutoAttack(const bool pAttacking) {
	mAutoAttacking = pAttacking;

	if (mAutoAttacking) {
		mPrimaryAttackTimer.setStep(400);
		mPrimaryAttackTimer.start();
	}
}

const bool Character::canEquip(const Item* pItem, const uint32 pSlotID) const {
	EXPECTED_BOOL(pItem);

	// Check: Required Level
	// TODO: I need to check what happens to required level on augs.
	if (pItem->getRequiredLevel() > getLevel()) return false;

	// Check: Race
	const uint32 raceBit = EquipRaces::convert(getRace());
	if ((pItem->_getRaces() & raceBit) == 0) return false;

	// Check: Class
	const uint32 classBit = EquipClasses::convert(getClass());
	if ((pItem->_getClasses() & classBit) == 0) return false;

	// Check: Deity
	// NOTE: Deity is special because a value of 0 means ALL.
	const uint32 deityBit = EquipDeities::convert(getDeity());
	if (deityBit && (pItem->_getDeities() & deityBit) == 0) return false;

	// Check: Slot
	const uint32 slotBit = EquipSlots::convert(pSlotID);
	if ((pItem->_getSlots() & slotBit) == 0) return false;

	// TODO: Check dw constraints. 1h in secondary while 2h in primary

	return true;
}

const bool Character::onWornSlotChange(const uint32 pSlot, Item* pOldItem, Item* pNewItem) {
	EXPECTED_BOOL(SlotID::isWorn(pSlot));
	EXPECTED_BOOL(pOldItem || pNewItem);

	// TODO: This where I will add / remove worn effects on Items.

	switch (pSlot) {
	case SlotID::Primary: {
		EXPECTED_BOOL(onPrimarySlotChange(pNewItem));
		break;
	}
	case SlotID::Secondary: {
		EXPECTED_BOOL(onSecondarySlotChange(pNewItem));
		break;
	}
	case SlotID::Range: {
		EXPECTED_BOOL(onRangeItemChange(pNewItem));
		break;
	}
	default:
		break;
	}

	return true;
}

const bool Character::onPrimarySlotChange(Item* pItem) {
	_clearPrimary();
	if (pItem == nullptr) return true; // No Item was equipped.
	if (pItem->isWeapon() == false) return true; // Non-weapon being equipped.

	// Update primary damage.
	setPrimaryDamage(pItem->_getDamage());
	
	// Update primary elemental damage.
	setPrimaryFireDamage(pItem->_getElementalDamage(ElementalDamageType::Fire));
	setPrimaryColdDamage(pItem->_getElementalDamage(ElementalDamageType::Cold));
	setPrimaryPoisonDamage(pItem->_getElementalDamage(ElementalDamageType::Poison));
	setPrimaryDiseaseDamage(pItem->_getElementalDamage(ElementalDamageType::Disease));
	setPrimaryChromaticDamage(pItem->_getElementalDamage(ElementalDamageType::Chromatic));
	setPrimaryPrismaticDamage(pItem->_getElementalDamage(ElementalDamageType::Prismatic));
	setPrimaryPhysicalDamage(pItem->_getElementalDamage(ElementalDamageType::Phys));
	setPrimaryCorruptionDamage(pItem->_getElementalDamage(ElementalDamageType::Corrupt));

	// Update primary animation
	switch (pItem->getItemType()) {
	case ItemType::OneHandSlash:
		setPrimaryAttackAnimation(Animation::ANIM_1HWEAPON);
		break;
	case ItemType::TwoHandSlash:
		setPrimaryAttackAnimation(Animation::ANIM_2HSLASHING);
		break;
	case ItemType::OneHandBlunt:
		setPrimaryAttackAnimation(Animation::ANIM_1HWEAPON);
		break;
	case ItemType::TwoHandBlunt:
		setPrimaryAttackAnimation(Animation::ANIM_2HWEAPON);
		break;
	case ItemType::OneHandPierce:
		setPrimaryAttackAnimation(Animation::ANIM_PIERCING);
		break;
	case ItemType::TwoHandPierce:
		setPrimaryAttackAnimation(Animation::ANIM_2HWEAPON);
		break;
	case ItemType::HandToHand:
		setPrimaryAttackAnimation(Animation::ANIM_HAND2HAND);
		break;
	default:
		Log::error("Got unknown ItemType in onPrimarySlotChange " + std::to_string((int)pItem->getItemType()));
		setPrimaryAttackAnimation(Animation::ANIM_HAND2HAND);
		break;
	}

	return true;
}

const bool Character::onSecondarySlotChange(Item* pItem)
{
	return true;
}

const bool Character::onRangeItemChange(Item* pItem)
{
	return true;
}

const bool Character::canCombine() const {
	if (isDead()) return false;
	if (isTrading()) return false;
	if (isShopping()) return false;

	// TODO: There will be more.

	return true;
}

const bool Character::canShop() const {
	if (isDead()) return false;
	if (isTrading()) return false;
	if (isShopping()) return false;

	// TODO: There will be more.

	return true;
}

const u16 Character::getFreeSpellBookSlot() const {
	EXPECTED_VAR(isCaster(), 0);
	EXPECTED_BOOL(mSpellBook);

	return mSpellBook->getFreeSlot();
}

const u16 Character::getSpellBookSlot(const u32 pSpellID) const {
	EXPECTED_VAR(isCaster(), 0);
	EXPECTED_BOOL(mSpellBook);

	return mSpellBook->getSlot(pSpellID);
}

const bool Character::onDeath() {
	setHPPercent(0);
	setActorType(AT_PLAYER_CORPSE);
	setName(getName() + "'s corpse");

	setAutoAttack(false);
	clearTarget(true);
	clearHaters();

	return true;
}

const u8 Character::getLevel() const {
	return mExperienceController->getLevel();
}

void Character::setLevel(const u8 pLevel) {
	mExperienceController->setLevel(pLevel);
	Actor::setLevel(pLevel);
}

const bool Character::isGroupLeader() {
	if (!hasGroup()) return false;
	return mGroup->isLeader(this);
}

void Character::setGuild(Guild* pGuild) {
	mGuild = pGuild;

	if (mGuild) {
		setGuildID(mGuild->getID());
	}
	else {
		setGuildID(GuildID::None);
		setGuildRank(GuildRank::None);
	}
}

void Character::setGroup(Group* pGroup) {
	mGroup = pGroup;

	if (!mGroup) {
		setIsGroupMainTank(false);
		setIsGroupMainTank(false);
		setIsGroupPuller(false);
	}
}

const bool Character::isRaidLeader() const {
	if (!hasRaid()) return false;
	//return mRaid->isLeader(this);
	return false;
}

const bool Character::canRequestTrade() const {
	if (isTrading()) return false;
	if (isDead()) return false;
	if (isCasting()) return false;
	if (isShopping()) return false;
	if (isStunned()) return false;

	return true;
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

const u16 Character::SpellBook::getFreeSlot() const {
	for (u16 i = 0; i < Limits::SpellBook::MAX_SLOTS; i++) {
		if (mSpellIDs[i] == 0)
			return i;
	}
	return 0;
}

const bool Character::SpellBook::isSlotFree(const u16 pSlot) const {
	EXPECTED_BOOL(Limits::SpellBook::slotValid(pSlot));
	return mSpellIDs[pSlot] == 0;
}

const u16 Character::SpellBook::getSlot(const u32 pSpellID) const {
	EXPECTED_VAR(Limits::SpellBar::spellIDValid(pSpellID), 0);

	for (u16 i = 0; i < Limits::SpellBook::MAX_SLOTS; i++) {
		if (mSpellIDs[i] == pSpellID)
			return i;
	}
	return 0;
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
