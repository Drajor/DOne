#pragma once

#include "../common/types.h"
#include <string>

struct AccountData {
	uint32 mID;
	std::string mName;
	std::string mCharacterName;
	uint32 mSharedPlatinum;
	std::string mPassword;
	int32 mStatus;
	int mLoginServerAccountID;
	bool mGMSpeed;
	bool mRevoked;
	uint32 mKarma;
	bool mHidden;
};