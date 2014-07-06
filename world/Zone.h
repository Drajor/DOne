#pragma once

#include <list>

class EQStreamFactory;
class EQStreamIdentifier;
class Character;

class Zone {
public:
	Zone();
	~Zone();
	bool initialise();
	void shutdown();
	void update();

private:
	void _handleIncomingConnections();

	bool mInitialised; // Flag indicating whether the Zone has been initialised.
	EQStreamFactory* mStreamFactory;
	EQStreamIdentifier* mStreamIdentifier;
	std::list<Character*> mCharacters; // List of Player Characters in Zone.
};