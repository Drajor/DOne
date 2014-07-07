#pragma once

class EQStreamFactory;
class EQStreamIdentifier;
class ZoneManager;

class World {
public:
	World();
	~World();
	bool initialise();
	void update();
private:
	void _handleIncomingConnections();
	bool mInitialised;
	EQStreamFactory* mStreamFactory;
	EQStreamIdentifier* mStreamIdentifier;
	ZoneManager* mZoneManager;
};