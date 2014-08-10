#pragma once

#include "../common/types.h"
#include "Constants.h"
#include "ClientAuthentication.h"
#include <list>
#include <map>
#include <string>

class EmuTCPServer;
class LoginServerConnection;
class UCSConnection;
class EQStreamFactory;
class EQStreamIdentifier;
class ZoneManager;
class AccountManager;
class DataStore;
class WorldClientConnection;
struct CharacterSelect_Struct;
struct CharCreate_Struct;

struct ZoneTransfer {
	String mCharacterName;
	ZoneID mFromZoneID;
	InstanceID mFromInstanceID;
	ZoneID mToZoneID;
	InstanceID mToInstanceID;
};

class World {
public:
	static World& getInstance() {
		static World instance;
		return instance;
	}

	bool initialise();
	void update();

	void addAuthentication(ClientAuthentication& pAuthentication);
	void removeAuthentication(ClientAuthentication& pAuthentication);
	bool checkAuthentication(WorldClientConnection* pConnection, uint32 pLoginServerAccountID, String pLoginServerKey);
	void addZoneAuthentication(ClientAuthentication& pAuthentication, String pCharacterName, ZoneID pZoneID, uint32 pInstanceID = 0);
	bool authenticationExists(uint32 pLoginServerID);
	bool ensureAccountExists(uint32 pLoginServerAccountID, String pLoginServerAccountName);

	// Return whether World is connected to the Login Server.
	bool isLoginServerConnected();

	bool getLocked() { return mLocked; }
	void setLocked(bool pLocked);

	// Login Server requests response for Client who would like to join the World.
	int16 getUserToWorldResponse(uint32 pLoginServerAccountID);

	// Character Select Screen
	bool getCharacterSelectInfo(uint32 pWorldAccountID, CharacterSelect_Struct* pCharacterSelectData);
	bool isCharacterNameUnique(String pCharacterName);
	bool isCharacterNameReserved(String pCharacterName);
	void reserveCharacterName(uint32 pWorldAccountID, String pCharacterName);
	bool deleteCharacter(uint32 pWorldAccountID, String pCharacterName);
	bool createCharacter(uint32 pWorldAccountID, String pCharacterName, CharCreate_Struct* pData);
	bool isWorldEntryAllowed(uint32 pWorldAccountID, String pCharacterName);

	uint16 getZonePort(ZoneID pZoneID, uint16 pInstanceID = 0);
	
	bool getCharacterZoneTransfer(String& pCharacterName, ZoneTransfer& pZoneTransfer);
	void addCharacterZoneTransfer(ZoneTransfer pZoneChangeData) { mZoneTransfers.push_back(pZoneChangeData); }
	void removeZoneTransfer(String& pCharacterName);
private:
	ClientAuthentication* findAuthentication(uint32 pLoginServerAccountID);
	std::list<ClientAuthentication*> mAuthenticatedClients; // These are Clients the Login Server has told us about but have not yet fully connected to the World.
	std::map<uint32, String> mReservedCharacterNames;
	std::list<ZoneTransfer> mZoneTransfers;
	void _checkUCSConnection();
	void _handleIncomingClientConnections();


	






	bool mInitialised;
	bool mLocked;
	EmuTCPServer* mTCPServer;
	LoginServerConnection* mLoginServerConnection;
	UCSConnection* mUCSConnection;
	EQStreamFactory* mStreamFactory;
	EQStreamIdentifier* mStreamIdentifier;
	AccountManager* mAccountManager;

	std::list<WorldClientConnection*> mClientConnections;

	World();
	~World();
	World(World const&);
	void operator=(World const&);
};