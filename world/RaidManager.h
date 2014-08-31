#pragma once

#include "Constants.h"
#include "Singleton.h"

class Character;
class Raid;

class RaidManager : public Singleton<RaidManager> {
private:
	friend class Singleton<RaidManager>;
	RaidManager() {};
	~RaidManager() {};
	RaidManager(RaidManager const&); // Do not implement.
	void operator=(RaidManager const&); // Do not implement.
public:

	void onEnterZone(Character* pCharacter);
	void onLeaveZone(Character* pCharacter);
	void onCamp(Character* pCharacter);
	void onLinkdead(Character* pCharacter);

private:
};

class Raid {

};
