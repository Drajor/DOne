#pragma once

#include "Constants.h"
#include "Singleton.h"
#include "Vector3.h"

// For the #zonesearch command.
struct ZoneDataSearchEntry {
	ZoneID mID;
	String mShortName;
	String mLongName;
}; // NOTE: No initializer due to PoD rules.
typedef std::list<ZoneDataSearchEntry> ZoneDataSearchResults;

struct ZoneData;
struct SpawnPointData;
struct ZonePointData;
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

	const bool getSpawnPoints(const uint16 pZoneID, std::list<SpawnPointData*>** pSpawnPoints);
	const bool getZonePoints(const uint16 pZoneID, std::list<ZonePointData*>** pZonePoints);

	ZoneDataSearchResults searchByName(String pSearchText);
	const uint16 findFirstByName(const String& pSearchText);

private:

	bool mInitialised = false;
	ZoneData* _find(const uint16 pZoneID) const;
	std::list<ZoneData*> mZoneData;
};