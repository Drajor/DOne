#pragma once

#include "Constants.h"
#include "ClientAuthentication.h"

class WorldClientConnection;
class Zone;
class Character;
struct WhoFilter;

struct ZoneSearchEntry {
	ZoneSearchEntry() : mName(""), mID(ZoneIDs::NoZone), mInstanceID(0), mNumCharacters(0) {};
	String mName;
	ZoneID mID;
	InstanceID mInstanceID;
	std::uint32_t mNumCharacters;
};
typedef std::list<ZoneSearchEntry> ZoneSearchResult;

class ZoneManager {
public:
	static ZoneManager& getInstance() {
		static ZoneManager instance;
		return instance;
	}
	bool initialise();

	ZoneSearchResult getAllZones();

	void addAuthentication(ClientAuthentication& pAuthentication, String pCharacterName, ZoneID pZoneID, uint32 pInstanceID = 0);
	
	void registerZoneTransfer(Character* pCharacter, ZoneID pZoneID, uint16 pInstanceID);
	Character* getZoningCharacter(String pCharacterName);

	
	void update();
	uint16 getZonePort(ZoneID pZoneID, uint32 pInstanceID = 0);
	void notifyCharacterChatTell(Character* pCharacter, const String& pTargetName, const String& pMessage);
	void notifyCharacterZoneOut(Character* pCharacter);
	void whoAllRequest(Character* pCharacter, WhoFilter& pFilter);
	Character* findCharacter(const String pCharacterName, bool pIncludeZoning = false, Zone* pExcludeZone = nullptr);
private:
	Zone* _makeZone(ZoneID pZoneID, uint32 pInstanceID = 0);
	uint32 _getNextZonePort();

	std::list<uint32> mAvailableZonePorts;
	std::list<Zone*> mZones;
	std::list<Character*> mZoningCharacters;

	ZoneManager();
	~ZoneManager();
	ZoneManager(ZoneManager const&);
	void operator=(ZoneManager const&);
};