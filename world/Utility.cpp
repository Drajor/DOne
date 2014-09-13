#include "Utility.h"
#include "Character.h"
#include "Zone.h"

#include <iostream>
#include <ctime>

void Utility::print(String pMessage)
{
	std::cout << pMessage << std::endl;
}

String Utility::characterLogDetails(Character* pCharacter) {
	if (pCharacter == nullptr) return "[NULL CHARACTER POINTER]";

	StringStream ss;
	ss << "[Character(Name: " << pCharacter->getName() << /*" Account ID: " << pCharacter->getWorldAccountID() <<*/ ")]";
	return ss.str();
}

String Utility::zoneLogDetails(Zone* pZone) {
	if (pZone == nullptr) return "[NULL ZONE POINTER]";

	StringStream ss;
	ss << "[Zone(Name: " << pZone->getLongName() << " ID:" << static_cast<uint16>(pZone->getID()) << " InstanceID:" << pZone->getInstanceID() << ")]";
	return ss.str();
}

String Utility::guildRankToString(const GuildRank& pRank){
	switch (pRank)	{
	case Member:
		return "Member";
	case Officer:
		return "Officer";
	case Leader:
		return "Leader";
	case GR_None:
		return "None";
	default:
		break;
	}

	return "ERROR";
}

int32 Utility::Time::now() {
	return static_cast<int32>(std::time(nullptr));
}
