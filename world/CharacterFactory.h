#pragma once

#include "Types.h"

class ILog;
class ILogFactory;
class IDataStore;
class ItemFactory;
class TaskDataStore;
class Account;
class Character;

class CharacterFactory {
public:
	~CharacterFactory();

	const bool initialise(IDataStore* pDataStore, ILogFactory* pLogFactory, ItemFactory* pItemFactory, TaskDataStore* pTaskDataStore);
	SharedPtr<Character> make(const String& pCharacterName, SharedPtr<Account> pAccount);

private:

	bool mInitialised = false;
	ILog* mLog = nullptr;
	ILogFactory* mLogFactory = nullptr;
	IDataStore* mDataStore = nullptr;
	ItemFactory* mItemFactory = nullptr;
	TaskDataStore* mTaskDataStore = nullptr;
};