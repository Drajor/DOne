#pragma once

#include "Constants.h"

static const char COMMAND_TOKEN = '#';
static const char HELP_TOKEN = '?';

typedef std::vector<String> CommandParameters;
class Character;

class Command {
public:
	Command(uint8 pMinimumStatus, std::list<String> pAliases, bool pLogged = true) : mMinimumStatus(pMinimumStatus), mAliases(pAliases), mLogged(pLogged) {};
	virtual ~Command() {};
	inline std::list<String>& getAliases() { return mAliases; }
	inline const uint8 getMinimumStatus() const { return mMinimumStatus; }
	void setInvoker(Character* pCharacter) { mInvoker = pCharacter; }
	inline void setParameters(CommandParameters pParameters) { mParameters = pParameters; }
	inline void clearParameters() { mParameters.clear(); }
	virtual const bool handleCommand(CommandParameters pParameters) = 0;
	virtual void helpMessage();
	inline const bool isLogged() const { return mLogged; }
	inline const uint8 getMinimumParameters() const { return mMinimumParameters; }
	inline const uint8 getMaximumParameters() const { return mMaximumParameters; }
	inline const bool getRequiresTarget() const { return mRequiresTarget; }
	virtual void invalidParameters(CommandParameters pParameters);
protected:
	virtual void conversionError(String& pParameter);

	template <typename T>
	const bool convertParameter(CommandParameters::size_type pIndex, T& pValue) {
		if (pIndex >= mParameters.size()) return false;

		if (!Utility::stoSafe(pValue, mParameters[pIndex])) {
			conversionError(mParameters[pIndex]);
			return false;
		}
		return true;
	}

	inline void setMinimumParameters(uint8 pValue) { mMinimumParameters = pValue; }
	inline void setMaximumParameters(uint8 pValue) { mMaximumParameters = pValue; }
	
	CommandParameters mParameters;
	std::list<String> mAliases;
	uint8 mMinimumStatus;
	std::list<String> mHelpMessages;
	Character* mInvoker = nullptr;
	bool mLogged = true;
	uint8 mMinimumParameters = 0;
	uint8 mMaximumParameters = UINT8_MAX;
	bool mRequiresTarget = false;
};

class CommandHandler {
public:
	~CommandHandler();
	void initialise();
	void command(Character* pCharacter, String pCommandMessage);
private:
	void _logCommand(Character* pCharacter, String pCommandMessage);
	Command* findCommand(String pCommandName);
	void _handleCommand(Character* pCharacter, String pCommandName, std::vector<String> pParameters);
	std::list<Command*> mCommands;
};