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
#include "Bonuses.h"
#include "ExperienceController.h"
#include "Zone.h"


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

	mLog->info(Poco::format("Creating Character, Account ID = %u", pAccount->getAccountID()));

	// Check: Does this Account have a name reserved?
	if (!pAccount->hasReservedCharacterName()) {
		mLog->error("Failed to create Character, no reserved name.");
		return false;
	}

	// Check: Class ID is valid.
	if (!Limits::Character::classID(pPayload->mClass)){
		mLog->error(Poco::format("Failed to create Character, class (%u) not valid.", pPayload->mClass));
		return false;
	}

	// Check: Race ID is valid.
	if (!Limits::Character::raceID(pPayload->mRace)) {
		mLog->error(Poco::format("Failed to create Character, race (%u) not valid.", pPayload->mRace));
		return false;
	}

	// Check: Gender ID is valid.
	if (!Limits::Character::genderID(pPayload->mGender)) {
		mLog->error(Poco::format("Failed to create Character, gender (%u) not valid.", pPayload->mGender));
		return false;
	}

	// Check: Deity ID is valid.
	if (!Limits::Character::deityID(pPayload->mDeity)) {
		mLog->error(Poco::format("Failed to create Character, deity (%u) not valid.", pPayload->mDeity));
		return false;
	}
	
	// TODO: Validate Character appearance details. Not sure how I am going to be able to determine what is and is not valid :/

	// TODO: Check Race is unlocked for the account.

	// Retrieve and clear reserved Character name.
	String characterName = pAccount->getReservedCharacterName();
	pAccount->clearReservedCharacterName();

	auto c = std::make_shared<Character>();
	auto e = new Experience::Controller();
	c->setAccount(pAccount);
	c->setExperienceController(e);

	c->setName(characterName);
	c->setClass(static_cast<u8>(pPayload->mClass));
	//c->setZoneID();
	c->setDeityID(pPayload->mDeity);

	
	if (!e->initialise(1, 10, 0, 0, 0, 5000, 0, 1000, 0)) {
	}
	

	//c->setLevel()

	//c->getExperienceController()->set
	//c->setLevel(1); // TODO: Default level.
	//c->getExperienceController()->set

	// Appearance.
	c->setRace(pPayload->mRace);
	c->setGender(pPayload->mGender);
	c->setFaceStyle(static_cast<u8>(pPayload->mFaceStyle));
	c->setHairStyle(static_cast<u8>(pPayload->mHairStyle));
	c->setBeardStyle(static_cast<u8>(pPayload->mBeardStyle));
	c->setHairColour(static_cast<u8>(pPayload->mHairColour));
	c->setBeardColour(static_cast<u8>(pPayload->mBeardColour));
	c->setLeftEyeColour(static_cast<u8>(pPayload->mEyeColour1));
	c->setRightEyeColour(static_cast<u8>(pPayload->mEyeColour2));
	c->setDrakkinHeritage(pPayload->mDrakkinHeritage);
	c->setDrakkinTattoo(pPayload->mDrakkinTattoo);
	c->setDrakkinDetails(pPayload->mDrakkinDetails);

	// Base Stats.
	//auto bonuses = c->getBaseBonuses();
	//bonuses->_addStrength(0);

	// Misc.
	c->setSkill(Skills::Swimming, 200);
	c->setLanguage(LanguageID::CommonTongue, 200);

	const auto characterID = mDataStore->characterCreate(c.get());

	if (characterID == 0) {
		return false;
	}

	return true;
}

const bool AccountManager::deleteCharacter(SharedPtr<Account> pAccount, const String& pCharacterName) {
	if (!pAccount) return false;
	//if (!pAccount->ownsCharacter(pCharacterName)) return false;

	//// Remove from Account.
	//auto data = pAccount->getData();
	//for (auto i : data->mCharacterData) {
	//	if (i->mName == pCharacterName) {
	//		delete i;
	//		data->mCharacterData.remove(i);
	//		break;
	//	}
	//}

	////return _save(data);
	return true;
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
	bool result = true;
	if (!mDataStore->isCharacterNameInUse(pCharacterName, result)) {
		mLog->error("Failed to check if Character name in use");

		// NOTE: We return true, indicating that the Character name is in use even though that may not be true.
		// This is prevent any chance of a duplicate Character name getting into the DB.
		return true;
	}

	return result;
}

const bool AccountManager::_isCharacterNameReserved(const String& pCharacterName) const {
	for (auto i : mAccounts) {
		if (i->getReservedCharacterName() == pCharacterName)
			return true;
	}

	return false;
}

const i8 AccountManager::onConnectRequest(const u32 pLSAccountID, const u32 pLSID, const bool pWorldLocked) {

	const auto accountCreated = isCreated(pLSAccountID, pLSID);

	// Server is currently locked and the connecting Account is unknown, deny connection.
	if (pWorldLocked && !accountCreated) {
		mLog->info("Unknown Account attempted connection to locked server, denying connection.");
		return ResponseID::Denied;
	}
	// Server is not currently locked and the connecting Account is not known, allow connection.
	else if (!pWorldLocked && !accountCreated) {
		mLog->info("Unknown Account connecting to server, allowing connection.");
		return ResponseID::Allowed;
	}

	const auto connected = isConnected(pLSAccountID, pLSID);

	// Check: Account is already connected.
	if (connected) {
		mLog->info("Account is already connected, denying connection.");
		return ResponseID::Full;
	}

	auto account = load(pLSAccountID, pLSID);

	// Account failed to load, deny connection.
	if (!account) {
		mLog->error("Failed to load Account, denying connection.");
		return ResponseID::Denied;
	}

	// Check: Suspended Account time may have expired.
	if (account->isSuspended()) {
		checkSuspension(account);
	}
	// Account is suspended.
	if (account->isSuspended()) {
		mLog->info("Suspended Account, denying connection.");
		return ResponseID::Suspended;
	}

	// Account is banned.
	if (account->isBanned()) {
		mLog->info("Banned Account, denying connection.");
		return ResponseID::Banned;
	}

	// Server is currently locked.
	if (pWorldLocked) {
		// Account has enough status to bypass lock.
		if (account->getStatus() >= AccountStatus::BypassLock) {
			mLog->info("World locked but Account has a high enough status to bypass lock, allowing connection.");
			return ResponseID::Allowed;
		}
		// Account does not have enough status to bypass lock.
		else {
			mLog->info("World locked but Account does not have high enough status to bypass lock, denying connect.");
			return ResponseID::Denied;
		}
	}
	
	return ResponseID::Allowed;
}

const bool AccountManager::onConnect(SharedPtr<Account> pAccount) {
	if (!pAccount) return false;

	// Configure Account session.
	pAccount->setSessionBeginTime(Poco::DateTime());
	const auto sessionID = mDataStore->accountConnect(pAccount.get());
	if (sessionID == 0) {
		// This should never occur.
		return false;
	}
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
