#include "Settings.h"

String Settings::mServerShortName = "";
String Settings::mServerLongName = "";
bool Settings::mLocked = false;
String Settings::mLoginServerAccountName = "";
String Settings::mLoginServerPassword = "";
String Settings::mLoginServerAddress = "";
uint16 Settings::mLoginServerPort = 0;

const bool Settings::_setServerShortName(const String& pServerShortName) {
	mServerShortName = pServerShortName;
	return true;
}

const bool Settings::_setServerLongName(const String& pServerLongName) {
	mServerLongName = pServerLongName;
	return true;
}

const bool Settings::_setLSAccountName(const String& pLSAccountName) {
	mLoginServerAccountName = pLSAccountName;
	return true;
}

const bool Settings::_setLSPassword(const String& pLSPassword) {
	mLoginServerPassword = pLSPassword;
	return true;
}
