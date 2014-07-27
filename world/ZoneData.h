#pragma once

// Home for static zone data.

#include "../common/types.h"
#include <string>

class ZoneData {
public:
	static bool initialise();
	static std::string getLongName(uint32 pZoneID);
	static std::string getShortName(uint32 pZoneID);
	static uint32 getLongNameStringID(uint32 pZoneID);
private:
	struct ZoneInformation {
		uint32 mID;
		uint32 mLongNameStringID;
		std::string mLongName;
		std::string mShortName;
		float mSafeX;
		float mSafeY;
		float mSafeZ;
	};
	static ZoneInformation mZoneData[1000];
	ZoneData() {};
	~ZoneData() {};
};