#include "AccountManager.h"
#include "DataStore.h"
#include "Data.h"
#include "LogSystem.h"
#include "Utility.h"
#include "Profile.h"
#include "Payload.h"

#include "Character.h"
#include "Zone.h"

#include "Shlwapi.h"

static bool deleteAll(AccountData* pValue) { delete pValue; return true; };
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

bool AccountManager::createAccount(const uint32 pAccountID, const String pAccoutName){
	EXPECTED_BOOL(exists(pAccountID) == false);

	AccountData* accountData = new AccountData();
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
	mAccounts.remove_if(Utility::containerEntryDelete<AccountData*>);
}
bool AccountManager::_save() {
	EXPECTED_BOOL(DataStore::getInstance().saveAccounts(mAccounts));

	return true;
}

bool AccountManager::_save(AccountData* pAccountData) {
	EXPECTED_BOOL(pAccountData);
	EXPECTED_BOOL(DataStore::getInstance().saveAccountCharacterData(pAccountData));

	return true;
}

bool AccountManager::_loadAccount(const String& pAccountName) {
	//EXPECTED_BOOL(exists(pAccountName));

	return true;
}

bool AccountManager::exists(const uint32 pAccountID){
	AccountData* account = _find(pAccountID);
	return account ? true : false;
}

AccountData* AccountManager::_load(const String& pAccountName) {
	return nullptr;
}

AccountStatus AccountManager::getStatus(const uint32 pAccountID) {
	auto account = _find(pAccountID);
	return account ? account->mStatus : ResponseID::ALLOWED;
}

bool AccountManager::deleteCharacter(const uint32 pAccountID, const String& pCharacterName) {
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

AccountData* AccountManager::_find(const uint32 pAccountID) {
	for (auto i : mAccounts) {
		if (i->mAccountID == pAccountID)
			return i;
	}

	return nullptr;
}

AccountData* AccountManager::_find(const String& pAccountName) {
	for (auto i : mAccounts) {
		if (i->mAccountName == pAccountName)
			return i;
	}

	return nullptr;
}

bool AccountManager::ban(const String& pAccountName) {
	AccountData* accountData = _find(pAccountName);
	EXPECTED_BOOL(accountData);
	accountData->mStatus = ResponseID::BANNED;

	_save();
	return true;
}

bool AccountManager::removeBan(const String& pAccountName) {
	AccountData* accountData = _find(pAccountName);
	EXPECTED_BOOL(accountData);
	accountData->mStatus = ResponseID::ALLOWED;

	_save();
	return true;
}

bool AccountManager::suspend(const String& pAccountName, const uint32 pSuspendUntil) {
	AccountData* accountData = _find(pAccountName);
	EXPECTED_BOOL(accountData);
	accountData->mStatus = ResponseID::SUSPENDED;
	accountData->mSuspendedUntil = pSuspendUntil;

	_save();
	return true;
}

bool AccountManager::removeSuspend(const String& pAccountName) {
	AccountData* accountData = _find(pAccountName);
	EXPECTED_BOOL(accountData);
	accountData->mStatus = ResponseID::ALLOWED;
	accountData->mSuspendedUntil = 0;

	_save();
	return true;
}

AccountData* AccountManager::getAccount(uint32 pAccountID) {
	AccountData* accountData = _find(pAccountID);
	EXPECTED_PTR(accountData);
	return accountData;
}

bool AccountManager::checkOwnership(const uint32 pAccountID, const String& pCharacterName) {
	AccountData* accountData = _find(pAccountID);
	EXPECTED_BOOL(accountData);
	for (auto i : accountData->mCharacterData) {
		if (i->mName == pCharacterName)
			return true;
	}

	return false;
}

void AccountManager::ensureAccountLoaded(const uint32 pAccountID) {
	AccountData* accountData = _find(pAccountID);
	EXPECTED(accountData);

	if (accountData->mCharacterDataLoaded) return;

	EXPECTED(DataStore::getInstance().loadAccountCharacterData(accountData));
	accountData->mCharacterDataLoaded = true;
}

bool AccountManager::handleCharacterCreate(const uint32 pAccountID, const String& pCharacterName, Payload::World::CreateCharacter* pPayload) {
	EXPECTED_BOOL(pPayload);

	// Find Account that is creating the Character.
	auto accountData = _find(pAccountID);
	EXPECTED_BOOL(accountData);
	EXPECTED_BOOL(isCharacterNameUnique(pCharacterName));

	// Create CharacterData for the new Character.

	auto characterData = new CharacterData();
	characterData->mName = pCharacterName;
	characterData->mClass = pPayload->mClass; // TODO: Sanity check
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
	
	auto accountCharacterData = new AccountData::CharacterData();
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

const bool AccountManager::updateCharacter(const uint32 pAccountID, const Character* pCharacter) {
	EXPECTED_BOOL(pCharacter);
	AccountData* accountData = _find(pAccountID);
	EXPECTED_BOOL(accountData);
	EXPECTED_BOOL(accountData->mCharacterDataLoaded);

	AccountData::CharacterData* accountCharacterData = nullptr;
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
	accountCharacterData->mRace = pCharacter->getRaceID();
	accountCharacterData->mGender = pCharacter->getGender();
	accountCharacterData->mDeity = pCharacter->getDeityID();
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
