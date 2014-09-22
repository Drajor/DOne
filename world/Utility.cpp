#include "Utility.h"
#include "Limits.h"
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

const String& Utility::Skills::fromID(const uint32 pSkillID) {
	// Check: pSkillID is in range.
	if (!Limits::Skills::validID(pSkillID)) {
		static const String BadSkillID = "BadSkillID";
		Log::error("Skill ID out of range: " + std::to_string(pSkillID) );
		return BadSkillID;
	}

	return SkillStrings[pSkillID];
}

const String& Utility::Languages::fromID(const uint32 pLanguageID) {
	// Check: pLanguageID is in range.
	if (!Limits::Languages::validID(pLanguageID)) {
		static const String BadLanguageID = "BadLanguageID";
		Log::error("Language ID out of range: " + std::to_string(pLanguageID));
		return BadLanguageID;
	}

	return LanguagesStrings[pLanguageID];
}
