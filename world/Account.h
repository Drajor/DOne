#pragma once

#include "Types.h"

namespace Data {
	struct Account;
	struct AccountCharacter;
}

class ILog;
class Character;

class Account {
public:

	// Constructor.
	Account(Data::Account* pData, ILog* pLog);

	// Returns the data for this Account.
	Data::Account* getData() { return mData; }

	// Returns data for a specific Character on this Account.
	Data::AccountCharacter* getData(const String& pCharacterName);

	// Returns whether this Account is loaded or not.
	inline bool isLoaded() { return mLoaded; }

	// Sets whether this Account is loaded or not.
	inline void setLoaded(const bool pValue) { mLoaded = pValue; }

	// Returns whether or not this Account is banned.
	const bool isBanned() const;

	// Returns whether or not this Account is suspended.
	const bool isSuspended() const;

	// Returns the login server account ID.
	const u32 getLoginAccountID() const;
	
	// Returns the login server ID this account is associated with.
	const u32 getLoginServerID() const;
	
	// Returns the login server account name.
	const String& getLoginAccountName() const;
	
	// Returns the amount of shared platinum on this Account.
	const i32 getSharedPlatinum() const;

	// Sets the amount of shared platinum on this Account.
	void setSharedPlatinum(const i32 pSharedPlatinum);

	// Returns the Account status.
	const i8 getStatus() const;

	// Sets the Account status.
	void setStatus(const i8 pStatus);

	// Returns the suspension time.
	const i64 getSuspensionTime() const;

	// Sets the suspension time.
	void setSuspensionTime(const i64 pTime);

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
	inline void setAuthentication(const String& pKey, const u32 pIP) { mKey = pKey; mIP = pIP; };

	// Returns whether or not this Account has authentication.
	inline const bool hasAuthentication() const { return !(mKey.empty() && mIP == 0); }

	// Clears the authentication on this Account.
	inline void clearAuthentication() { mKey = ""; mIP = 0; }

	// Returns the key for this Account (empty when not online).
	inline const String& getKey() const { return mKey; }

	// Returns the IP for this Account (0 when not online).
	inline const u32 getIP() const { return mIP; }

	// Returns the reserved Character name.
	inline const String& getReservedCharacterName() const { return mReservedCharacterName; }

	// Returns whether this Account has a reserved name or not.
	inline const bool hasReservedCharacterName() const { return mReservedCharacterName.size() > 0; }

	// Sets the reserved Character name.
	inline void setReservedCharacterName(const String& pCharacterName) { mReservedCharacterName = pCharacterName; }

	// Clears the reserved Character name.
	inline void clearReservedCharacterName() { mReservedCharacterName = ""; }
private:

	ILog* mLog = nullptr;
	Data::Account* mData = nullptr;
	Character* mActiveCharacter = nullptr;
	String mKey;
	u32 mIP = 0;
	bool mLoaded = false;
	String mReservedCharacterName;
};