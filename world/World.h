#pragma once

#include "Types.h"
#include "LogSystem.h"
#include "../common/timer.h"

#include <list>
#include <map>

namespace Payload {
	namespace World {
		struct CreateCharacter;
	}
}

class Character;
class AccountManager;
class ZoneManager;
class GuildManager;
class IDataStore;
class ILogFactory;
class EventDispatcher;
class CharacterFactory;

class LoginServerConnection;
class EQStreamFactory;
class EQStreamIdentifier;
class WorldConnection;

class World {
public:
	World();
	~World();

	const bool initialise(IDataStore* pDataStore, ILogFactory* pLogFactory, CharacterFactory* pCharacterFactory, GuildManager* pGuildManager, ZoneManager* pZoneManager, AccountManager* pAccountManager);
	void update();

	inline const i32 getStatus() const { return mStatus; }
	inline const i32 numPlayers() const { return mPlayers; }
	inline const i32 numZones() const { return mZones; }

	// Returns whether the World is currently locked or not.
	inline const bool isLocked() const { return mLocked; }
	
	// Sets whether the World is locked.
	void setLocked(bool pLocked);

	// LoginServerConnection
	void onConnectRequest(LoginServerConnection* pConnection, const u32 pLoginAccountID);
	void onAuthentication(LoginServerConnection* pConnection, const u32 pLoginAccountID, const String& pLoginAccountName, const String& pLoginKey, const u32 pIP);

	// WorldConnection.
	const bool onConnect(WorldConnection* pConnection, const u32 pLoginAccountID, const String& pKey, const bool pZoning);
	const bool onApproveName(WorldConnection* pConnection, const String& pCharacterName);
	const bool onCreateCharacter(WorldConnection* pConnection, Payload::World::CreateCharacter* pPayload);
	const bool onDeleteCharacter(WorldConnection* pConnection, const String& pCharacterName);
	const bool onEnterWorld(WorldConnection* pConnection, const String& pCharacterName, const bool pZoning);

	void onLeaveWorld(Character* pCharacter);

private:

	bool mInitialised = false;
	bool mLocked = false;
	AccountManager* mAccountManager = nullptr;
	ZoneManager* mZoneManager = nullptr;
	GuildManager* mGuildManager = nullptr;
	IDataStore* mDataStore = nullptr;
	ILogFactory* mLogFactory = nullptr;
	ILog* mLog = nullptr;
	CharacterFactory* mCharacterFactory = nullptr;
	EventDispatcher* mEventDispatcher = nullptr;
	LoginServerConnection* mLoginServerConnection = nullptr;
	EQStreamFactory* mStreamFactory = nullptr;
	EQStreamIdentifier* mStreamIdentifier = nullptr;
	std::list<WorldConnection*> mWorldConnections;

	const bool _handleZoning(WorldConnection* pConnection, const String& pCharacterName);
	bool _handleEnterWorld(WorldConnection* pConnection, const String& pCharacterName);

	void updateLoginServer() const;
	void checkIncomingConnections();
	
	i32 mStatus = 100;
	i32 mPlayers = 100;
	i32 mZones = 100;

	// Login Server status timer.
	Timer mStatusUpdateTimer;
};