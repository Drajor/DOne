#pragma once

#include "Types.h"
#include <list>

namespace Data {
	struct Account;
}

namespace Payload {
	namespace World {
		struct CreateCharacter;
	}
}

class IDataStore;
class ILog;
class ILogFactory;
class Character;
class Account;

class AccountManager {
public:
	~AccountManager();

	// Initialises the AccountManager.
	const bool initialise(IDataStore* pDataStore, ILogFactory* pLogFactory);

	// Returns the total number of Accounts.
	inline const u32 numAccounts() const { return mAccounts.size(); }

	// Creates a new Account.
	Account* createAccount(const u32 pAccountID, const String pAccountName, const u32 pLoginServerID);

	// Returns an Account.
	Account* getAccount(const u32 pLoginAccountID, const u32 pLoginServerID);

	// Returns an authenticated Account.
	Account* getAuthenticatedAccount(const u32 pLoginAccountID, const String& pKey, const u32 pIP);
	
	// Returns the status of Account.
	const i8 getStatus(const u32 pLoginAccountID, const u32 pLoginServerID);

	// Returns whether a specific Account exists.
	const bool exists(const u32 pLoginAccountID, const String& pLoginAccountName, const u32 pLoginServerID) const;
	
	// Creates a Character for a specified Account.
	const bool createCharacter(Account* pAccount, Payload::World::CreateCharacter* pPayload);

	// Deletes a Character from a specified Account.
	const bool deleteCharacter(Account* pAccount, const String& pCharacterName);

	// Updates a specific Character.
	const bool updateCharacter(Account* pAccount, const Character* pCharacter);

	// Bans the specified Account.
	const bool ban(Account* pAccount);
	
	// Removes the ban on the specified Account.
	const bool removeBan(Account* pAccount);

	// Suspends the specified Account.
	const bool suspend(Account* pAccount, const u32 pExpiry);

	// Removes the suspension on the specified Account.
	const bool removeSuspend(Account* pAccount);

	// Returns whether a Character name is allowed to be used.
	const bool isCharacterNameAllowed(const String& pCharacterName) const;

	const bool onConnect(Account* pAccount);
	const bool onDisconnect(Account* pAccount);

private:

	bool mInitialised = false;
	IDataStore* mDataStore = nullptr;
	ILogFactory* mLogFactory = nullptr;
	ILog* mLog = nullptr;

	const bool _loadAccount(Account* pAccount);
	const bool _unloadAccount(Account* pAccount);

	// Returns whether a specific Character name is already being used.
	const bool _isCharacterNameInUse(const String& pCharacterName) const;

	// Returns whether a specific Character name is currently reserved.
	const bool _isCharacterNameReserved(const String& pCharacterName) const;

	void _clear();
	bool _save(Data::Account* pAccountData);
	bool _save();
	
	Account* _find(const u32 pLoginAccountID, const String& pLoginAccountName, const u32 pLoginServerID) const;
	Account* _find(const u32 pLoginAccountID, const u32 pLoginServerID) const;

	std::list<String> mCharacterNames;

	std::list<Account*> mAccounts;
};