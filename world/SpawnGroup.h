#pragma once

#include "Types.h"
#include <map>

class SpawnGroup {
public:
	SpawnGroup(u32 pID) : mID(pID) {};
	const u32 getID() const { return mID; }
	void add(const u32 pNPCTypeID, const u32 pChance) { mEntries.insert(std::make_pair(pNPCTypeID, pChance)); }
	const u32 roll();
private:
	u32 mID = 0;
	std::map<u32, u32> mEntries;
};