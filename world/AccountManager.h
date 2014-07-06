#pragma once

#include "../common/types.h"
#include <string>

/*
	Status
	-2	Banned
	-1	(Should be suspended however suspended accounts have the field 'suspendeduntil' set instead...)
	0	Normal
	>0	Guide..GM etc.
*/

struct AccountData {
	uint32 mID;
	uint32 mLoginServerAccountID;
	std::string mName;
	std::string mCharacterName;
	uint32 mSharedPlatinum;
	int32 mStatus;
	bool mGMSpeed;
	bool mHidden;
	// mSuspendUntil
};