#pragma once

#include "Constants.h"

static const char COMMAND_TOKEN = '#';
static const char HELP_TOKEN = '?';

typedef std::vector<String> CommandParameters;
class Character;

class Command {
public:
	Command(uint8 pMinimumStatus, std::list<String> pAliases) : mMinimumStatus(pMinimumStatus), mAliases(pAliases){};
	virtual ~Command() {};
	std::list<String>& getAliases() { return mAliases; }
	uint8 getMinimumStatus() { return mMinimumStatus; }
	void setInvoker(Character* pCharacter) { mInvoker = pCharacter; }
	virtual const bool handleCommand(CommandParameters pParameters) = 0;
	virtual void helpMessage();
protected:
	virtual void invalidParameters(CommandParameters pParameters);
	virtual void conversionError(String& pParameter);
	
	std::list<String> mAliases;
	uint8 mMinimumStatus;
	std::list<String> mHelpMessages;
	Character* mInvoker = nullptr;
};

class CommandHandler {
public:
	~CommandHandler();
	void initialise();
	void command(Character* pCharacter, String pCommandMessage);
private:
	Command* findCommand(String pCommandName);
	void _handleCommand(Character* pCharacter, String pCommandName, std::vector<String> pParameters);
	std::list<Command*> mCommands;
};