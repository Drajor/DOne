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
	InstanceID mInstanceID = 0;
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

	ZoneSearchResult getAllZones();

	void addAuthentication(ClientAuthentication& pAuthentication, String pCharacterName, ZoneID pZoneID, uint32 pInstanceID = 0);
	
	void registerZoneTransfer(Character* pCharacter, ZoneID pZoneID, uint16 pInstanceID);
	Character* getZoningCharacter(String pCharacterName);

	
	void update();
	uint16 getZonePort(ZoneID pZoneID, uint32 pInstanceID = 0);
	void notifyCharacterChatTell(Character* pCharacter, const String& pTargetName, const String& pMessage);
	void onLeaveZone(Character* pCharacter);
	void whoAllRequest(Character* pCharacter, WhoFilter& pFilter);
	Character* findCharacter(const String pCharacterName, bool pIncludeZoning = false, Zone* pExcludeZone = nullptr);
private:
	Zone* _makeZone(const ZoneID pZoneID, const uint32 pInstanceID = 0);
	const uint32 _getNextZonePort();

	std::list<uint32> mAvailableZonePorts;
	std::list<Zone*> mZones;
	std::list<Character*> mZoningCharacters;
};