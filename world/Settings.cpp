#include "Settings.h"
#include "Utility.h"
#include "Limits.h"

String Settings::mServerShortName = "";
String Settings::mServerLongName = "";
bool Settings::mLocked = false;
String Settings::mLoginServerAccountName = "";
String Settings::mLoginServerPassword = "";
String Settings::mLoginServerAddress = "";
uint16 Settings::mLoginServerPort = 0;

const bool Settings::_setServerShortName(const String& pServerShortName) {
	EXPECTED_BOOL(Limits::World::shortNameLength(pServerShortName));
	mServerShortName = pServerShortName;
	return true;
}

const bool Settings::_setServerLongName(const String& pServerLongName) {
	EXPECTED_BOOL(Limits::World::longNameLength(pServerLongName));
	mServerLongName = pServerLongName;
	return true;
}

const bool Settings::_setLSAccountName(const String& pLSAccountName) {
	EXPECTED_BOOL(Limits::World::lsAccountNameLength(pLSAccountName));
	mLoginServerAccountName = pLSAccountName;
	return true;
}

const bool Settings::_setLSPassword(const String& pLSPassword) {
	EXPECTED_BOOL(Limits::World::lsAccountPasswordLength(pLSPassword));
	mLoginServerPassword = pLSPassword;
	return true;
}
