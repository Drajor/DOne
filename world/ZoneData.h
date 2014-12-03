#pragma once

#include "Types.h"
#include "Singleton.h"
#include <list>

// For the #zonesearch command.
struct ZoneDataSearchEntry {
	u16 mID;
	String mShortName;
	String mLongName;
}; // NOTE: No initializer due to PoD rules.
typedef std::list<ZoneDataSearchEntry> ZoneDataSearchResults;

namespace Data {
	struct Zone;
	struct SpawnPoint;
	struct ZonePoint;
}
struct Vector3;

class ZoneDataManager : public Singleton<ZoneDataManager> {
private:
	friend class Singleton<ZoneDataManager>;
	ZoneDataManager() {};
	~ZoneDataManager();
	ZoneDataManager(ZoneDataManager const&); // Do not implement.
	void operator=(ZoneDataManager const&); // Do not implement.
public:

	const bool initialise();
	const bool getLongName(const uint16 pZoneID, String& pLongName);
	const bool getShortName(const uint16 pZoneID, String& pShortName);
	const bool getLongNameStringID(const uint16 pZoneID, uint32& pStringID);
	const bool getSafePoint(const uint16 pZoneID, Vector3& pSafePoint);

	const bool getSpawnPoints(const uint16 pZoneID, std::list<Data::SpawnPoint*>& pSpawnPointData);
	const bool getZonePoints(const uint16 pZoneID, std::list<Data::ZonePoint*>** pZonePoints);

	ZoneDataSearchResults searchByName(String pSearchText);
	const uint16 findFirstByName(const String& pSearchText);

private:

	bool mInitialised = false;
	Data::Zone* _find(const uint16 pZoneID) const;
	std::list<Data::Zone*> mZoneData;
};