#pragma once

#include "Types.h"
#include <list>

class ZoneManager;
class Character;
class Raid;

class RaidManager {
public:
	const bool initialise(ZoneManager* pZoneManager);

	void onEnterZone(Character* pCharacter);
	void onLeaveZone(Character* pCharacter);
	void onCamp(Character* pCharacter);
	void onLinkdead(Character* pCharacter);
	void handleMessage(Character* pCharacter, const String pMessage) {};

private:

	bool mInitialised = false;
	ZoneManager* mZoneManager = nullptr;

	std::list<Raid*> mRaids;
};