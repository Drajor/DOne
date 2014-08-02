#include "CommandHandler.h"
#include "Character.h"
#include "Zone.h"
#include "LogSystem.h"
#include <vector>
#include <sstream>
#include "ZoneClientConnection.h"
#include "Constants.h"

#define PACKET_PLAY
#ifdef PACKET_PLAY
#include "../common/eq_packet_structs.h"
#include "../common/EQPacket.h"
#endif

// Thank you: http://stackoverflow.com/questions/236129/how-to-split-a-string-in-c
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

// Thank you: http://stackoverflow.com/questions/236129/how-to-split-a-string-in-c
std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
}

bool stofSafe(float& pValue, std::string& pString) {
	try {
		pValue = std::stof(pString);
		return true;
	}
	catch (...) {
		return false;
	}
	return false;
}

bool stoulSafe(uint32& pValue, std::string& pString) {
	try {
		pValue = std::stoul(pString);
		return true;
	}
	catch (...) {
		return false;
	}
	return false;
}


void CommandHandler::command(Character* pCharacter, std::string pCommandMessage) {
	Log::info("Got a command message");

	// Break up the message.
	std::vector<std::string> elements = split(pCommandMessage, ' ');
	// Extract command name.
	std::string commandID = elements[0].substr(1, elements[0].length() - 1);
	// Remove command name (#zone)
	elements.erase(elements.begin());

	_handleCommand(pCharacter, commandID, elements);
}



void CommandHandler::_handleCommand(Character* pCharacter, std::string pCommandName, std::vector<std::string> pParameters) {
	// #warp
	if (pCommandName == "warp" && pCharacter->getStatus() > 200) {
		if (pParameters.size() == 3) {
			float x = 0.0f;
			float y = 0.0f;
			float z = 0.0f;
			if (stofSafe(x, pParameters[0]) && stofSafe(y, pParameters[1]) && stofSafe(z, pParameters[2])) {
				pCharacter->getZone()->moveCharacter(pCharacter, x, y, z);
			}
			else {
				pCharacter->message(MessageType::Red, "There was a problem with your parameters.");
			}
		}
	}
	// #heal
	else if (pCommandName == "heal") {
		pCharacter->healPercentage(100);
	}
	// #damage <amount>
	else if (pCommandName == "damage" && pParameters.size() == 1) {
		unsigned int damage = 0;
		if (stoulSafe(damage, pParameters[0])) {
			pCharacter->damage(damage);
		}
	}
	// #loc
	else if (pCommandName == "loc") {
		std::stringstream ss;
		ss << "Your location is " << pCharacter->getX() << ", " << pCharacter->getY() << ", " << pCharacter->getZ();
		pCharacter->message(MessageType::White, ss.str());
	}
	else if (pCommandName == "appearance") {
		if (pParameters.size() == 2) {
			uint32 appType = 0;
			uint32 appParameter = 0;
			if (stoulSafe(appType, pParameters[0]) && stoulSafe(appParameter, pParameters[1])) {
				pCharacter->getConnection()->sendAppearance(appType, appParameter);
			}
		}
	}
	// #anim <number>
	else if (pCommandName == "anim") {
		if (pParameters.size() == 1) {
			uint32 animationID = 0;
			if (stoulSafe(animationID, pParameters[0])) {
				pCharacter->doAnimation(animationID);
			}
		}
	}
	// #addexp <number>
	else if (pCommandName == "addexp") {
		if (pParameters.size() == 1) {
			uint32 expAdd = 0;
			if (stoulSafe(expAdd, pParameters[0])) {
				pCharacter->addExperience(expAdd);
			}
		}
	}
	// #remexp <number>
	else if (pCommandName == "remexp") {
		if (pParameters.size() == 1) {
			uint32 expRemove = 0;
			if (stoulSafe(expRemove, pParameters[0])) {
				pCharacter->removeExperience(expRemove);
			}
		}
	}
	// #setlevel <number>
	else if (pCommandName == "setlevel") {
		if (pParameters.size() == 1) {
			uint32 level = 0;
			if (stoulSafe(level, pParameters[0])) {
				pCharacter->setLevel(static_cast<uint8>(level));
			}
		}
	}
	// #gm <on/off>
	else if (pCommandName == "gm" && pCharacter->getStatus() >= 100) {
		if (pParameters.size() == 1) {
			bool gm = pParameters[0] == "on";
			pCharacter->setGM(gm);
			pCharacter->getZone()->notifyCharacterGM(pCharacter);
		}
	}
	else if (pCommandName == "zone" && pCharacter->getStatus() >= 100) {
		if (pParameters.size() == 2) {
			uint32 zoneID = 0;
			uint32 instanceID = 0;
			if (stoulSafe(zoneID, pParameters[0]) && stoulSafe(instanceID, pParameters[1])) {
				pCharacter->getConnection()->sendRequestZoneChange(zoneID, instanceID);
			}
		}
	}
	// #race <number>
	//else if (pCommandName == "race" && pCharacter->getStatus() >= 100) {
	//	if (pParameters.size() == 1) {
	//		uint32 race = 0;
	//		if (stoulSafe(pParameters[0], race)) {
	//			pCharacter->setRace(race);

	//		}
	//	}
	//}
	// #setstat
	else if (pCommandName == "setstat" && pCharacter->getStatus() >= 255) {
		if (pParameters.size() == 2) {
			std::string statStr = pParameters[0];
			std::string statName = "unknown";
			uint32 value = 0;
			Statistic statistic;
			if (!stoulSafe(value, pParameters[1])) {
				return;
			}

			// Determine which statistic is changing.
			if (statStr == "str") { statistic = Statistic::Strength; statName = "strength"; }
			else if (statStr == "sta") { statistic = Statistic::Stamina; statName = "stamina"; }
			else if (statStr == "cha") { statistic = Statistic::Charisma; statName = "charisma"; }
			else if (statStr == "dex") { statistic = Statistic::Dexterity; statName = "dexterity"; }
			else if (statStr == "int") { statistic = Statistic::Intelligence; statName = "intelligence"; }
			else if (statStr == "agi") { statistic = Statistic::Agility; statName = "agility"; }
			else if (statStr == "wis") { statistic = Statistic::Wisdom; statName = "wisdom"; }
			else {
				return;
			}

			std::stringstream ss;
			ss << "Changing " << pCharacter->getName() << "'s " << statName << " from " << pCharacter->getBaseStatistic(statistic) << " to " << value;
			pCharacter->message(MessageType::LightGreen, ss.str());
			pCharacter->setBaseStatistic(statistic, value);
		}
	}
	else {
		pCharacter->message(MessageType::Yellow, "Unknown command.");
	}
}
