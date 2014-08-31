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

struct ZonePoint {
	ZoneID mID = 0;
	Vector3 mPosition;
	float mHeading;

	Vector3 mDestinationPosition;
	float mDestinationHeading = 0.0f;

	ZoneID mDestinationZoneID = 0;
	InstanceID mDestinationInstanceID = 0;
};

class ZoneData : public Singleton<ZoneData> {
private:
	friend class Singleton<ZoneData>;
	ZoneData() {};
	~ZoneData();
	ZoneData(ZoneData const&); // Do not implement.
	void operator=(ZoneData const&); // Do not implement.
public:

	bool initialise();
	const bool getLongName(const ZoneID pZoneID, String& pLongName);
	const bool getShortName(const ZoneID pZoneID, String& pShortName);
	const bool getLongNameStringID(const ZoneID pZoneID, uint32& pStringID);

	ZoneDataSearchResults searchByName(String pSearchText);

private:
	struct ZoneInformation {
		ZoneID mID = 0;
		uint32 mLongNameStringID = 0;
		String mLongName = "";
		String mShortName = "";
		float mSafeX = 0.0f;
		float mSafeY = 0.0f;
		float mSafeZ = 0.0f;
		std::list<ZonePoint*> mZonePoints;
	};
	std::list<ZoneInformation*> mZoneInformation;
	ZoneInformation* _find(const ZoneID pZoneID) const;
};