#pragma once

#include "Types.h"
#include <list>
#include "Poco/DateTime.h"

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

/*

IP ban is not currently supported, it can be added later if it is needed.

*/

class AccountManager {
public:
	~AccountManager();

	// Initialises the AccountManager.
	const bool initialise(IDataStore* pDataStore, ILogFactory* pLogFactory);

	// Creates a new Account.
	SharedPtr<Account> create(const u32 pLSAccountID, const String& pLSAccountName, const u32 pLSID);

	// Loads and returns an existing Account.
	SharedPtr<Account> load(const u32 pLSAccountID, const u32 pLServerID);

	// Returns whether a specific Account exists.
	const bool isCreated(const u32 pLSAccountID, const u32 pLSID);

	// Returns whether a specific Account is currently connected to the server.
	const bool isConnected(const u32 pLSAccountID, const u32 pLSID);
	
	// Creates a Character for a specified Account.
	const bool createCharacter(SharedPtr<Account> pAccount, Payload::World::CreateCharacter* pPayload);

	// Deletes a Character from a specified Account.
	const bool deleteCharacter(SharedPtr<Account> pAccount, const String& pCharacterName);

	// Updates a specific Character.
	const bool updateCharacter(SharedPtr<Account> pAccount, const Character* pCharacter);

	// Sets the the status of a specified Account.
	const bool setStatus(SharedPtr<Account> pAccount, const i32 pStatus);

	// Bans the specified Account.
	const bool ban(SharedPtr<Account> pAccount);
	
	// Removes the ban on the specified Account.
	const bool removeBan(SharedPtr<Account> pAccount);

	// Suspends the specified Account.
	const bool suspend(SharedPtr<Account> pAccount, const Poco::DateTime pExpiry);

	// Removes the suspension on the specified Account.
	const bool removeSuspension(SharedPtr<Account> pAccount);

	// Checks whether the suspension on the specified Account.
	// If the Account is suspended and the suspension has expired, it will be removed.
	void checkSuspension(SharedPtr<Account> pAccount);

	// Returns whether a Character name is allowed to be used.
	const bool isCharacterNameAllowed(const String& pCharacterName) const;

	const bool onConnect(SharedPtr<Account> pAccount);
	const bool onDisconnect(SharedPtr<Account> pAccount);

	// Saves a specific Account.
	const bool _save(Account* pAccount);

	// Returns whether a specific Character name is already being used.
	const bool _isCharacterNameInUse(const String& pCharacterName) const;

	// Returns whether a specific Character name is currently reserved.
	const bool _isCharacterNameReserved(const String& pCharacterName) const;

private:

	bool mInitialised = false;
	IDataStore* mDataStore = nullptr;
	ILogFactory* mLogFactory = nullptr;
	ILog* mLog = nullptr;
	
	SharedPtr<Account> _find(const u32 pLSAccountID, const String& pSAccountName, const u32 pLSID) const;
	SharedPtr<Account> _find(const u32 pLSAccountID, const u32 pLSID) const;

	std::list<SharedPtr<Account>> mAccounts;
};