#pragma once

#include "Types.h"
#include "Poco/DateTime.h"

namespace Data {
	struct Account;
	struct AccountCharacter;
}

class AccountManager;
class Character;

namespace AccountStatus {
	enum : i32 {
		Banned = -2,
		Suspended = -1,
		Default = 0,
		BypassLock = 20,
	};
}

/*

An Account is persistent from the time a client connects to World till when they disconnect.

*/

class Account {
public:

	typedef SharedPtr<Account> Ptr;

	Account(AccountManager* pOwner);;
	~Account();

	inline void saved() { mSaveNeeded = false; }

	// Returns the data for this Account.
	Data::Account* getData() { return mData; }

	// Returns data for a specific Character on this Account.
	Data::AccountCharacter* getData(const String& pCharacterName);

	// Returns whether or not this Account is banned.
	inline const bool isBanned() const { return getStatus() == AccountStatus::Banned; }

	// Returns whether or not this Account is suspended.
	inline const bool isSuspended() const { return getStatus() == AccountStatus::Suspended; }

	// Returns the Account ID.
	inline const u32 getAccountID() const { return mAccountID; }

	// Sets the Account ID.
	inline void setAccountID(const u32 pValue) { mAccountID = pValue; mSaveNeeded = true; }

	// Returns the Login Server Account ID.
	inline const u32 getLSAccountID() const { return mLSAccountID; }

	// Sets the Login Server Account ID.
	inline void setLSAccountID(const u32 pValue) { mLSAccountID = pValue; mSaveNeeded = true; }
	
	// Returns the Login Server ID this account is associated with.
	inline const u32 getLSID() const { return mLSID; }

	// Sets the Login Server ID.
	inline void setLSID(const u32 pValue) { mLSID = pValue; mSaveNeeded = true; }
	
	// Returns the Login Server Account name.
	inline const String& getLSAccountName() const { return mLSAccountName; }

	// Sets the Login Server Account name.
	inline void setLSAccountName(const String& pValue) { mLSAccountName = pValue; mSaveNeeded = true; }
	
	// Returns the amount of shared platinum on this Account.
	inline const i32 getSharedPlatinum() const { return mSharedPlatinum; }

	// Sets the amount of shared platinum on this Account.
	inline void setSharedPlatinum(const i32 pValue) { mSharedPlatinum = pValue; mSaveNeeded = true; }

	// Returns the Account status.
	inline const i32 getStatus() const { return mStatus; }

	// Sets the Account status.
	void setStatus(const i32 pValue) { mStatus = pValue; mSaveNeeded = true; }

	// Returns the suspension time.
	const Poco::DateTime getSuspensionExpiry() const { return mSuspensionExpiry; }

	// Sets the suspension time.
	inline void setSuspensionExpiry(const Poco::DateTime pValue) { mSuspensionExpiry = pValue; mSaveNeeded = true; }

	// Returns the creation time of the Account.
	inline const Poco::DateTime getCreated() const { return mCreated; }

	// Sets the creation time of the Account.
	inline void setCreated(const Poco::DateTime pValue) { mCreated = pValue; mSaveNeeded = true; }

	// Returns the number of Characters on this Account.
	const u32 numCharacters() const;

	// Returns whether this Account owns the specified Character.
	const bool ownsCharacter(const String& pCharacterName) const;

	// Sets the active Character.
	inline void setActiveCharacter(Character* pCharacter) { mActiveCharacter = pCharacter; }

	// Returns the active Character.
	inline Character* getActiveCharacter() const { return mActiveCharacter; }

	// Clears the active Character.
	inline void clearActiveCharacter() { mActiveCharacter = nullptr; }

	// Sets the authentication on this Account.
	inline void setAuthentication(const String& pKey, const u32 pIP) { mKey = pKey; mIP = pIP; mSaveNeeded = true; }

	//// Returns whether or not this Account has authentication.
	//inline const bool hasAuthentication() const { return !(mKey.empty() && mIP == 0); }

	// Clears the authentication on this Account.
	inline void clearAuthentication() { mKey = ""; mIP = 0; mSaveNeeded = true; }

	// Sets the authentication key for this Account.
	inline void setKey(const String& pValue) { mKey = pValue; mSaveNeeded = true; }

	// Returns the authentication key for this Account.
	inline const String& getKey() const { return mKey; }

	// Sets the authentication IP for this Account.
	inline void setIP(const u32 pValue) { mIP = pValue; mSaveNeeded = true; }

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
	bool mSaveNeeded = false;
	AccountManager* mOwner = nullptr;

	u32 mAccountID = 0;						// (World) Internal ID.
	u32 mExtraCharacterSlots = 0;			// The number of extra Characters that can be created on this Account. TODO!
	i32 mSharedPlatinum = 0;				// Amount of shared platinum on this Account.
	i32 mStatus = AccountStatus::Default;	// Status of the Account, see enum AccountStatus for more details.
	Poco::DateTime mCreated;				// Time the Account was created.
	Poco::DateTime mSuspensionExpiry;		// Time when an Account suspension expires.

	i32 mSessionID = 0;						// 
	Poco::DateTime mSessionBeginTime;		// Time when connected (if connected)
	Poco::DateTime mSessionEndTime;			// Time when disconnected.

	// Login Server details.
	u32 mLSAccountID = 0;					// Provided by Login Server
	String mLSAccountName = "";				// Provided by Login Server
	u32 mLSID = 1;							// ID of the Login Server this Account originates from.

	Data::Account* mData = nullptr;
	Character* mActiveCharacter = nullptr;
	String mKey;
	u32 mIP = 0;
	String mReservedCharacterName;
};