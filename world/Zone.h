#pragma once

#include "ClientAuthentication.h"
#include <list>
#include <map>
#include "../common/types.h"

class EQStreamFactory;
class EQStreamIdentifier;
class DataStore;
class ZoneClientConnection;

class World;
class Character;

class Zone {
public:
	Zone(World* pWorld, DataStore* pDataStore, uint32 pPort, uint32 pZoneID, uint32 pInstanceID = 0);
	~Zone();
	bool initialise();
	void addAuthentication(ClientAuthentication& pAuthentication, std::string pCharacterName);
	void removeAuthentication(std::string pCharacterName);
	bool checkAuthentication(std::string pCharacterName);

	void shutdown();
	void update();

	uint32 getID() { return mID; }
	uint32 getInstanceID() { return mInstanceID; }
	uint16 getPort() { return mPort; }

	void notifyCharacterLogOut(Character* pCharacter);
	void notifyCharacterZoneOut(Character* pCharacter);
	void notifyCharacterLinkDead(Character* pCharacter);
	void notifyCharacterAFK(Character* mCharacter, bool pAFK);
	void notifyCharacterShowHelm(Character* mCharacter, bool pShowHelm);
	void notifyCharacterAnonymous(uint8 pAnonymous);

	void updateCharacterPosition(Character* pCharacter, float pX, float pY, float pZ, float pHeading);
	void moveCharacter(Character* pCharacter, float pX, float pY, float pZ);
	uint16 getNextSpawnID() { return mNextSpawnID++; }
	
private:
	void _handleIncomingConnections();
	


	std::map<std::string, ClientAuthentication> mAuthenticatedCharacters;


	uint16 mNextSpawnID;
	uint32 mID; // Zone ID
	uint32 mInstanceID;
	uint32 mPort;

	bool mInitialised; // Flag indicating whether the Zone has been initialised.
	EQStreamFactory* mStreamFactory;
	EQStreamIdentifier* mStreamIdentifier;
	World* mWorld;
	DataStore* mDataStore;
	std::list<Character*> mCharacters; // List of Player Characters in Zone.
	std::list<ZoneClientConnection*> mZoneClientConnections;
};