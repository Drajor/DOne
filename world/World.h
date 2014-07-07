#pragma once

class EQStreamFactory;
class EQStreamIdentifier;
class ZoneManager;
class DataStore;

class World {
public:
	World(DataStore* pDataStore);
	~World();
	bool initialise();
	void update();
private:
	void _handleIncomingConnections();
	bool mInitialised;
	EQStreamFactory* mStreamFactory;
	EQStreamIdentifier* mStreamIdentifier;
	ZoneManager* mZoneManager;
	DataStore* mDataStore;
};