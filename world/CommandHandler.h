#pragma once

#include "Types.h"
#include <vector>
#include <list>

static const char COMMAND_TOKEN = '#';
static const char HELP_TOKEN = '?';

typedef std::vector<String> CommandParameters;

class DataStore;
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
	inline void setRequiredParameters(const u8 pMinimum, const u8 pMaximum) { mMinimumParameters = pMinimum; mMaximumParameters = pMaximum; }
	virtual void invalidParameters(CommandParameters pParameters);
protected:
	virtual void conversionError(String& pParameter);

	template <typename T>
	const bool convertParameter(CommandParameters::size_type pIndex, T& pValue) {
		if (pIndex >= mParameters.size()) return false;

		if (!Utility::stoSafe(pValue, mParameters[pIndex])) {
			//conversionError(mParameters[pIndex]);
			return false;
		}
		return true;
	}

	inline void setMinimumParameters(uint8 pValue) { mMinimumParameters = pValue; }
	inline void setMaximumParameters(uint8 pValue) { mMaximumParameters = pValue; }
	
	CommandParameters mParameters;
	std::list<String> mAliases;
	u8 mMinimumStatus = UINT8_MAX;
	std::list<String> mHelpMessages;
	Character* mInvoker = nullptr;
	bool mLogged = true;
	u8 mMinimumParameters = 0;
	u8 mMaximumParameters = UINT8_MAX;
	bool mRequiresTarget = false;
};

class CommandHandler {
public:
	~CommandHandler();
	const bool initialise(DataStore* pDataStore);
	void command(Character* pCharacter, String pCommandMessage);

private:

	bool mInitialised = false;
	DataStore* mDataStore = nullptr;

	void _logCommand(Character* pCharacter, String pCommandMessage);
	Command* findCommand(String pCommandName);
	void _handleCommand(Character* pCharacter, const String& pCommandName, std::vector<String> pParameters);
	std::list<Command*> mCommands;
};