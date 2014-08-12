#pragma once

#include "Constants.h"

class Character;

class Raid {

};

class RaidManager {
public:
	static RaidManager& getInstance() {
		static RaidManager instance;
		return instance;
	}

	void onEnterZone(Character* pCharacter);;
	void onLeaveZone(Character* pCharacter);;
	void onCamp(Character* pCharacter);
	void onLinkdead(Character* pCharacter);

private:
	RaidManager() {};
	~RaidManager() {};
	RaidManager(RaidManager const&);
	void operator=(RaidManager const&);
	

};