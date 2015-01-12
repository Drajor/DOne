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

class AccountManager;
class ZoneManager;
class IDataStore;
class ILogFactory;
class EventDispatcher;

class LoginServerConnection;
class EQStreamFactory;
class EQStreamIdentifier;
class WorldConnection;

class World {
public:
	World();
	~World();

	const bool initialise(IDataStore* pDataStore, ILogFactory* pLogFactory, ZoneManager* pZoneManager, AccountManager* pAccountManager);
	void update();

	// General

	const i32 getStatus() const { return mStatus; }
	const i32 getPlayers() const { return mPlayers; }
	const i32 getZones() const { return mZones; }

	const bool isLocked() const { return mLocked; }
	void setLocked(bool pLocked);

	// LoginServerConnection

	// Login Server requests response for Client who would like to join the World.
	const u8 getConnectResponse(const u32 pLoginAccountID, const u32 pLoginServerID);

	void onConnectRequest(LoginServerConnection* pConnection, const u32 pLoginAccountID);
	void onAuthentication(LoginServerConnection* pConnection, const u32 pLoginAccountID, const String& pLoginAccountName, const String& pLoginKey, const u32 pIP);

	// WorldConnection.

	const bool onConnect(WorldConnection* pConnection, const u32 pLoginAccountID, const String& pKey, const bool pZoning);
	const bool onApproveName(WorldConnection* pConnection, const String& pCharacterName);
	const bool onCreateCharacter(WorldConnection* pConnection, Payload::World::CreateCharacter* pPayload);
	const bool onDeleteCharacter(WorldConnection* pConnection, const String& pCharacterName);
	const bool onEnterWorld(WorldConnection* pConnection, const String& pCharacterName, const bool pZoning);

protected:
	
	AccountManager* mAccountManager = nullptr;
	ZoneManager* mZoneManager = nullptr;
	IDataStore* mDataStore = nullptr;
	ILogFactory* mLogFactory = nullptr;
	ILog* mLog = nullptr;
	EventDispatcher* mEventDispatcher = nullptr;

private:
	void _handleIncomingClientConnections();

	bool mInitialised = false;
	bool mLocked = false;
	LoginServerConnection* mLoginServerConnection = nullptr;
	EQStreamFactory* mStreamFactory = nullptr;
	EQStreamIdentifier* mStreamIdentifier = nullptr;

	std::list<WorldConnection*> mWorldConnections;

	bool _initialiseLoginServerConnection();

	bool _handleZoning(WorldConnection* pConnection, const String& pCharacterName);
	bool _handleEnterWorld(WorldConnection* pConnection, const String& pCharacterName);
	
	i32 mStatus = 100;
	i32 mPlayers = 100;
	i32 mZones = 100;

	void _updateLoginServer() const;
	// Login Server status timer.
	Timer mStatusUpdateTimer;
};