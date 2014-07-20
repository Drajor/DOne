#pragma once

#include <string>
#include <vector>

static const char COMMAND_TOKEN = '#';
class Character;

class CommandHandler {
public:
	void command(Character* pCharacter, std::string pCommandMessage);
private:
	void _handleCommand(Character* pCharacter, std::string pCommandName, std::vector<std::string> pParameters);
};