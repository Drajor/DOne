#pragma once

#include "Constants.h"

class Character;

class Guild {
	uint32 mID;
	String mName;
	std::list<Character*> mOnlineMembers;
};

class GuildManager {

};