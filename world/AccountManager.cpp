#include "AccountManager.h"
#include "DataStore.h"
#include "Data.h"
#include "LogSystem.h"
#include "Utility.h"
#include "Profile.h"
#include "Payload.h"
#include "Limits.h"

#include "Character.h"
#include "Zone.h"

#include "Shlwapi.h"

static bool deleteAll(Data::Account* pValue) { delete pValue; return true; };
AccountManager::~AccountManager() {
	_save();
	_clear();
}

bool AccountManager::initialise() {
	Profile p("AccountManager::initialise");
	Log::status("[Account Manager] Initialising.");

	const bool accountsLoaded = DataStore::getInstance().loadAccounts(mAccounts);
	EXPECTED_BOOL(accountsLoaded);

	Log::info("[Account Manager] Loaded data for " + std::to_string(mAccounts.size()) + " Accounts.");
	return true;
}

bool AccountManager::createAccount(const u32 pAccountID, const String pAccoutName){
	EXPECTED_BOOL(exists(pAccountID) == false);

	auto accountData = new Data::Account();
	accountData->mAccountID = pAccountID;
	accountData->mAccountName = pAccoutName;
	accountData->mCreated = Utility::Time::now();
	accountData->mStatus = ResponseID::ALLOWED;

	mAccounts.push_back(accountData);
	EXPECTED_BOOL(_save());
	EXPECTED_BOOL(_save(accountData));

	return true;
}

void AccountManager::_clear() {
	mAccounts.remove_if(Utility::containerEntryDelete<Data::Account*>);
}
bool AccountManager::_save() {
	EXPECTED_BOOL(DataStore::getInstance().saveAccounts(mAccounts));

	return true;
}

bool AccountManager::_save(Data::Account* pAccountData) {
	EXPECTED_BOOL(pAccountData);
	EXPECTED_BOOL(DataStore::getInstance().saveAccountCharacterData(pAccountData));

	return true;
}

bool AccountManager::exists(const u32 pAccountID){
	auto account = _find(pAccountID);
	return account ? true : false;
}

AccountStatus AccountManager::getStatus(const u32 pAccountID) {
	auto account = _find(pAccountID);
	return account ? account->mStatus : ResponseID::ALLOWED;
}

bool AccountManager::deleteCharacter(const u32 pAccountID, const String& pCharacterName) {
	auto account = _find(pAccountID);
	EXPECTED_BOOL(account);
	EXPECTED_BOOL(DataStore::getInstance().deleteCharacter(pCharacterName));

	// Remove from AccountData
	bool removed = false;
	for (auto i : account->mCharacterData) {
		if (i->mName == pCharacterName) {
			delete i;
			account->mCharacterData.remove(i);
			removed = true;
			break;
		}
	}
	EXPECTED_BOOL(removed); // Check: Sanity.
	EXPECTED_BOOL(_save(account)); // Save.

	return true;
}

bool AccountManager::isCharacterNameUnique(const String& pCharacterName)
{
	return true;
}

Data::Account* AccountManager::_find(const u32 pAccountID) const {
	for (auto i : mAccounts) {
		if (i->mAccountID == pAccountID)
			return i;
	}
	return nullptr;
}

Data::Account* AccountManager::_find(const String& pAccountName) const {
	for (auto i : mAccounts) {
		if (i->mAccountName == pAccountName)
			return i;
	}
	return nullptr;
}

bool AccountManager::ban(const String& pAccountName) {
	auto accountData = _find(pAccountName);
	EXPECTED_BOOL(accountData);
	accountData->mStatus = ResponseID::BANNED;

	EXPECTED_BOOL(_save());
	return true;
}

bool AccountManager::removeBan(const String& pAccountName) {
	auto accountData = _find(pAccountName);
	EXPECTED_BOOL(accountData);
	accountData->mStatus = ResponseID::ALLOWED;

	EXPECTED_BOOL(_save());
	return true;
}

bool AccountManager::suspend(const String& pAccountName, const u32 pSuspendUntil) {
	auto accountData = _find(pAccountName);
	EXPECTED_BOOL(accountData);
	accountData->mStatus = ResponseID::SUSPENDED;
	accountData->mSuspendedUntil = pSuspendUntil;

	EXPECTED_BOOL(_save());
	return true;
}

bool AccountManager::removeSuspend(const String& pAccountName) {
	auto accountData = _find(pAccountName);
	EXPECTED_BOOL(accountData);
	accountData->mStatus = ResponseID::ALLOWED;
	accountData->mSuspendedUntil = 0;

	EXPECTED_BOOL(_save());
	return true;
}

Data::Account* AccountManager::getAccount(const u32 pAccountID) const {
	auto accountData = _find(pAccountID);
	EXPECTED_PTR(accountData);
	return accountData;
}

bool AccountManager::checkOwnership(const u32 pAccountID, const String& pCharacterName) {
	auto accountData = _find(pAccountID);
	EXPECTED_BOOL(accountData);
	for (auto i : accountData->mCharacterData) {
		if (i->mName == pCharacterName)
			return true;
	}

	return false;
}

const bool AccountManager::ensureAccountLoaded(const u32 pAccountID) {
	auto accountData = _find(pAccountID);
	EXPECTED_BOOL(accountData);

	if (accountData->mCharacterDataLoaded) return true;

	EXPECTED_BOOL(DataStore::getInstance().loadAccountCharacterData(accountData));
	accountData->mCharacterDataLoaded = true;
	return true;
}

bool AccountManager::handleCharacterCreate(const u32 pAccountID, const String& pCharacterName, Payload::World::CreateCharacter* pPayload) {
	EXPECTED_BOOL(pPayload);

	// Check: Class ID
	EXPECTED_BOOL(Limits::Character::classID(pPayload->mClass));

	// Check: Race ID
	EXPECTED_BOOL(Limits::Character::raceID(pPayload->mRace));

	// TODO: Check Race is unlocked for the account.

	// Find Account that is creating the Character.
	auto accountData = _find(pAccountID);
	EXPECTED_BOOL(accountData);
	EXPECTED_BOOL(isCharacterNameUnique(pCharacterName));

	// Create CharacterData for the new Character.

	auto characterData = new Data::Character();
	characterData->mName = pCharacterName;
	characterData->mClass = pPayload->mClass;
	characterData->mZoneID = 1; // TODO:

	// Appearance Data
	characterData->mRace = pPayload->mRace; // TODO: Sanity
	characterData->mGender = pPayload->mGender; // Sanity 0/1
	characterData->mFaceStyle = pPayload->mFaceStyle;
	characterData->mHairStyle = pPayload->mHairStyle;
	characterData->mBeardStyle = pPayload->mBeardStyle;
	characterData->mHairColour = pPayload->mHairColour;
	characterData->mBeardColour = pPayload->mBeardColour;
	characterData->mEyeColourLeft = pPayload->mEyeColour1;
	characterData->mEyeColourRight = pPayload->mEyeColour2;
	characterData->mDrakkinHeritage = pPayload->mDrakkinHeritage;
	characterData->mDrakkinTattoo = pPayload->mDrakkinTattoo;
	characterData->mDrakkinDetails = pPayload->mDrakkinDetails;
	characterData->mDeity = pPayload->mDeity; // TODO: Sanity

	EXPECTED_BOOL(DataStore::getInstance().saveCharacter(pCharacterName, characterData));

	// Create Account::CharacterData for the new Character.
	
	auto accountCharacterData = new Data::Account::CharacterData();
	accountCharacterData->mName = pCharacterName;
	accountCharacterData->mLevel = characterData->mLevel;
	accountCharacterData->mRace = characterData->mRace;
	accountCharacterData->mClass = characterData->mClass;
	accountCharacterData->mDeity = characterData->mDeity;
	accountCharacterData->mZoneID = characterData->mZoneID;
	accountCharacterData->mGender = characterData->mGender;
	accountCharacterData->mFaceStyle = characterData->mFaceStyle;
	accountCharacterData->mHairStyle = characterData->mHairStyle;
	accountCharacterData->mHairColour = characterData->mHairColour;
	accountCharacterData->mBeardStyle = characterData->mBeardStyle;
	accountCharacterData->mBeardColour = characterData->mBeardColour;
	accountCharacterData->mEyeColourLeft = characterData->mEyeColourLeft;
	accountCharacterData->mEyeColourRight = characterData->mEyeColourRight;
	accountCharacterData->mDrakkinHeritage = characterData->mDrakkinHeritage;
	accountCharacterData->mDrakkinTattoo = characterData->mDrakkinTattoo;
	accountCharacterData->mDrakkinDetails = characterData->mDrakkinDetails;
	accountCharacterData->mPrimary = 0; // TODO: Items
	accountCharacterData->mSecondary = 0; // TODO: Items

	// TODO: Materials

	accountData->mCharacterData.push_back(accountCharacterData);
	EXPECTED_BOOL(_save(accountData));
	return true;
}

const bool AccountManager::updateCharacter(const u32 pAccountID, const Character* pCharacter) {
	EXPECTED_BOOL(pCharacter);
	auto accountData = _find(pAccountID);
	EXPECTED_BOOL(accountData);
	EXPECTED_BOOL(accountData->mCharacterDataLoaded);

	Data::Account::CharacterData* accountCharacterData = nullptr;
	for (auto i : accountData->mCharacterData) {
		if (i->mName == pCharacter->getName()) {
			accountCharacterData = i;
			break;
		}
	}
	EXPECTED_BOOL(accountCharacterData);
	EXPECTED_BOOL(pCharacter->getZone());

	accountCharacterData->mName = pCharacter->getName();
	accountCharacterData->mLevel = pCharacter->getLevel();
	accountCharacterData->mClass = pCharacter->getClass();
	accountCharacterData->mRace = pCharacter->getRace();
	accountCharacterData->mGender = pCharacter->getGender();
	accountCharacterData->mDeity = pCharacter->getDeity();
	accountCharacterData->mZoneID = pCharacter->getZone()->getID();
	accountCharacterData->mFaceStyle = pCharacter->getFaceStyle();
	accountCharacterData->mHairStyle = pCharacter->getHairStyle();
	accountCharacterData->mHairColour = pCharacter->getHairColour();
	accountCharacterData->mBeardStyle = pCharacter->getBeardStyle();
	accountCharacterData->mBeardColour = pCharacter->getBeardColour();
	accountCharacterData->mEyeColourLeft = pCharacter->getLeftEyeColour();
	accountCharacterData->mEyeColourRight = pCharacter->getRightEyeColour();
	accountCharacterData->mDrakkinHeritage = pCharacter->getDrakkinHeritage();
	accountCharacterData->mDrakkinTattoo = pCharacter->getDrakkinTattoo();
	accountCharacterData->mDrakkinDetails = pCharacter->getDrakkinDetails();
	accountCharacterData->mPrimary = 0; // TODO: Items
	accountCharacterData->mSecondary = 0; // TODO: Items

	// TODO: Materials

	EXPECTED_BOOL(_save(accountData));

	return true;
}

const u32 AccountManager::getNumCharacters(const u32 pAccountID) const {
	auto account = _find(pAccountID);
	if (!account) {
		Log::error("Failed to find account with id " + std::to_string(pAccountID));
		return 0;
	}
	return account->mCharacterData.size();
}

const i32 AccountManager::getSharedPlatinum(const u32 pAccountID) const {
	auto account = _find(pAccountID);
	if (!account) {
		Log::error("Failed to find account with id " + std::to_string(pAccountID));
		return 0;
	}
	return account->mPlatinumSharedBank;
}

const bool AccountManager::setSharedPlatinum(const u32 pAccountID, const i32 pPlatinum) {
	auto account = _find(pAccountID);
	EXPECTED_BOOL(account);
	account->mPlatinumSharedBank = pPlatinum;

	return true;
}
