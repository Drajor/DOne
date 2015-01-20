#pragma once

#include "Types.h"

class ILog;
class ILogFactory;
class IDataStore;
class ItemFactory;
class Account;
class Character;

class CharacterFactory {
public:
	~CharacterFactory();

	const bool initialise(IDataStore* pDataStore, ILogFactory* pLogFactory, ItemFactory* pItemFactory);
	Character* make(const String& pCharacterName, Account* pAccount);

private:

	bool mInitialised = false;
	ILog* mLog = nullptr;
	ILogFactory* mLogFactory = nullptr;
	IDataStore* mDataStore = nullptr;
	ItemFactory* mItemFactory = nullptr;
};