#pragma once

#include "Types.h"

namespace Data {
	struct Account;
	struct CharacterData;
}

class ILog;
class Character;

class Account {
public:
	Account(Data::Account* pData, ILog* pLog);
	Data::Account* getData() { return mData; }
	Data::CharacterData* getData(const String& pCharacterName);

	inline bool isLoaded() { return mLoaded; }
	inline void setLoaded(const bool pValue) { mLoaded = pValue; }

	inline const bool isOnline() const { return mActiveCharacter != nullptr; }
	const u32 getLoginAccountID() const;
	const u32 getLoginServerID() const;
	const String& getLoginAccountName() const;
	
	// Returns the amount of shared platinum on this Account.
	const i32 getSharedPlatinum() const;

	// Sets the amount of shared platinum on this Account.
	void setSharedPlatinum(const i32 pSharedPlatinum);

	const i8 getStatus() const;
	void setStatus(const i8 pStatus);

	const u32 getSuspensionTime() const;
	void setSuspensionTime(const u32 pTime);

	const u32 numCharacters() const;
	const bool ownsCharacter(const String& pCharacterName) const;

	inline Character* getActiveCharacter() const { return mActiveCharacter; }
	inline void clearActiveCharacter() { mActiveCharacter = nullptr; }

	inline void setAuthentication(const String& pKey, const u32 pIP) { mKey = pKey; mIP = pIP; };
	inline void clearAuthentication() { mKey = ""; mIP = 0; }
	inline const String& getKey() const { return mKey; }
	inline const u32 getIP() const { return mIP; }

	inline const String& getReservedCharacterName() const { return mReservedCharacterName; }
	inline void setReservedCharacterName(const String& pCharacterName) { mReservedCharacterName = pCharacterName; }
private:

	ILog* mLog = nullptr;
	Data::Account* mData = nullptr;
	Character* mActiveCharacter = nullptr;
	String mKey;
	u32 mIP = 0;
	bool mLoaded = false;
	String mReservedCharacterName;
};