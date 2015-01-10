#pragma once

#include "Types.h"
#include <list>

// For the #zonesearch command.
struct ZoneDataSearchEntry {
	u16 mID;
	String mShortName;
	String mLongName;
}; // NOTE: No initializer due to PoD rules.
typedef std::list<ZoneDataSearchEntry> ZoneDataSearchResults;

class IDataStore;

namespace Data {
	struct Zone;
	struct SpawnGroup;
	struct SpawnPoint;
	struct ZonePoint;
}
struct Vector3;

class ZoneDataManager {
public:
	const bool initialise(IDataStore* pDataStore);
	
	const bool load(const u16 pZoneID);
	const bool unload(const u16 pZoneID);
	const bool reload(const u16 pZoneID);
	const bool save(const u16 pZoneID);

	const bool getLongName(const u16 pZoneID, String& pLongName);
	const bool getShortName(const u16 pZoneID, String& pShortName);
	const bool getLongNameStringID(const u16 pZoneID, u32& pStringID);
	const bool getSafePoint(const u16 pZoneID, Vector3& pSafePoint);

	const bool getSpawnGroups(const u16 pZoneID, std::list<Data::SpawnGroup*>& pSpawnGroupData);
	const bool getSpawnPoints(const u16 pZoneID, std::list<Data::SpawnPoint*>& pSpawnPointData);
	const bool getZonePoints(const u16 pZoneID, std::list<Data::ZonePoint*>& pZonePoints);

	ZoneDataSearchResults searchByName(String pSearchText);
	const uint16 findFirstByName(const String& pSearchText);

	Data::Zone* getZoneData(const u16 pZoneID) const;
	inline std::list<Data::Zone*>& getZoneData() { return mZoneData; }
	
private:

	bool mInitialised = false;
	IDataStore* mDataStore = nullptr;
	std::list<Data::Zone*> mZoneData;
};