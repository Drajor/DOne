#include "AccountManager.h"
#include "ServiceLocator.h"
#include "IDataStore.h"
#include "Account.h"
#include "Data.h"
#include "LogSystem.h"
#include "Utility.h"
#include "Profile.h"
#include "Payload.h"
#include "Limits.h"

#include "Character.h"
#include "Zone.h"

#include "Shlwapi.h"

AccountManager::~AccountManager() {
	if (mInitialised == false) return;

	_save();
	_clear();
}

const bool AccountManager::initialise(IDataStore* pDataStore, ILogFactory* pLogFactory) {
	if (mInitialised) return false;
	if (!pLogFactory) return false;
	if (!pDataStore) return false;

	mLogFactory = pLogFactory;
	mLog = mLogFactory->make();
	mDataStore = pDataStore;
	mLog->setContext("[AccountManager]");

	mLog->status("Initialising.");

	// Load Account Data.
	std::list<Data::Account*> accountData;
	if (!mDataStore->loadAccounts(accountData)) {
		mLog->error("Failed to load accounts!");
		_clear();
		return false;
	}
	
	// Create Accounts.
	for (auto i : accountData) {
		auto account = new Account(i, mLogFactory->make());
		mAccounts.push_back(account);
	}

	mLog->info("Loaded data for " + toString(numAccounts()) + " Accounts.");

	mLog->status("Initialisation complete.");
	mInitialised = true;
	return true;
}

Account* AccountManager::createAccount(const u32 pLoginAccountID, const String pLoginAccountName, const u32 pLoginServerID) {
	// Check: Account does not already exist.
	if (exists(pLoginAccountID, pLoginAccountName, pLoginServerID)) {
		mLog->error("Attempting to create Account that already exists.");
		return nullptr;
	}

	auto data = new Data::Account();
	data->mLoginAccountID = pLoginAccountID;
	data->mLoginAccountName = pLoginAccountName;
	data->mLoginServerID = pLoginServerID;
	data->mCreated = Utility::Time::now();
	data->mStatus = AccountStatus::Default;
	
	EXPECTED_BOOL(_save());
	EXPECTED_BOOL(_save(data));

	Account* account = new Account(data, mLogFactory->make());
	mAccounts.push_back(account);

	return account;
}

Account* AccountManager::getAccount(const u32 pLoginAccountID, const u32 pLoginServerID) {
	return _find(pLoginAccountID, pLoginServerID);
}

void AccountManager::_clear() {
	for (auto i : mAccounts)
		delete i;
	mAccounts.clear();
}

bool AccountManager::_save() {
	// This a a kludge for now :(
	std::list<Data::Account*> accounts;
	for (auto i : mAccounts) {
		accounts.push_back(i->getData());
	}

	if (!mDataStore->saveAccounts(accounts)) {
		mLog->error("Failed to save Account Data");
		return false;
	}

	return true;
}

bool AccountManager::_save(Data::Account* pAccountData) {
	if (!pAccountData) return false;
	
	if (!mDataStore->saveAccountCharacterData(pAccountData)) {
		mLog->error("Failed to save Account Character Data.");
		return false;
	}

	return true;
}

const bool AccountManager::exists(const u32 pLoginAccountID, const String& pLoginAccountName, const u32 pLoginServerID) const {
	auto account = _find(pLoginAccountID, pLoginAccountName, pLoginServerID);
	return account ? true : false;
}

const i8 AccountManager::getStatus(const u32 pLoginAccountID, const u32 pLoginServerID) {
	auto account = _find(pLoginAccountID, pLoginServerID);
	return account ? account->getStatus() : AccountStatus::Default;
}

const bool AccountManager::createCharacter(Account* pAccount, Payload::World::CreateCharacter* pPayload) {
	//EXPECTED_BOOL(pPayload);

	//// Check: Class ID
	//EXPECTED_BOOL(Limits::Character::classID(pPayload->mClass));

	//// Check: Race ID
	//EXPECTED_BOOL(Limits::Character::raceID(pPayload->mRace));

	//// TODO: Check Race is unlocked for the account.

	//// Find Account that is creating the Character.
	//auto accountData = _find(pAccountID);
	//EXPECTED_BOOL(accountData);
	//EXPECTED_BOOL(isCharacterNameUnique(pCharacterName));

	//// Create CharacterData for the new Character.

	//auto characterData = new Data::Character();
	//characterData->mName = pCharacterName;
	//characterData->mClass = pPayload->mClass;
	//characterData->mZoneID = 1; // TODO:

	//// Appearance Data
	//characterData->mRace = pPayload->mRace; // TODO: Sanity
	//characterData->mGender = pPayload->mGender; // Sanity 0/1
	//characterData->mFaceStyle = pPayload->mFaceStyle;
	//characterData->mHairStyle = pPayload->mHairStyle;
	//characterData->mBeardStyle = pPayload->mBeardStyle;
	//characterData->mHairColour = pPayload->mHairColour;
	//characterData->mBeardColour = pPayload->mBeardColour;
	//characterData->mEyeColourLeft = pPayload->mEyeColour1;
	//characterData->mEyeColourRight = pPayload->mEyeColour2;
	//characterData->mDrakkinHeritage = pPayload->mDrakkinHeritage;
	//characterData->mDrakkinTattoo = pPayload->mDrakkinTattoo;
	//characterData->mDrakkinDetails = pPayload->mDrakkinDetails;
	//characterData->mDeity = pPayload->mDeity; // TODO: Sanity

	//EXPECTED_BOOL(mDataStore->saveCharacter(pCharacterName, characterData));

	//// Create Account::CharacterData for the new Character.
	//
	//auto accountCharacterData = new Data::Account::CharacterData();
	//accountCharacterData->mName = pCharacterName;
	//accountCharacterData->mLevel = characterData->mExperience.mLevel;
	//accountCharacterData->mRace = characterData->mRace;
	//accountCharacterData->mClass = characterData->mClass;
	//accountCharacterData->mDeity = characterData->mDeity;
	//accountCharacterData->mZoneID = characterData->mZoneID;
	//accountCharacterData->mGender = characterData->mGender;
	//accountCharacterData->mFaceStyle = characterData->mFaceStyle;
	//accountCharacterData->mHairStyle = characterData->mHairStyle;
	//accountCharacterData->mHairColour = characterData->mHairColour;
	//accountCharacterData->mBeardStyle = characterData->mBeardStyle;
	//accountCharacterData->mBeardColour = characterData->mBeardColour;
	//accountCharacterData->mEyeColourLeft = characterData->mEyeColourLeft;
	//accountCharacterData->mEyeColourRight = characterData->mEyeColourRight;
	//accountCharacterData->mDrakkinHeritage = characterData->mDrakkinHeritage;
	//accountCharacterData->mDrakkinTattoo = characterData->mDrakkinTattoo;
	//accountCharacterData->mDrakkinDetails = characterData->mDrakkinDetails;
	//accountCharacterData->mPrimary = 0; // TODO: Items
	//accountCharacterData->mSecondary = 0; // TODO: Items

	//// TODO: Materials

	//accountData->mCharacterData.push_back(accountCharacterData);
	//EXPECTED_BOOL(_save(accountData));
	return true;
}

const bool AccountManager::deleteCharacter(Account* pAccount, const String& pCharacterName) {
	if (!pAccount) return false;
	if (!pAccount->ownsCharacter(pCharacterName)) return false;

	//auto account = _find(pAccountID, 0); // TODO
	//EXPECTED_BOOL(account);
	//EXPECTED_BOOL(mDataStore->deleteCharacter(pCharacterName));

	//// Remove from AccountData
	//bool removed = false;
	//for (auto i : account->mCharacterData) {
	//	if (i->mName == pCharacterName) {
	//		delete i;
	//		account->mCharacterData.remove(i);
	//		removed = true;
	//		break;
	//	}
	//}
	//EXPECTED_BOOL(removed); // Check: Sanity.
	//EXPECTED_BOOL(_save(account)); // Save.

	return true;
}

const bool AccountManager::updateCharacter(Account* pAccount, const Character* pCharacter) {
	if (!pAccount) return false;
	if (!pAccount->isLoaded()) return false;
	if (!pCharacter) return false;
	if (!pAccount->ownsCharacter(pCharacter->getName())) return false;

	// Check: Data exists.
	auto data = pAccount->getData(pCharacter->getName());
	if (!data) {
		mLog->error("Failure: Failed to find Data::CharacterData when updating Character");
		return false;
	}

	// Check: Character is in a Zone.
	auto zone = pCharacter->getZone();
	if (!zone) {
		// TODO: When this happens we need to continue with the save and either save a destination zone (if they are zoning) or bind if neither.
		return false;
	}

	data->mName = pCharacter->getName();
	data->mLevel = pCharacter->getLevel();
	data->mClass = pCharacter->getClass();
	data->mRace = pCharacter->getRace();
	data->mGender = pCharacter->getGender();
	data->mDeity = pCharacter->getDeity();
	data->mZoneID = pCharacter->getZone()->getID();
	data->mFaceStyle = pCharacter->getFaceStyle();
	data->mHairStyle = pCharacter->getHairStyle();
	data->mHairColour = pCharacter->getHairColour();
	data->mBeardStyle = pCharacter->getBeardStyle();
	data->mBeardColour = pCharacter->getBeardColour();
	data->mEyeColourLeft = pCharacter->getLeftEyeColour();
	data->mEyeColourRight = pCharacter->getRightEyeColour();
	data->mDrakkinHeritage = pCharacter->getDrakkinHeritage();
	data->mDrakkinTattoo = pCharacter->getDrakkinTattoo();
	data->mDrakkinDetails = pCharacter->getDrakkinDetails();
	data->mPrimary = 0; // TODO: Items
	data->mSecondary = 0; // TODO: Items

	// TODO: Materials

	// Save the Account.
	return _save(pAccount->getData());
}

Account* AccountManager::_find(const u32 pLoginAccountID, const String& pLoginAccountName, const u32 pLoginServerID) const {
	for (auto i : mAccounts) {
		if (i->getLoginAccountID() == pLoginAccountID && i->getLoginAccountName() == pLoginAccountName && i->getLoginServerID() == pLoginServerID)
			return i;
	}

	return nullptr;
}

Account* AccountManager::_find(const u32 pLoginAccountID, const u32 pLoginServerID) const {
	for (auto i : mAccounts) {
		if (i->getLoginAccountID() == pLoginAccountID && i->getLoginServerID() == pLoginServerID)
			return i;
	}

	return nullptr;
}

const bool AccountManager::ban(Account* pAccount) {
	if (!pAccount) return false;

	// Change Account status.
	pAccount->setStatus(AccountStatus::Banned);

	// Save.
	return _save();
}

const bool AccountManager::removeBan(Account* pAccount) {
	if (!pAccount) return false;

	// Change Account status.
	pAccount->setStatus(AccountStatus::Default);

	// Save.
	return _save();
}

const bool AccountManager::suspend(Account* pAccount, const u32 pExpiry) {
	if (!pAccount) return false;

	// Change Account status.
	pAccount->setStatus(AccountStatus::Suspended);
	pAccount->setSuspensionTime(pExpiry);

	// Save.
	return _save();
}

const bool AccountManager::removeSuspension(Account* pAccount) {
	if (!pAccount) return false;

	// Change Account status.
	pAccount->setStatus(AccountStatus::Default);
	pAccount->setSuspensionTime(0);

	// Save.
	return _save();
}

void AccountManager::checkSuspension(Account * pAccount) {
	if (!pAccount) return;
	if (!pAccount->isSuspended()) return;

	// Check: Has the suspension expired?
	if (Utility::Time::now() >= pAccount->getSuspensionTime())
		removeSuspension(pAccount);
}

Account* AccountManager::getAuthenticatedAccount(const u32 pLoginAccountID, const String& pKey, const u32 pIP) {
	auto f = [pLoginAccountID, pKey, pIP](const Account* pAccount) {
		return pAccount->getLoginAccountID() == pLoginAccountID && pAccount->getKey() == pKey && pAccount->getIP() == pIP;
	};
	auto i = std::find_if(mAccounts.begin(), mAccounts.end(), f);
	return i == mAccounts.end() ? nullptr : *i;
}

const bool AccountManager::isCharacterNameAllowed(const String& pCharacterName) const {
	// Check: Length
	if (!Limits::Character::nameInputLength(pCharacterName)) return false;

	// Check: Case of first character (must be uppercase)
	if (pCharacterName[0] < 'A' || pCharacterName[0] > 'Z') return false;

	// Check: Each character is alpha.
	for (String::size_type i = 0; i < pCharacterName.length(); i++) {
		if (!isalpha(pCharacterName[i])) {
			return false;
		}
	}

	// Check: Name already in use.
	if (_isCharacterNameInUse(pCharacterName)) return false;

	// Check: Name is reserved.
	if (_isCharacterNameReserved(pCharacterName)) return false;

	return true;
}

const bool AccountManager::_isCharacterNameInUse(const String& pCharacterName) const {
	return false;
}

const bool AccountManager::_isCharacterNameReserved(const String& pCharacterName) const {
	for (auto i : mAccounts) {
		if (i->getReservedCharacterName() == pCharacterName)
			return true;
	}

	return false;
}

const bool AccountManager::onConnect(Account* pAccount) {
	if (!pAccount) return false;
	if (pAccount->isLoaded()) return false; // Already loaded.

	return _loadAccount(pAccount);
}

const bool AccountManager::onDisconnect(Account* pAccount) {
	if (!pAccount) return false;
	if (!pAccount->isLoaded()) return true;

	auto character = pAccount->getActiveCharacter();
	if (character) {
		updateCharacter(pAccount, character);
		pAccount->clearActiveCharacter();
	}

	_unloadAccount(pAccount);

	pAccount->clearAuthentication();

	return _save();
}

const bool AccountManager::_loadAccount(Account* pAccount) {
	if (!pAccount) return false;

	if (pAccount->isLoaded()) {
		mLog->error("Failure: Attempt to load Account that is already loaded.");
		return false;
	}
	
	if (!mDataStore->loadAccountCharacterData(pAccount->getData())) {
		mLog->error("Failure: Unable to load Account Character Data.");
		return false;
	}

	pAccount->setLoaded(true);
	return true;
}

const bool AccountManager::_unloadAccount(Account* pAccount) {
	if (!pAccount) return false;
	
	if (!pAccount->isLoaded()) {
		mLog->error("Failure: Attempt to unload Account that is not loaded.");
		return false;
	}

	auto characterData = pAccount->getData()->mCharacterData;
	for (auto i : characterData) {
		delete i;
	}
	characterData.clear();

	pAccount->setLoaded(false);
	return true;
}
