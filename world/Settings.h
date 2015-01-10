#pragma once

#include "Types.h"

class Settings {
public:
	static inline const String& getServerShortName() { return mServerShortName; }
	static inline const String& getServerLongName() { return mServerLongName; }
	static inline const bool getLocked() { return mLocked; }
	static inline const String& getLSAccountName() { return mLoginServerAccountName; }
	static inline const String& getLSPassword() { return mLoginServerPassword; }
	static inline const String& getLSAddress() { return mLoginServerAddress; }
	static inline const u16 getLSPort() { return mLoginServerPort; }
	static inline const u16 getUCSPort() { return mUCSPort; }
	static inline const bool getValidationEnabled() { return mValidationEnabled; }

	static const bool _setServerShortName(const String& pServerShortName);
	static const bool _setServerLongName(const String& pServerLongName);
	static inline const bool _setLocked(const bool pLocked) { mLocked = pLocked; return true; }
	static const bool _setLSAccountName(const String& pLSAccountName);
	static const bool _setLSPassword(const String& pLSPassword);
	static inline const bool _setLSAddress(const String& pLSAddress) { mLoginServerAddress = pLSAddress; return true; }
	static inline const bool _setLSPort(const u16 pLSPort) { mLoginServerPort = pLSPort; return true; }
	static inline const bool _setUCSPort(const u16 pUCSPort) { mUCSPort = pUCSPort; return true; }
	static inline void _setValidationEnabled(const bool pEnabled) { mValidationEnabled = pEnabled; }

private:

	static String mServerShortName;
	static String mServerLongName;
	static bool mLocked;

	static String mLoginServerAccountName;
	static String mLoginServerPassword;
	static String mLoginServerAddress;
	static u16 mLoginServerPort;

	static u16 mUCSPort;

	static bool mValidationEnabled;
};