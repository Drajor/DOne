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

class ILog;
class ILogFactory;
class IDataStore;

namespace Data {
	struct Zone;
	struct SpawnGroup;
	struct SpawnPoint;
	struct ZonePoint;
}
struct Vector3;

class ZoneDataStore {
public:

	~ZoneDataStore();

	const bool initialise(IDataStore* pDataStore, ILogFactory* pLogFactory);
	
	const bool getSafePoint(const u16 pZoneID, Vector3& pSafePoint);
	const bool getSpawnGroups(const u16 pZoneID, std::list<Data::SpawnGroup*>& pSpawnGroupData);
	const bool getSpawnPoints(const u16 pZoneID, std::list<Data::SpawnPoint*>& pSpawnPointData);
	const bool getZonePoints(const u16 pZoneID, std::list<Data::ZonePoint*>& pZonePoints);

	ZoneDataSearchResults searchByName(String pSearchText);
	const uint16 findFirstByName(const String& pSearchText);

	Data::Zone* getData(const u16 pZoneID) const;
	inline std::list<Data::Zone*>& getData() { return mData; }
	
private:

	bool mInitialised = false;
	ILog* mLog = nullptr;
	IDataStore* mDataStore = nullptr;

	std::list<Data::Zone*> mData;
};