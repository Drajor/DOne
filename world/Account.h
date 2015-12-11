#pragma once

#include <list>

#include "Poco/DateTime.h"

#include "Types.h"
#include "Constants.h"
#include "AccountConstants.h"

namespace Data {
	struct Account;
	struct AccountCharacter;
}

class AccountManager;
class Character;



/*

An Account is persistent from the time a client connects to World till when they disconnect.

*/

struct AccountCharacter {
	String mName;
	u32 mLevel = 0;
	u32 mClass = 0;
	u32 mRace = 0;
	u32 mGender = 0;
	u32 mDeity = 0;
	u32 mZoneID = 0;
	u32 mFaceStyle = 0;
	u32 mHairStyle = 0;
	u32 mHairColour = 0;
	u32 mBeardStyle = 0;
	u32 mBeardColour = 0;
	u32 mEyeColourLeft = 0;
	u32 mEyeColourRight = 0;
	u32 mDrakkinHeritage = 0;
	u32 mDrakkinTattoo = 0;
	u32 mDrakkinDetails = 0;
	u32 mPrimary = 0;
	u32 mSecondary = 0;
	u32 mHeadMaterial = 0;
	u32 mHeadColour = 0;

	bool mCanReturnHome = true;
	bool mCanEnterTutorial = false;

	struct Equipment {
		u32 mMaterial = 0;
		u32 mColour = 0;
	};

	Equipment mEquipment[Limits::Account::MAX_EQUIPMENT_SLOTS];
};

class Account {
public:

	typedef SharedPtr<Account> Ptr;

	Account(AccountManager* pOwner);;
	~Account();

	inline void saved() { mTouched = false; }

	// Returns data for a specific Character on this Account.
	Data::AccountCharacter* getData(const String& pCharacterName);

	// Returns whether or not this Account is banned.
	inline const bool isBanned() const { return getStatus() == AccountStatus::Banned; }

	// Returns whether or not this Account is suspended.
	inline const bool isSuspended() const { return getStatus() == AccountStatus::Suspended; }

	// Returns the Account ID.
	inline const u32 getAccountID() const { return mAccountID; }

	// Sets the Account ID.
	inline void setAccountID(const u32 pValue) { mAccountID = pValue; _touch(); }

	// Returns the Login Server Account ID.
	inline const u32 getLSAccountID() const { return mLSAccountID; }

	// Sets the Login Server Account ID.
	inline void setLSAccountID(const u32 pValue) { mLSAccountID = pValue; _touch(); }
	
	// Returns the Login Server ID this account is associated with.
	inline const u32 getLSID() const { return mLSID; }

	// Sets the Login Server ID.
	inline void setLSID(const u32 pValue) { mLSID = pValue; _touch(); }
	
	// Returns the Login Server Account name.
	inline const String& getLSAccountName() const { return mLSAccountName; }

	// Sets the Login Server Account name.
	inline void setLSAccountName(const String& pValue) { mLSAccountName = pValue; _touch(); }
	
	// Returns the amount of shared platinum on this Account.
	inline const i32 getSharedPlatinum() const { return mSharedPlatinum; }

	// Sets the amount of shared platinum on this Account.
	inline void setSharedPlatinum(const i32 pValue) { mSharedPlatinum = pValue; _touch(); }

	// Returns the Account status.
	inline const i32 getStatus() const { return mStatus; }

	// Sets the Account status.
	void setStatus(const i32 pValue) { mStatus = pValue; _touch(); }

	// Returns the suspension time.
	const Poco::DateTime getSuspensionExpiry() const { return mSuspensionExpiry; }

	// Sets the suspension time.
	inline void setSuspensionExpiry(const Poco::DateTime pValue) { mSuspensionExpiry = pValue; _touch(); }

	// Returns the creation time of the Account.
	inline const Poco::DateTime getCreated() const { return mCreated; }

	// Sets the creation time of the Account.
	inline void setCreated(const Poco::DateTime pValue) { mCreated = pValue; _touch(); }

	// Sets the active Character.
	inline void setActiveCharacter(SharedPtr<Character> pCharacter) { mActiveCharacter = pCharacter; }

	// Returns the active Character.
	inline SharedPtr<Character> getActiveCharacter() const { return mActiveCharacter; }

	// Clears the active Character.
	inline void clearActiveCharacter() { mActiveCharacter = nullptr; }

	// Sets the authentication on this Account.
	inline void setAuthentication(const String& pKey, const u32 pIP) { mKey = pKey; mIP = pIP; _touch(); }

	// Clears the authentication on this Account.
	inline void clearAuthentication() { mKey = ""; mIP = 0; _touch(); }

	// Sets the authentication key for this Account.
	inline void setKey(const String& pValue) { mKey = pValue; _touch(); }

	// Returns the authentication key for this Account.
	inline const String& getKey() const { return mKey; }

	// Sets the authentication IP for this Account.
	inline void setIP(const u32 pValue) { mIP = pValue; _touch(); }

	// Returns the authentication IP for this Account.
	inline const u32 getIP() const { return mIP; }

	// Returns the reserved Character name.
	inline const String& getReservedCharacterName() const { return mReservedCharacterName; }

	// Returns whether this Account has a reserved name or not.
	inline const bool hasReservedCharacterName() const { return mReservedCharacterName.size() > 0; }

	// Sets the reserved Character name.
	inline void setReservedCharacterName(const String& pCharacterName) { mReservedCharacterName = pCharacterName; }

	// Clears the reserved Character name.
	inline void clearReservedCharacterName() { mReservedCharacterName = ""; }

	inline void setSessionID(const i32 pValue) { mSessionID = pValue; }
	inline const i32 getSessionID() const { return mSessionID; }
	inline void setSessionBeginTime(const Poco::DateTime& pValue) { mSessionBeginTime = pValue; }
	inline const Poco::DateTime& getSessionBeginTime() const { return mSessionBeginTime; }
	inline void setSessionEndTime(const Poco::DateTime& pValue) { mSessionEndTime = pValue; }
	inline const Poco::DateTime& getSessionEndTime() const { return mSessionEndTime; }

private:
	// Flag indicates if an Account has had changes that are unsaved.
	bool mTouched = false;
	inline void _touch() { mTouched = true; }

	AccountManager* mOwner = nullptr;
	SharedPtr<Character> mActiveCharacter = nullptr;

	u32 mAccountID = 0;						// (World) Internal ID.
	u32 mExtraCharacterSlots = 0;			// The number of extra Characters that can be created on this Account. TODO!
	i32 mSharedPlatinum = 0;				// Amount of shared platinum on this Account.
	i32 mStatus = AccountStatus::Default;	// Status of the Account, see enum AccountStatus for more details.
	Poco::DateTime mCreated;				// Date/Time the Account was created.
	Poco::DateTime mSuspensionExpiry;		// Date/Time when an Account suspension expires.

	i32 mSessionID = 0;						// Current Session ID.
	String mKey;
	u32 mIP = 0;
	Poco::DateTime mSessionBeginTime;		// Date/Time when connected (if connected)
	Poco::DateTime mSessionEndTime;			// Date/Time when disconnected.

	// Login Server details.
	u32 mLSAccountID = 0;					// Provided by Login Server
	String mLSAccountName = "";				// Provided by Login Server
	u32 mLSID = 1;							// ID of the Login Server this Account originates from.

	String mReservedCharacterName;
};