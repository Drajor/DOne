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

	const bool isZoneAvailable(const uint16 pZoneID, const uint16 pInstanceID);

	ZoneSearchResult getAllZones();
	
	// Returns the port used by Zone (ID=pZoneID, Instance=pInstanceID) or 0 if the Zone was not found.
	const uint16 getZonePort(const uint16 pZoneID, const uint16 pInstanceID);

	void handleTell(Character* pCharacter, const String& pTargetName, const String& pMessage);

	void whoAllRequest(Character* pCharacter, WhoFilter& pFilter);
	Character* findCharacter(const String pCharacterName, bool pIncludeZoning = false, Zone* pExcludeZone = nullptr);

	// Character Zoning.
	void addZoningCharacter(Character* pCharacter);
	const bool removeZoningCharacter(const String& pCharacterName);
	const bool hasZoningCharacter(const uint32 pAccountID);
	Character* getZoningCharacter(const String& pCharacterName);
	String getZoningCharacterName(const uint32 pAccountID);
private:
	Zone* _search(const uint16 pZoneID, const uint16 pInstanceID);
	const bool _makeZone(const uint16 pZoneID, const uint16 pInstanceID);
	const uint32 _getNextZonePort();
	std::list<uint32> mAvailableZonePorts;
	std::list<Zone*> mZones;
	std::list<Character*> mZoningCharacters;
};