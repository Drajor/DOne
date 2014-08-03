#pragma once

#include "../common/types.h"
#include <list>
#include <string>

class Character;

class Guild {
	uint32 mID;
	std::string mName;
	std::list<Character*> mOnlineMembers;
};

class GuildManager {

};