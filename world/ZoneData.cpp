#include "ZoneData.h"
#include "ServiceLocator.h"
#include "DataStore.h"
#include "Data.h"
#include "Utility.h"
#include "LogSystem.h"
#include "Vector3.h"

const bool ZoneDataManager::initialise(IDataStore* pDataStore) {
	EXPECTED_BOOL(mInitialised == false);
	EXPECTED_BOOL(pDataStore);

	Log::status("[Zone Data] Initialising.");

	mDataStore = pDataStore;

	EXPECTED_BOOL(load(0));
	Log::info("[Zone Data] Loaded data for " + std::to_string(mZoneData.size()) + " Zones.");
	
	mInitialised = true;
	return true;
}

const bool ZoneDataManager::save(const u16 pZoneID) {
	// TODO: Make this zone specific.
	EXPECTED_BOOL(mDataStore->saveZones(mZoneData));
	return true;
}

const bool ZoneDataManager::load(const u16 pZoneID) {
	EXPECTED_BOOL(mZoneData.empty());
	EXPECTED_BOOL(mDataStore->loadZones(mZoneData));

	return true;
}

const bool ZoneDataManager::unload(const u16 pZoneID) {
	for (auto i : mZoneData) {
		delete i;
	}
	mZoneData.clear();

	return true;
}

const bool ZoneDataManager::reload(const u16 pZoneID) {
	EXPECTED_BOOL(save(pZoneID));
	EXPECTED_BOOL(unload(pZoneID));
	EXPECTED_BOOL(load(pZoneID));

	return true;
}


Data::Zone* ZoneDataManager::getZoneData(const u16 pZoneID) const {
	for (auto i : mZoneData) {
		if (pZoneID == i->mID)
			return i;
	}

	return nullptr;
}

const bool ZoneDataManager::getLongName(const u16 pZoneID, String& pLongName) {
	auto zoneData = getZoneData(pZoneID);
	EXPECTED_BOOL(zoneData);
	pLongName = zoneData->mLongName;
	return true;
}

const bool ZoneDataManager::getShortName(const u16 pZoneID, String& pShortName){
	auto zoneData = getZoneData(pZoneID);
	EXPECTED_BOOL(zoneData);
	pShortName = zoneData->mShortName;
	return true;
}

const bool ZoneDataManager::getLongNameStringID(const u16 pZoneID, u32& pStringID) {
	auto zoneData = getZoneData(pZoneID);
	EXPECTED_BOOL(zoneData);
	pStringID = zoneData->mLongNameStringID;
	return true;
}

ZoneDataSearchResults ZoneDataManager::searchByName(String pSearchText) {
	ZoneDataSearchResults results;
	for (auto i : mZoneData) {
		if (Utility::findCI(i->mShortName, pSearchText) || Utility::findCI(i->mLongName, pSearchText)) {
			results.push_back({i->mID, i->mShortName, i->mLongName});
		}
	}

	return results;
}

const uint16 ZoneDataManager::findFirstByName(const String& pSearchText) {
	for (auto i : mZoneData) {
		if (Utility::compareCI(i->mShortName, pSearchText) || Utility::compareCI(i->mLongName, pSearchText)) {
			return i->mID;
		}
	}

	return 0;
}

const bool ZoneDataManager::getSpawnGroups(const u16 pZoneID, std::list<Data::SpawnGroup*>& pSpawnGroupData) {
	auto zoneData = getZoneData(pZoneID);
	EXPECTED_BOOL(zoneData);
	pSpawnGroupData = zoneData->mSpawnGroups;
	return true;
}

const bool ZoneDataManager::getSpawnPoints(const u16 pZoneID, std::list<Data::SpawnPoint*>& pSpawnPointData) {
	auto zoneData = getZoneData(pZoneID);
	EXPECTED_BOOL(zoneData);
	pSpawnPointData = zoneData->mSpawnPoints;
	return true;
}

const bool ZoneDataManager::getZonePoints(const u16 pZoneID, std::list<Data::ZonePoint*>& pZonePoints) {
	auto zoneData = getZoneData(pZoneID);
	EXPECTED_BOOL(zoneData);
	pZonePoints = zoneData->mZonePoints;
	return true;
}

const bool ZoneDataManager::getSafePoint(const u16 pZoneID, Vector3& pSafePoint) {
	auto zoneData = getZoneData(pZoneID);
	EXPECTED_BOOL(zoneData);
	pSafePoint = zoneData->mSafePosition;
	return true;
}
