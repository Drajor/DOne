#include "ZoneData.h"

std::string ZoneData::getLongName(uint32 pZoneID)
{
	return "North Qeynos";
}

std::string ZoneData::getShortName(uint32 pZoneID)
{
	return "qeynos2";
}

bool ZoneData::initialise()
{
	//ZoneData::mZoneData[757] = { 757, 0, "breedinggrounds", "The Breeding Grounds", 0.0f, 0.0f, 3.0f };
	//mZoneData[0] = {}

	return true;
}

uint32 ZoneData::getLongNameStringID(uint32 pZoneID)
{
	return 2244;
}
