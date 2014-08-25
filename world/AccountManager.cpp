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
	accountData->mCreated = 0; // TODO: now
	accountData->mStatus = ResponseID::ALLOWED;

	mAccounts.push_back(accountData);
	_save();
	_save(accountData);
}

void AccountManager::_clear() {
	mAccounts.remove_if(Utility::containerEntryDelete<AccountData*>);
}
bool AccountManager::_save() {
	EXPECTED_BOOL(DataStore::getInstance().saveAccounts(mAccounts));
}

bool AccountManager::_save(AccountData* pAccountData) {
	EXPECTED_BOOL(pAccountData);
	EXPECTED_BOOL(DataStore::getInstance().saveAccountCharacterData(pAccountData));
}

bool AccountManager::_loadAccount(const String& pAccountName) {
	//EXPECTED_BOOL(exists(pAccountName));

	return true;
}

bool AccountManager::exists(const uint32 pAccountID){
	AccountData* account = _find(pAccountID);
	return account ? true : false;
	//return PathFileExists(String("./data/accounts/" + pAccountName + ".xml").c_str());
}

AccountData* AccountManager::_load(const String& pAccountName) {
	return nullptr;
}

AccountStatus AccountManager::getStatus(const uint32 pAccountID) {
	AccountData* account = _find(pAccountID);
	return account ? account->mStatus : ResponseID::ALLOWED;
}

bool AccountManager::deleteCharacter(const String& pCharacterName) {
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

bool AccountManager::handleCharacterCreate(uint32 pAccountID, const String& pCharacterName, Payload::World::CreateCharacter* pPayload)
{
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
	accountCharacterData->mRace = pCharacter->getRace();
	accountCharacterData->mGender = pCharacter->getGender();
	accountCharacterData->mDeity = pCharacter->getDeity();
	accountCharacterData->mZoneID = pCharacter->getZone()->getID();
	accountCharacterData->mFace = pCharacter->getFaceStyle();
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
}
