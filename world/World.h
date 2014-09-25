#pragma once

#include "Constants.h"
#include "Singleton.h"
#include "LogSystem.h"
#include "ClientAuthentication.h"

class LoginServerConnection;
class EQStreamFactory;
class EQStreamIdentifier;
class ZoneManager;
class AccountManager;
class DataStore;
class WorldClientConnection;

struct CharacterData;

//struct ZoneTransfer {
//	uint32 mAccountID = 0;
//	String mCharacterName = "";
//	ZoneID mFromZoneID = 0;
//	InstanceID mFromInstanceID = 0;
//	ZoneID mToZoneID = 0;
//	InstanceID mToInstanceID = 0;
//};

class World : public Singleton<World> {
private:
	friend class Singleton<World>;
	World() : mLog("[World]") { };
	~World();
	World(World const&); // Do not implement.
	void operator=(World const&); // Do not implement.
public:
	bool initialise();
	void update();

	void addAuthentication(ClientAuthentication& pAuthentication);
	void removeAuthentication(ClientAuthentication& pAuthentication);
	bool checkAuthentication(WorldClientConnection* pConnection, const uint32 pAccountID, const String& pKey);
	//const bool addZoneAuthentication(ClientAuthentication& pAuthentication, String pCharacterName, ZoneID pZoneID, uint32 pInstanceID = 0);
	bool authenticationExists(uint32 pLoginServerID);
	bool ensureAccountExists(const uint32 pAccountID, const String& pAccountName);

	// Return whether World is connected to the Login Server.
	bool isLoginServerConnected();

	bool getLocked() { return mLocked; }
	void setLocked(bool pLocked);

	// Login Server requests response for Client who would like to join the World.
	ResponseID getConnectResponse(uint32 pLoginServerAccountID);

	// Character Select Screen
	bool isCharacterNameUnique(String pCharacterName);
	bool isCharacterNameReserved(String pCharacterName);
	void reserveCharacterName(uint32 pWorldAccountID, String pCharacterName);
	bool deleteCharacter(const uint32 pAccountID, const String& pCharacterName);
	
	//// Zone Transfer.
	//const bool getCharacterZoneTransfer(const String& pCharacterName, ZoneTransfer& pZoneTransfer);
	//inline void addCharacterZoneTransfer(ZoneTransfer pZoneChangeData) { mZoneTransfers.push_back(pZoneChangeData); }
	//void removeZoneTransfer(const String& pCharacterName);
	//const bool hasZoningCharacter(const uint32 pAccountID);
	//String getZoningCharacterName(const uint32 pAccountID);

	const bool handleEnterWorld(WorldClientConnection* pConnection, const String& pCharacterName, const bool pZoning);
private:
	LogContext mLog;
	ClientAuthentication* findAuthentication(uint32 pLoginServerAccountID);
	std::list<ClientAuthentication*> mAuthenticatedClients; // These are Clients the Login Server has told us about but have not yet fully connected to the World.
	std::map<uint32, String> mReservedCharacterNames;
	//std::list<ZoneTransfer> mZoneTransfers;
	void _handleIncomingClientConnections();

	bool mInitialised = false;
	bool mLocked = false;
	LoginServerConnection* mLoginServerConnection = nullptr;
	EQStreamFactory* mStreamFactory = nullptr;
	EQStreamIdentifier* mStreamIdentifier = nullptr;

	std::list<WorldClientConnection*> mClientConnections;

	bool _handleZoning(WorldClientConnection* pConnection, const String& pCharacterName);
	bool _handleEnterWorld(WorldClientConnection* pConnection, const String& pCharacterName);
};