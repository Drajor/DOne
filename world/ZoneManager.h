#pragma once

#include "Types.h"

#include <list>

class ILog;
class ILogFactory;
class NPCFactory;
class ZoneDataStore;
class TaskDataStore;
class GroupManager;
class RaidManager;
class GuildManager;
class TitleManager;
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
	u16 mID = 0;
	u16 mInstanceID = 0;
	u32 mNumCharacters = 0;
};
typedef std::list<ZoneSearchEntry> ZoneSearchResult;

class ZoneManager {
public:
	~ZoneManager();

	const bool initialise(World* pWorld, ZoneDataStore* pZoneDataManager, TaskDataStore* pTaskDataStore, GroupManager* pGroupManager, RaidManager* pRaidManager, GuildManager* pGuildManager, TitleManager* pTitleManager, CommandHandler* pCommandHandler, ItemFactory* pItemFactory, ILogFactory* pLogFactory, NPCFactory* pNPCFactory);
	void update();
	const bool saveCharacter(Character* pCharacter);

	// Guild Events.
	void onCreateGuild();
	void onDeleteGuild();

	// Returns whether or not the specified Zone is running.
	const bool isZoneRunning(const u16 pZoneID, const u16 pInstanceID) const;

	// Returns whether or not the specified Zone is available for entry. Will only return false when the Zone is in the process of shutting down.
	const bool isZoneAvailable(const u16 pZoneID, const u16 pInstanceID);

	// Returns whether or not the specified Zone can be shutdown. Will return false if there are any Characters in the Zone or if there are any Characters zoning into the Zone.
	const bool canZoneShutdown(const u16 pZoneID, const u16 pInstanceID) const;

	// Attempt to boot a Zone.
	const bool requestZoneBoot(const u16 pZoneID, const u16 pInstanceID);

	// Attempt to shutdown a Zone.
	const bool requestZoneShutdown(const u16 pZoneID, const u16 pInstanceID);

	ZoneSearchResult getAllZones();
	
	// Returns the port used by Zone (ID=pZoneID, Instance=pInstanceID) or 0 if the Zone was not found.
	const u16 getZonePort(const u16 pZoneID, const u16 pInstanceID) const;

	void handleTell(Character* pCharacter, const String& pTargetName, const String& pMessage);

	void handleWhoRequest(Character* pCharacter, const WhoFilter& pFilter, std::list<Character*>& pResults);
	Character* findCharacter(const String& pCharacterName, bool pIncludeZoning = false, Zone* pExcludeZone = nullptr) const;
	Character* findZoningCharacter(const String& pCharacterName) const;

	bool onEnterZone(Character* pCharacter);
	bool onLeaveZone(Character* pCharacter);
	bool onLeaveWorld(Character* pCharacter);

	Character* getZoningCharacter(const String& pCharacterName);

private:

	const bool _canZoneShutdown(Zone* pZone) const;

	bool mInitialised = false;
	ILog* mLog = nullptr;
	ILogFactory* mLogFactory = nullptr;
	World* mWorld = nullptr;
	ZoneDataStore* mZoneDataStore = nullptr;
	TaskDataStore* mTaskDataStore = nullptr;
	GroupManager* mGroupManager = nullptr;
	RaidManager* mRaidManager = nullptr;
	GuildManager* mGuildManager = nullptr;
	TitleManager* mTitleManager = nullptr;
	CommandHandler* mCommandHandler = nullptr;
	ItemFactory* mItemFactory = nullptr;
	NPCFactory* mNPCFactory = nullptr;
	Experience::Calculator* mExperienceCalculator = nullptr;

	Zone* _search(const u16 pZoneID, const u16 pInstanceID) const;
	const bool _makeZone(const u16 pZoneID, const u16 pInstanceID);
	const u16 _getNextZonePort();
	std::list<u16> mAvailableZonePorts;
	std::list<Zone*> mZones;

	// List of Characters that are currently zoning
	std::list<Character*> mZoningCharacters;
};