#pragma once

#include "Constants.h"

// For the #zonesearch command.
struct ZoneDataSearchEntry {
	ZoneID mID;
	String mShortName;
	String mLongName;
};
typedef std::list<ZoneDataSearchEntry> ZoneDataSearchResults;

struct ZonePoint {
	ZoneID mID;
	float mX;
	float mY;
	float mZ;
	float mHeading;

	float mDestinationX;
	float mDestinationY;
	float mDestinationZ;
	float mDestinationHeading;

	ZoneID mDestinationZoneID;
	InstanceID mDestinationInstanceID;
};

class ZoneData {
public:
	static ZoneData& getInstance() {
		static ZoneData instance;
		return instance;
	}

	bool initialise();
	String getLongName(ZoneID pZoneID);
	String getShortName(ZoneID pZoneID);
	std::uint32_t getLongNameStringID(ZoneID pZoneID);

	ZoneDataSearchResults searchByName(String pSearchText);

private:
	struct ZoneInformation {
		ZoneInformation() : mID(0), mLongNameStringID(0), mLongName(""), mShortName(""), mSafeX(0.0f), mSafeY(0.0f), mSafeZ(0.0f) {};
		ZoneID mID;
		std::uint32_t mLongNameStringID;
		String mLongName;
		String mShortName;
		float mSafeX;
		float mSafeY;
		float mSafeZ;
		std::list<ZonePoint*> mZonePoints;
	};
	std::list<ZoneInformation*> mZoneInformation;
	ZoneInformation* findZoneInformation(ZoneID pZoneID);

	ZoneData() {};
	~ZoneData();
	ZoneData(ZoneData const&);
	void operator=(ZoneData const&);
};