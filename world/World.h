#pragma once

#include "Types.h"
#include "Singleton.h"
#include "LogSystem.h"
#include "../common/timer.h"

#include <list>
#include <map>

class LoginServerConnection;
class EQStreamFactory;
class EQStreamIdentifier;
class WorldClientConnection;

class World : public Singleton<World> {
private:
	friend class Singleton<World>;
	World();;
	~World();
	World(World const&); // Do not implement.
	void operator=(World const&); // Do not implement.
public:
	bool initialise();
	bool _initialiseLoginServerConnection();
	void update();

	// General

	const i32 getStatus() const { return mStatus; }
	const i32 getPlayers() const { return mPlayers; }
	const i32 getZones() const { return mZones; }

	// Removes authentication by Account ID.
	void removeAuthentication(const u32 pAccountID);

	// Returns whether there is an existing authentication that matches both pAccountID and pKey.
	const bool checkAuthentication(const u32 pAccountID, const String& pKey) const;

	// Returns whether there is an existing authentication for pAccountID
	const bool authenticationExists(const u32 pAccountID) const;

	// Login Server Connection

	void handleConnectRequest(const u32 pAccountID);
	void handleClientAuthentication(const u32 pAccountID, const String& pAccountName, const String& pKey, const u32 pIP);

	void addAuthentication(const u32 pAccountID, const String& pAccountName, const String& pKey, const u32 pIP);

	// Return whether World is connected to the Login Server.
	bool isLoginServerConnected();

	bool getLocked() { return mLocked; }
	void setLocked(bool pLocked);

	// Login Server requests response for Client who would like to join the World.
	const u8 getConnectResponse(const u32 pAccountID);

	// Character Select Screen
	bool isCharacterNameUnique(String pCharacterName);
	bool isCharacterNameReserved(String pCharacterName);
	void reserveCharacterName(uint32 pWorldAccountID, String pCharacterName);
	bool deleteCharacter(const uint32 pAccountID, const String& pCharacterName);

	const bool handleEnterWorld(WorldClientConnection* pConnection, const String& pCharacterName, const bool pZoning);
private:
	LogContext mLog;
	std::map<uint32, String> mReservedCharacterNames;
	void _handleIncomingClientConnections();

	bool mInitialised = false;
	bool mLocked = false;
	LoginServerConnection* mLoginServerConnection = nullptr;
	EQStreamFactory* mStreamFactory = nullptr;
	EQStreamIdentifier* mStreamIdentifier = nullptr;

	std::list<WorldClientConnection*> mClientConnections;

	bool _handleZoning(WorldClientConnection* pConnection, const String& pCharacterName);
	bool _handleEnterWorld(WorldClientConnection* pConnection, const String& pCharacterName);
	
	i32 mStatus = 100;
	i32 mPlayers = 100;
	i32 mZones = 100;

	void _updateLoginServer() const;
	// Login Server status timer.
	Timer mStatusUpdateTimer;

	struct Authentication {
		u32 mAccountID = 0;
		String mAccountName;
		String mKey;
		u32 mIP = 0;
		u32 mExpiryTime = 0;
	};
	std::list<Authentication*> mAuthentations;
};