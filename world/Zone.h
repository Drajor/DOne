#pragma once

#include <list>
#include "../common/types.h"

class EQStreamFactory;
class EQStreamIdentifier;
class DataStore;
class ZoneClientConnection;

class Character;

class Zone {
public:
	Zone(DataStore* pDataStore, uint32 pPort, uint32 pZoneID, uint32 pInstanceID = 0);
	~Zone();
	bool initialise();
	void shutdown();
	void update();

	uint32 getID() { return mID; }
	uint32 getInstanceID() { return mInstanceID; }
	uint16 getPort() { return mPort; }

	void addExpectedCharacter(std::string pCharacterName);
	void removeExpectedCharacter(std::string pCharacterName);
	bool isClientExpected(std::string pCharacterName);

private:
	void _handleIncomingConnections();
	
	uint32 mID; // Zone ID
	uint32 mInstanceID;
	uint32 mPort;

	bool mInitialised; // Flag indicating whether the Zone has been initialised.
	EQStreamFactory* mStreamFactory;
	EQStreamIdentifier* mStreamIdentifier;
	DataStore* mDataStore;
	std::list<Character*> mCharacters; // List of Player Characters in Zone.
	std::list<ZoneClientConnection*> mZoneClientConnections;
	std::list<std::string> mExpectedCharacters; // List of Character(names) expected to be entering the Zone.
};