#include "Utility.h"
#include "Character.h"
#include "Zone.h"
#include <iostream>

void Utility::print(String pMessage)
{
	std::cout << pMessage << std::endl;
}

void Utility::criticalError(String pMessage)
{
	print("[Critical Error] " + pMessage);
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
	ss << "[Zone(Name: " << pZone->getLongName() << " ID:" << static_cast<std::uint16_t>(pZone->getID()) << " InstanceID:" << pZone->getInstanceID() << ")]";
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
