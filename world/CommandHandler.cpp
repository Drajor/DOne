#include "CommandHandler.h"
#include "Character.h"
#include "Zone.h"
#include "LogSystem.h"
#include <vector>
#include <sstream>

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
	if (pCommandName == "warp" && pCharacter->getStatus() > 200) {
		if (pParameters.size() == 3) {
			float x = 0.0f;
			float y = 0.0f;
			float z = 0.0f;
			if (stofSafe(x, pParameters[0]) && stofSafe(y, pParameters[1]) && stofSafe(z, pParameters[2])) {
				pCharacter->getZone()->moveCharacter(pCharacter, x, y, z);
				//pCharacter->setPosition(x, y, z);
				//pCharacter->getZoneClientConnection()->sendPosition();
			}
			else {
				pCharacter->message(0, "Problem!");
				Log::error("Command Invalid");
			}
		}
	}
}
