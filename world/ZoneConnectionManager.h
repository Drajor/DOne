#pragma once

#include "Types.h"
#include <list>
#include <map>

class EQStreamFactory;
class EQStreamIdentifier;
class ZoneConnection;
class Zone;
class ILogFactory;
class ILog;
class ZoneManager;
class GroupManager;
class RaidManager;
class GuildManager;

class ZoneConnectionManager {
public:
	~ZoneConnectionManager();

	const bool initialise(const u16 pPort, Zone* pZone, ILogFactory* pLogFactory, ZoneManager* pZoneManager, GroupManager* pGroupManager, RaidManager* pRaidManager, GuildManager* pGuildManager);
	void update();

	const std::size_t numPreConnections() const { return mPreConnections.size(); }
	const std::size_t numActiveConnections() const { return mActiveConnections.size(); }
	const std::size_t numLinkDeadConnections() const { return mLinkDeadConnections.size(); }
	const std::size_t numConnections() { return numPreConnections() + numActiveConnections() + numLinkDeadConnections(); }

private:

	bool mInitialised = false;
	ILog* mLog = nullptr;
	ILogFactory* mLogFactory = nullptr;
	Zone* mZone = nullptr;
	ZoneManager* mZoneManager = nullptr;
	GroupManager* mGroupManager = nullptr;
	RaidManager* mRaidManager = nullptr;
	GuildManager* mGuildManager = nullptr;

	u16 mPort = 0;
	EQStreamFactory* mStreamFactory = nullptr;
	EQStreamIdentifier* mStreamIdentifier = nullptr;

	std::list<ZoneConnection*> mPreConnections;
	std::list<ZoneConnection*> mActiveConnections;
	std::map<i64, ZoneConnection*> mLinkDeadConnections;

	void checkIncomingConnections();
	void updatePreConnections();
	void updateActiveConnections();
	void checkLinkdeadConnections();
};