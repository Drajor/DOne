#pragma once

#include <string>
#include <vector>
#include <list>
#include <cstdint>

static const char COMMAND_TOKEN = '#';
static const char HELP_TOKEN = '?';

typedef std::vector<std::string> CommandParameters;
class Character;

class Command {
public:
	Command(std::uint8_t pMinimumStatus, std::list<std::string> pAliases) : mMinimumStatus(pMinimumStatus), mAliases(pAliases){};
	virtual ~Command() {};
	std::list<std::string>& getAliases() { return mAliases; }
	std::uint8_t getMinimumStatus() { return mMinimumStatus; }
	virtual void handleCommand(Character* pCharacter, CommandParameters pParameters) = 0;
	virtual void helpMessage(Character* pCharacter);
protected:
	virtual void invalidParameters(Character* pCharacter, CommandParameters pParameters);
	
	std::list<std::string> mAliases;
	std::uint8_t mMinimumStatus;
	std::string mHelpMessage;
};

class CommandHandler {
public:
	~CommandHandler();
	void initialise();
	void command(Character* pCharacter, std::string pCommandMessage);
private:
	Command* findCommand(std::string pCommandName);
	void _handleCommand(Character* pCharacter, std::string pCommandName, std::vector<std::string> pParameters);
	std::list<Command*> mCommands;
};