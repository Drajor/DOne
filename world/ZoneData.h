#pragma once

// Home for static zone data.

#include "Constants.h"
#include "../common/types.h"
#include <string>
#include <cstdint>
#include <list>

/*
For each zone, the client has a lits of hard-coded zone lines.
*/
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
	std::uint16_t mDestinationInstanceID;
};

struct ZoneDataSearchEntry {
	ZoneID mID;
	std::string mShortName;
	std::string mLongName;
};

typedef std::list<ZoneDataSearchEntry> ZoneDataSearchResults;

class ZoneData {
public:
	static ZoneData& getInstance() {
		static ZoneData instance;
		return instance;
	}

	bool initialise();
	bool isZoneIDValid(ZoneID pZoneID);
	std::string getLongName(ZoneID pZoneID);
	std::string getShortName(ZoneID pZoneID);
	uint32 getLongNameStringID(ZoneID pZoneID);

	ZoneDataSearchResults searchByName(std::string pName);

private:
	struct ZoneInformation {
		ZoneInformation() : mID(0), mLongNameStringID(0), mLongName(""), mShortName(""), mSafeX(0.0f), mSafeY(0.0f), mSafeZ(0.0f) {};
		ZoneID mID;
		uint32 mLongNameStringID;
		std::string mLongName;
		std::string mShortName;
		float mSafeX;
		float mSafeY;
		float mSafeZ;
		std::list<ZonePoint*> mZonePoints;
	};
	std::list<ZoneInformation*> mZoneInformation;
	ZoneInformation* findZoneInformation(ZoneID pZoneID);

	ZoneData() {};
	~ZoneData() {};
	ZoneData(ZoneData const&);
	void operator=(ZoneData const&);
};