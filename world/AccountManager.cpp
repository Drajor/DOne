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
	if (mDataStore)	_save();

	mDataStore = nullptr;
	mLogFactory = nullptr;

	if (mLog) {
		delete mLog;
		mLog = nullptr;
	}

	for (auto i : mAccounts)
		delete i;
	mAccounts.clear();
}

const bool AccountManager::initialise(IDataStore* pDataStore, ILogFactory* pLogFactory) {
	if (mInitialised) return false;

	mLogFactory = pLogFactory;
	mDataStore = pDataStore;

	if (!mDataStore) return false;
	if (!mLogFactory) return false;

	// Create and configure AccountManager log.
	mLog = mLogFactory->make();
	mLog->setContext("[AccountManager]");
	mLog->status("Initialising.");

	// Load Data.
	std::list<Data::Account*> accountData;
	if (!mDataStore->loadAccounts(accountData)) {
		mLog->error("Failed to load accounts!");
		return false;
	}
	
	// Create Accounts.
	for (auto i : accountData) {
		auto account = new Account(i, mLogFactory->make());
		mAccounts.push_back(account);
	}

	mLog->info("Loaded data for " + toString(numAccounts()) + " Accounts.");

	mLog->status("Finished initialising.");
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
	if (!pAccount) return false;
	if (!pPayload) return false;
	if (!pAccount->isLoaded()) return false;
	if (!pAccount->hasReservedCharacterName()) return false;

	// Check: Class ID is valid.
	if (!Limits::Character::classID(pPayload->mClass)) return false;

	// Check: Race ID is valid.
	if (!Limits::Character::raceID(pPayload->mRace)) return false;

	// Check: Gender ID is valid.
	if (!Limits::Character::genderID(pPayload->mGender)) return false;

	// Check: Deity ID is valid.
	if (!Limits::Character::deityID(pPayload->mDeity)) return false;

	// TODO: Check Race is unlocked for the account.

	// Retrieve and clear reserved Character name.
	String characterName = pAccount->getReservedCharacterName();
	pAccount->clearReservedCharacterName();

	auto c = new Data::Character();
	c->mName = characterName;
	c->mClass = pPayload->mClass;
	c->mZoneID = 1; // TODO:

	c->mExperience.mLevel = 1;
	c->mExperience.mMaximumLevel = 10;

	// Appearance Data
	c->mRace = pPayload->mRace;
	c->mGender = pPayload->mGender;
	c->mFaceStyle = pPayload->mFaceStyle;
	c->mHairStyle = pPayload->mHairStyle;
	c->mBeardStyle = pPayload->mBeardStyle;
	c->mHairColour = pPayload->mHairColour;
	c->mBeardColour = pPayload->mBeardColour;
	c->mEyeColourLeft = pPayload->mEyeColour1;
	c->mEyeColourRight = pPayload->mEyeColour2;
	c->mDrakkinHeritage = pPayload->mDrakkinHeritage;
	c->mDrakkinTattoo = pPayload->mDrakkinTattoo;
	c->mDrakkinDetails = pPayload->mDrakkinDetails;
	c->mDeity = pPayload->mDeity;
	c->mNew = true;

	// Save Character.
	if (!mDataStore->saveCharacter(characterName, c)) {
		mLog->error("Failure: Creating Character.");
		delete c;
		return false;
	}

	// Create Data::AccountCharacter for the new Character.
	
	auto accountCharacterData = new Data::AccountCharacter();
	accountCharacterData->mName = characterName;
	accountCharacterData->mLevel = c->mExperience.mLevel;
	accountCharacterData->mRace = c->mRace;
	accountCharacterData->mClass = c->mClass;
	accountCharacterData->mDeity = c->mDeity;
	accountCharacterData->mZoneID = c->mZoneID;
	accountCharacterData->mGender = c->mGender;
	accountCharacterData->mFaceStyle = c->mFaceStyle;
	accountCharacterData->mHairStyle = c->mHairStyle;
	accountCharacterData->mHairColour = c->mHairColour;
	accountCharacterData->mBeardStyle = c->mBeardStyle;
	accountCharacterData->mBeardColour = c->mBeardColour;
	accountCharacterData->mEyeColourLeft = c->mEyeColourLeft;
	accountCharacterData->mEyeColourRight = c->mEyeColourRight;
	accountCharacterData->mDrakkinHeritage = c->mDrakkinHeritage;
	accountCharacterData->mDrakkinTattoo = c->mDrakkinTattoo;
	accountCharacterData->mDrakkinDetails = c->mDrakkinDetails;
	accountCharacterData->mPrimary = 0;
	accountCharacterData->mSecondary = 0;

	// Add to Account.
	auto data = pAccount->getData();
	data->mCharacterData.push_back(accountCharacterData);

	// Save.
	if (!_save(data)){
		mLog->error("Save failed in " + String(__FUNCTION__));

		// Clean up.
		data->mCharacterData.remove(accountCharacterData);
		delete accountCharacterData;
		delete c;

		return false;
	}

	return true;
}

const bool AccountManager::deleteCharacter(Account* pAccount, const String& pCharacterName) {
	if (!pAccount) return false;
	if (!pAccount->isLoaded()) return false;
	if (!pAccount->ownsCharacter(pCharacterName)) return false;

	// Remove from Account.
	auto data = pAccount->getData();
	for (auto i : data->mCharacterData) {
		if (i->mName == pCharacterName) {
			delete i;
			data->mCharacterData.remove(i);
			break;
		}
	}

	return _save(data);
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

	auto& characterData = pAccount->getData()->mCharacterData;
	for (auto i : characterData) {
		delete i;
	}
	characterData.clear();

	pAccount->setLoaded(false);
	return true;
}
