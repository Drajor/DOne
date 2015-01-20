#pragma once

#include "Types.h"
#include <list>

namespace Data {
	struct Title;
}

class ILog;
class ILogFactory;
class IDataStore;
class Character;

class TitleManager {
public:
	~TitleManager();

	const bool initialise(IDataStore* pDataStore, ILogFactory* pLog);

	std::list<Data::Title*> getTitles(Character* pCharacter);

	const String& getPrefix(const u32 pTitleID) const;
	const String& getSuffix(const u32 pTitleID) const;

private:

	bool mInitialised = false;
	ILog* mLog = nullptr;
	IDataStore* mDataStore = nullptr;

	std::list<Data::Title*> mData;
};