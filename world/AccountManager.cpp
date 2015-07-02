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

#include "Poco/Format.h"
#include "Poco/DateTimeFormatter.h"

#include "Character.h"
#include "Zone.h"



#include "Shlwapi.h"

AccountManager::~AccountManager() {
	mAccounts.clear();
	
	mDataStore = nullptr;
	mLogFactory = nullptr;

	if (mLog) {
		delete mLog;
		mLog = nullptr;
	}
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

	mLog->status("Finished initialising.");
	mInitialised = true;
	return true;
}

SharedPtr<Account> AccountManager::create(const u32 pLSAccountID, const String& pLSAccountName, const u32 pLSID) {
	mLog->info(Poco::format("Creating Account, Login Account ID = %u, Login Account Name = %s, Login Server ID = %u", pLSAccountID, pLSAccountName, pLSID));

	// Check: Account does not already exist.
	if (isCreated(pLSAccountID, pLSID)) {
		mLog->error("Account already exists.");
		return nullptr;
	}

	// Create Account.
	auto accountID = mDataStore->accountCreate(pLSAccountID, pLSAccountName, pLSID, AccountStatus::Default);

	// Check: Account was not created.
	if (accountID == 0) {
		mLog->error("Account creation failed.");
		return nullptr;
	}

	return load(pLSAccountID, pLSID);
}

SharedPtr<Account> AccountManager::load(const u32 pLSAccountID, const u32 pLSID) {
	mLog->info(Poco::format("Loading Account, Login Server Account ID = %u, Login Server ID = %u", pLSAccountID, pLSID));

	auto account = std::make_shared<Account>(this);
	const auto success = mDataStore->accountLoad(account.get(), pLSAccountID, pLSID);

	// This is called because Account object has just had all variables set which make it appear as if needs saving when it doesn't.
	account->saved();

	// Check:
	if (!success) {
		mLog->error("Failed to load Account!");
		account = nullptr;
		return nullptr;
	}

	mLog->info("Successfully loaded Account.");
	return account;
}

const bool AccountManager::_save(Account* pAccount) {
	if (!pAccount) return false;

	const auto success = mDataStore->accountSave(pAccount);

	if (!success) {
		mLog->error("Failed to save Account, ID = " + toString(pAccount->getAccountID()));
		return false;
	}

	pAccount->saved();
	return true;
}

const bool AccountManager::isCreated(const u32 pLSAccountID, const u32 pLSID) {
	return mDataStore->accountExists(pLSAccountID, pLSID);
}

const bool AccountManager::isConnected(const u32 pLSAccountID, const u32 pLSID) {
	auto account = _find(pLSAccountID, pLSID);
	return account ? true : false;
}

const bool AccountManager::createCharacter(SharedPtr<Account> pAccount, Payload::World::CreateCharacter* pPayload) {
	if (!pAccount) return false;
	if (!pPayload) return false;
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

	//// Save.
	//if (!_save(data)){
	//	mLog->error("Save failed in " + String(__FUNCTION__));

	//	// Clean up.
	//	data->mCharacterData.remove(accountCharacterData);
	//	delete accountCharacterData;
	//	delete c;

	//	return false;
	//}

	return true;
}

const bool AccountManager::deleteCharacter(SharedPtr<Account> pAccount, const String& pCharacterName) {
	if (!pAccount) return false;
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

	//return _save(data);
	return true;
}

const bool AccountManager::updateCharacter(SharedPtr<Account> pAccount, const Character* pCharacter) {
	if (!pAccount) return false;
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
	//return _save(pAccount->getData());
	return true;
}

SharedPtr<Account> AccountManager::_find(const u32 pLSAccountID, const String& pLSAccountName, const u32 pLSID) const {
	for (auto i : mAccounts) {
		if (i->getLSAccountID() == pLSAccountID && i->getLSAccountName() == pLSAccountName && i->getLSID() == pLSID)
			return i;
	}

	return nullptr;
}

SharedPtr<Account> AccountManager::_find(const u32 pLSAccountID, const u32 pLSID) const {
	for (auto i : mAccounts) {
		if (i->getLSAccountID() == pLSAccountID && i->getLSID() == pLSID)
			return i;
	}

	return nullptr;
}

const bool AccountManager::setStatus(SharedPtr<Account> pAccount, const i32 pStatus) {
	if (!pAccount) return false;

	mLog->info(Poco::format("Changing Account status from %d to %d.", pAccount->getStatus(), pStatus));
	pAccount->setStatus(pStatus);

	return true;
}

const bool AccountManager::ban(SharedPtr<Account> pAccount) {
	if (!pAccount) return false;

	mLog->info(Poco::format("Banning Account ID = %u", pAccount->getAccountID()));
	return setStatus(pAccount, AccountStatus::Banned);
}

const bool AccountManager::removeBan(SharedPtr<Account> pAccount) {
	if (!pAccount) return false;

	mLog->info(Poco::format("Unbanning Account ID = %u", pAccount->getAccountID()));
	return setStatus(pAccount, AccountStatus::Default);
}

const bool AccountManager::suspend(SharedPtr<Account> pAccount, const Poco::DateTime pExpiry) {
	if (!pAccount) return false;

	mLog->info(Poco::format("Suspending Account ID = %u, Expiry = %s", pAccount->getAccountID(), Poco::DateTimeFormatter::format(pExpiry, "%e %b %Y %H:%M")));
	pAccount->setSuspensionExpiry(pExpiry);
	return setStatus(pAccount, AccountStatus::Suspended);
}

const bool AccountManager::removeSuspension(SharedPtr<Account> pAccount) {
	if (!pAccount) return false;

	mLog->info(Poco::format("Unsuspending Account ID = %u", pAccount->getAccountID()));
	pAccount->setSuspensionExpiry(Poco::DateTime(0, 1, 1, 0, 0, 0, 0, 0));
	return setStatus(pAccount, AccountStatus::Default);
}

void AccountManager::checkSuspension(SharedPtr<Account> pAccount) {
	if (!pAccount) return;
	if (!pAccount->isSuspended()) return;

	// Check: Has the suspension expired?
	Poco::DateTime now;
	if (now >= pAccount->getSuspensionExpiry())
		removeSuspension(pAccount);
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

const bool AccountManager::onConnect(SharedPtr<Account> pAccount) {
	if (!pAccount) return false;

	// Configure Account session.
	pAccount->setSessionBeginTime(Poco::DateTime());
	const auto sessionID = mDataStore->accountConnect(pAccount.get());
	pAccount->setSessionID(sessionID);

	mAccounts.push_back(pAccount);
	return true;
}

const bool AccountManager::onDisconnect(SharedPtr<Account> pAccount) {
	if (!pAccount) return false;

	auto character = pAccount->getActiveCharacter();
	if (character) {
		//updateCharacter(pAccount, character);
		pAccount->clearActiveCharacter();
	}

	// Clean up Account session.
	pAccount->setSessionEndTime(Poco::DateTime());
	mDataStore->accountDisconnect(pAccount.get());

	// Clean up Account authentication.
	pAccount->clearAuthentication();

	mAccounts.remove(pAccount);
	return true;
}
