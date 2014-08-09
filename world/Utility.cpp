#include "Utility.h"
#include "Character.h"
#include "Zone.h"
#include <iostream>

void Utility::print(std::string pMessage)
{
	std::cout << pMessage << std::endl;
}

void Utility::criticalError(std::string pMessage)
{
	print("[Critical Error] " + pMessage);
}

std::string Utility::characterLogDetails(Character* pCharacter) {
	if (pCharacter == nullptr) return "[NULL CHARACTER POINTER]";

	std::stringstream ss;
	ss << "[Character(Name: " << pCharacter->getName() << /*" Account ID: " << pCharacter->getWorldAccountID() <<*/ ")]";
	return ss.str();
}

std::string Utility::zoneLogDetails(Zone* pZone) {
	if (pZone == nullptr) return "[NULL ZONE POINTER]";

	std::stringstream ss;
	ss << "[Zone(Name: " << pZone->getLongName() << " ID:" << static_cast<std::uint16_t>(pZone->getID()) << " InstanceID:" << pZone->getInstanceID() << ")]";
	return ss.str();
}
