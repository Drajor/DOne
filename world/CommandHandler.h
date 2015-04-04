#pragma once

#include "Types.h"
#include <vector>
#include <list>

static const char COMMAND_TOKEN = '#';
static const char HELP_TOKEN = '?';

typedef std::vector<String> CommandParameters;

class IDataStore;
class Character;
class Actor;

class Command {
public:
	Command(u32 pMinimumStatus, std::list<String> pAliases, bool pLogged = true) : mMinimumStatus(pMinimumStatus), mAliases(pAliases), mLogged(pLogged) {};
	virtual ~Command() {};
	inline std::list<String>& getAliases() { return mAliases; }
	inline const u32 getMinimumStatus() const { return mMinimumStatus; }
	void setInvoker(Character* pCharacter) { mInvoker = pCharacter; }
	inline void setParameters(CommandParameters pParameters) { mParameters = pParameters; }
	inline void clearParameters() { mParameters.clear(); }
	virtual const bool execute(CommandParameters pParameters) = 0;
	virtual void helpMessage();
	inline const bool isLogged() const { return mLogged; }
	inline const uint8 getMinimumParameters() const { return mMinimumParameters; }
	inline const uint8 getMaximumParameters() const { return mMaximumParameters; }
	inline const bool getRequiresTarget() const { return mRequiresTarget; }
	inline Actor* getTargetActor() const { return mTargetActor; }
	inline void setTargetActor(Actor* pActor) { mTargetActor = pActor; }
	inline const bool getRequiresCharacterTarget() const { return mRequiresCharacterTarget; }
	inline Character* getTargetCharacter() const { return mTargetCharacter; }
	inline void setTargetCharacter(Character* pCharacter) { mTargetCharacter = pCharacter; }
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
	u32 mMinimumStatus = UINT32_MAX;

	std::list<String> mHelpMessages;
	Character* mInvoker = nullptr;
	bool mLogged = true;
	u8 mMinimumParameters = 0;
	u8 mMaximumParameters = UINT8_MAX;
	bool mRequiresTarget = false;
	Actor* mTargetActor = nullptr;
	bool mRequiresCharacterTarget = false;
	Character* mTargetCharacter = nullptr;
};

class CommandHandler {
public:
	~CommandHandler();
	const bool initialise(IDataStore* pDataStore);
	void command(Character* pInvoker, String pCommandMessage);

private:

	bool mInitialised = false;
	IDataStore* mDataStore = nullptr;

	const bool preExecute(Command* pCommand, Character* pInvoker, CommandParameters& pParameters) const;
	void _cleanUp(Command* pCommand);

	void _logCommand(Character* pCharacter, String pCommandMessage);
	Command* findCommand(String pCommandName);
	void _handleCommand(Character* pCharacter, const String& pCommandName, std::vector<String> pParameters);
	std::list<Command*> mCommands;
};