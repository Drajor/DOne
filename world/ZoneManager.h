#pragma once

#include "Constants.h"

#include <list>
#include <memory>

class ILog;
class ILogFactory;
class ZoneDataManager;
class GroupManager;
class RaidManager;
class GuildManager;
class CommandHandler;
class ItemFactory;
class Zone;
class Character;
class World;
struct WhoFilter;

namespace Experience {
	class Calculator;
}

struct ZoneSearchEntry {
	String mName = "";
	u16 mID = ZoneIDs::NoZone;
	u16 mInstanceID = 0;
	u32 mNumCharacters = 0;
};
typedef std::list<ZoneSearchEntry> ZoneSearchResult;

class ZoneManager {
public:
	~ZoneManager();

	const bool initialise(World* pWorld, ZoneDataManager* pZoneDataManager, GroupManager* pGroupManager, RaidManager* pRaidManager, GuildManager* pGuildManager, CommandHandler* pCommandHandler, ItemFactory* pItemFactory, ILogFactory* pLogFactory);
	void update();

	// Returns whether or not the specified Zone is running.
	const bool isZoneRunning(const u16 pZoneID, const u16 pInstanceID) const;

	const bool isZoneAvailable(const u16 pZoneID, const u16 pInstanceID);

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
	Character* findCharacter(const String pCharacterName, bool pIncludeZoning = false, Zone* pExcludeZone = nullptr) const;

	void onEnterZone(Character* pCharacter);
	void onLeaveZone(Character* pCharacter);
	void onLeaveWorld(Character* pCharacter);

	Character* getZoningCharacter(const String& pCharacterName);

private:

	bool mInitialised = false;
	ILog* mLog = nullptr;
	ILogFactory* mLogFactory = nullptr;
	World* mWorld = nullptr;
	ZoneDataManager* mZoneDataManager = nullptr;
	GroupManager* mGroupManager = nullptr;
	RaidManager* mRaidManager = nullptr;
	GuildManager* mGuildManager = nullptr;
	CommandHandler* mCommandHandler = nullptr;
	ItemFactory* mItemFactory = nullptr;
	Experience::Calculator* mExperienceCalculator = nullptr;

	Zone* _search(const u16 pZoneID, const u16 pInstanceID) const;
	const bool _makeZone(const u16 pZoneID, const u16 pInstanceID);
	const u16 _getNextZonePort();
	std::list<u16> mAvailableZonePorts;
	std::list<Zone*> mZones;
	std::list<Character*> mZoningCharacters;
};