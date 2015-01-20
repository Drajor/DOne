#include "ZoneData.h"
#include "Data.h"
#include "IDataStore.h"
#include "LogSystem.h"
#include "Vector3.h"
#include "Utility.h"

ZoneDataStore::~ZoneDataStore() {
	if (mLog) {
		delete mLog;
		mLog = nullptr;
	}

	for (auto i : mData) {
		delete i;
	}
	mData.clear();
}

const bool ZoneDataStore::initialise(IDataStore* pDataStore, ILogFactory* pLogFactory) {
	if (mInitialised) return false;
	if (!pLogFactory) return false;
	if (!pDataStore) return false;

	mDataStore = pDataStore;
	mLog = pLogFactory->make();

	mLog->setContext("[ZoneDataManager]");
	mLog->status("Initialising.");

	// Load data.
	if (!mDataStore->loadZones(mData)) {
		mLog->error("Failed to load data.");
		return false;
	}
	mLog->info("Loaded data for " + toString(mData.size()) + " Zones.");
	
	mLog->status("Finished initialising.");
	mInitialised = true;
	return true;
}

Data::Zone* ZoneDataStore::getData(const u16 pZoneID) const {
	for (auto i : mData) {
		if (pZoneID == i->mID)
			return i;
	}

	return nullptr;
}

ZoneDataSearchResults ZoneDataStore::searchByName(String pSearchText) {
	ZoneDataSearchResults results;
	for (auto i : mData) {
		if (Utility::findCI(i->mShortName, pSearchText) || Utility::findCI(i->mLongName, pSearchText)) {
			results.push_back({i->mID, i->mShortName, i->mLongName});
		}
	}

	return results;
}

const uint16 ZoneDataStore::findFirstByName(const String& pSearchText) {
	for (auto i : mData) {
		if (Utility::compareCI(i->mShortName, pSearchText) || Utility::compareCI(i->mLongName, pSearchText)) {
			return i->mID;
		}
	}

	return 0;
}

const bool ZoneDataStore::getSpawnGroups(const u16 pZoneID, std::list<Data::SpawnGroup*>& pSpawnGroupData) {
	auto zoneData = getData(pZoneID);
	EXPECTED_BOOL(zoneData);
	pSpawnGroupData = zoneData->mSpawnGroups;
	return true;
}

const bool ZoneDataStore::getSpawnPoints(const u16 pZoneID, std::list<Data::SpawnPoint*>& pSpawnPointData) {
	auto zoneData = getData(pZoneID);
	EXPECTED_BOOL(zoneData);
	pSpawnPointData = zoneData->mSpawnPoints;
	return true;
}

const bool ZoneDataStore::getZonePoints(const u16 pZoneID, std::list<Data::ZonePoint*>& pZonePoints) {
	auto zoneData = getData(pZoneID);
	EXPECTED_BOOL(zoneData);
	pZonePoints = zoneData->mZonePoints;
	return true;
}

const bool ZoneDataStore::getSafePoint(const u16 pZoneID, Vector3& pSafePoint) {
	auto zoneData = getData(pZoneID);
	EXPECTED_BOOL(zoneData);
	pSafePoint = zoneData->mSafePosition;
	return true;
}
