#pragma once

#include "../common/types.h"
#include <string>

struct ClientAuthentication {
	ClientAuthentication() : mAccountID(0), mAccountName(""), mKey(""), mWorldAdmin(0), mIP(0), mLocal(0) {};
	uint32 mAccountID; // Login Server Account
	std::string mAccountName; // Login Server Account
	std::string mKey;
	int16 mWorldAdmin;
	uint32 mIP;
	uint8 mLocal;
};