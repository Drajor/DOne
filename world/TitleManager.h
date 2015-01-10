#pragma once

#include "Types.h"
#include <list>

class DataStore;
class Character;

struct Title {
	u32 mID = 0;
	String mPrefix;
	String mSuffix;
};

class TitleManager {
public:
	~TitleManager();

	const bool initialise(DataStore* pDataStore);
	const bool deinitialise();

	const std::list<const Title*> getTitles(Character* pCharacter);
	const String& getPrefix(const u32 pTitleID) const;
	const String& getSuffix(const u32 pTitleID) const;

private:

	bool mInitialised = false;
	DataStore* mDataStore = nullptr;

	std::list<Title*> mTitles;
};