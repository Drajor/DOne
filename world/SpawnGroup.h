#pragma once

#include "Types.h"

class SpawnGroup {
public:
	const u32 getID() const { return mID; }
private:
	u32 mID = 0;
};