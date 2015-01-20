#include "TitleManager.h"
#include "Data.h"
#include "IDataStore.h"
#include "LogSystem.h"
#include "Character.h"

TitleManager::~TitleManager() {
	for (auto i : mData)
		delete i;
	mData.clear();
}

const bool TitleManager::initialise(IDataStore* pDataStore, ILogFactory* pLogFactory) {
	if (mInitialised) return false;
	if (!pDataStore) return false;
	if (!pLogFactory) return false;

	mLog = pLogFactory->make();
	mDataStore = pDataStore;

	mLog->setContext("[TitleManager]");
	mLog->status("Initialising.");

	// Load data.
	if (!mDataStore->loadTitles(mData)) {
		return false;
	}

	mLog->info("Loaded data for " + std::to_string(mData.size()) + " Titles.");
	mLog->status("Finished initialising.");
	mInitialised = true;
	return true;
}

std::list<Data::Title*> TitleManager::getTitles(Character* pCharacter) {
	std::list<Data::Title*> availableTitles;

	for (auto i : mData)
		availableTitles.push_back(i);

	return availableTitles;
}

const String& TitleManager::getPrefix(const u32 pTitleID) const {
	for (auto i : mData) {
		if (i->mID == pTitleID)
			return i->mPrefix;
	}

	Log::error("Could not find Title in TitleManager::getPrefix with ID " + std::to_string(pTitleID));
	return EmptyString;
}

const String& TitleManager::getSuffix(const u32 pTitleID) const {
	for (auto i : mData) {
		if (i->mID == pTitleID)
			return i->mSuffix;
	}

	Log::error("Could not find Title in TitleManager::getSuffix with ID " + std::to_string(pTitleID));
	return EmptyString;
}
