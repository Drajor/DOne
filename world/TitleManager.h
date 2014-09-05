#pragma once

#include "Constants.h"
#include "Singleton.h"

class Character;
struct Title;

class TitleManager : public Singleton<TitleManager> {
private:
	friend class Singleton<TitleManager>;
	TitleManager() {};
	~TitleManager();
	TitleManager(TitleManager const&); // Do not implement.
	void operator=(TitleManager const&); // Do not implement.
public:
	const bool initialise();
	const bool deinitialise();
	const std::list<const Title*> getTitles(Character* pCharacter);
	const String& getPrefix(const uint32 pTitleID) const;
	const String& getSuffix(const uint32 pTitleID) const;

private:
	bool mInitialised = false;

	std::list<Title*> mTitles;
};

struct Title {
	uint32 mID;
	String mPrefix;
	String mSuffix;
};