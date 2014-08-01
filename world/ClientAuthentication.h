#pragma once

#include "../common/types.h"
#include <string>

struct ClientAuthentication {
	ClientAuthentication() : mWorldAccountID(0), mLoginServerAccountID(0), mLoginServerAccountName(""), mKey(""), mWorldAdmin(0), mIP(0), mLocal(0) {};
	uint32 mWorldAccountID;
	uint32 mLoginServerAccountID;
	std::string mLoginServerAccountName;
	std::string mKey;
	int16 mWorldAdmin;
	uint32 mIP;
	uint8 mLocal;
};