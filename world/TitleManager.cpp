#include "TitleManager.h"
#include "Utility.h"
#include "Character.h"
#include "Profile.h"

TitleManager::~TitleManager() {
	deinitialise();
}

const bool TitleManager::initialise() {
	EXPECTED_BOOL(mInitialised == false);

	Profile p("TitleManager::initialise");
	Log::status("[Title Manager] Initialising.");

	Title* a = new Title();
	a->mID = 1;
	a->mPrefix = "Apples";
	a->mSuffix = "Oranges";
	mTitles.push_back(a);

	Title* b = new Title();
	b->mID = 2;
	b->mPrefix = "Jam";
	mTitles.push_back(b);

	Log::info("[Title Manager] Loaded data for " + std::to_string(mTitles.size()) + " Titles.");
	mInitialised = true;
	return true;
}

const bool TitleManager::deinitialise() {
	for (auto i : mTitles)
		delete i;
	mTitles.clear();

	return true;
}

const std::list<const Title*> TitleManager::getTitles(Character* pCharacter) {
	std::list<const Title*> availableTitles;

	for (auto i : mTitles)
		availableTitles.push_back(i);

	return availableTitles;
}

const String& TitleManager::getPrefix(const uint32 pTitleID) const {
	for (auto i : mTitles) {
		if (i->mID == pTitleID)
			return i->mPrefix;
	}

	Log::error("Could not find Title in TitleManager::getPrefix with ID " + std::to_string(pTitleID));
	return EmptyString;
}

const String& TitleManager::getSuffix(const uint32 pTitleID) const {
	for (auto i : mTitles) {
		if (i->mID == pTitleID)
			return i->mSuffix;
	}

	Log::error("Could not find Title in TitleManager::getSuffix with ID " + std::to_string(pTitleID));
	return EmptyString;
}
