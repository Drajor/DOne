#pragma once

#include <string>
#include "../common/types.h"

struct AccountData {
	uint32 mWorldAccountID;
	uint32 mLoginServerAccountID;
	std::string mName;
	std::string mCharacterName;
	uint32 mSharedPlatinum;
	int32 mStatus;
	bool mGMSpeed;
	bool mHidden;
	// mSuspendUntil
};