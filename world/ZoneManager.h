#pragma once

#include "Constants.h"
#include "Singleton.h"
#include "ClientAuthentication.h"

class Zone;
class Character;
struct WhoFilter;

struct ZoneSearchEntry {
	String mName = "";
	ZoneID mID = ZoneIDs::NoZone;
	uint16 mInstanceID = 0;
	uint32 mNumCharacters = 0;
};
typedef std::list<ZoneSearchEntry> ZoneSearchResult;

class ZoneManager : public Singleton<ZoneManager> {
private:
	friend class Singleton<ZoneManager>;
	ZoneManager() {};
	~ZoneManager();
	ZoneManager(ZoneManager const&); // Do not implement.
	void operator=(ZoneManager const&); // Do not implement.
public:
	bool initialise();
	void update();

	// Returns whether or not the specified Zone is running.
	const bool isZoneRunning(const uint16 pZoneID, const uint16 pInstanceID) const;

	const bool isZoneAvailable(const uint16 pZoneID, const uint16 pInstanceID);

	const bool canZoneShutdown(const u16 pZoneID, const u16 pInstanceID) const;
	const bool canZoneShutdown(Zone* pZone) const;

	// Attempt to boot a Zone.
	const bool requestZoneBoot(const u16 pZoneID, const u16 pInstanceID);

	// Attempt to shutdown a Zone.
	const bool requestZoneShutdown(const u16 pZoneID, const u16 pInstanceID);

	ZoneSearchResult getAllZones();
	
	// Returns the port used by Zone (ID=pZoneID, Instance=pInstanceID) or 0 if the Zone was not found.
	const u16 getZonePort(const u16 pZoneID, const u16 pInstanceID) const;

	void handleTell(Character* pCharacter, const String& pTargetName, const String& pMessage);

	void handleWhoRequest(Character* pCharacter, const WhoFilter& pFilter, std::list<Character*>& pResults);
	Character* findCharacter(const String pCharacterName, bool pIncludeZoning = false, Zone* pExcludeZone = nullptr);

	// Character Zoning.
	void addZoningCharacter(Character* pCharacter);
	const bool removeZoningCharacter(const String& pCharacterName);
	const bool hasZoningCharacter(const uint32 pAccountID);
	Character* getZoningCharacter(const String& pCharacterName);
	String getZoningCharacterName(const uint32 pAccountID);
private:

	bool mInitialised = false;
	Zone* _search(const uint16 pZoneID, const uint16 pInstanceID) const;
	const bool _makeZone(const u16 pZoneID, const u16 pInstanceID);
	const u16 _getNextZonePort();
	std::list<u16> mAvailableZonePorts;
	std::list<Zone*> mZones;
	std::list<Character*> mZoningCharacters;
};