#pragma once

#include "Constants.h"
#include "Singleton.h"
#include "Data.h"

namespace Payload {
	namespace World {
		struct CreateCharacter;
	}
}
class Character;

class AccountManager : public Singleton<AccountManager> {
private:
	friend class Singleton<AccountManager>;
	AccountManager() {};
	~AccountManager();
	AccountManager(AccountManager const&); // Do not implement.
	void operator=(AccountManager const&); // Do not implement.
public:
	bool initialise();

	AccountData* getAccount(uint32 pAccountID);
	bool checkOwnership(const uint32 pAccountID, const String& pCharacterName);
	bool createAccount(const uint32 pAccountID, const String pAccoutName);

	AccountStatus getStatus(const uint32 pAccountID);
	bool create(const String& pAccountName);
	bool exists(const uint32 pAccountID);

	bool isCharacterNameUnique(const String& pCharacterName);

	bool handleCharacterCreate(const uint32 pAccountID, const String& pCharacterName, Payload::World::CreateCharacter* pPayload);
	bool deleteCharacter(const uint32 pAccountID, const String& pCharacterName);
	const bool updateCharacter(const uint32 pAccountID, const Character* pCharacter);

	bool ban(const String& pAccountName);
	bool removeBan(const String& pAccountName);

	bool suspend(const String& pAccountName, const uint32 pSuspendUntil);
	bool removeSuspend(const String& pAccountName);
	void ensureAccountLoaded(const uint32 pAccountID);
private:
	void _clear();
	bool _save(AccountData* pAccountData);
	bool _save();

	AccountData* _load(const String& pAccountName);
	bool _loadAccount(const String& pAccountName);

	AccountData* _find(const uint32 pAccountID);
	AccountData* _find(const String& pAccountName);

	std::list<String> mCharacterNames;
	std::list<AccountData*> mAccounts;
};