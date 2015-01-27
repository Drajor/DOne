#pragma once

#include "Types.h"
#include <list>

class ILog;
class ILogFactory;
class ZoneManager;
class Character;
class Group;

class GroupManager {
public:

	~GroupManager();

	const bool initialise(ILogFactory* pLogFactory, ZoneManager* pZoneManager);

	// Character Events.
	void onEnterZone(Character* pCharacter);
	void onLeaveZone(Character* pCharacter);
	void onCamp(Character* pCharacter);
	void onLinkdead(Character* pCharacter);
	void onDeath(Character* pCharacter);

	const bool onMessage(Character* pCharacter, const String& pMessage);

	// Character is inviting another Character to join group.
	const bool onInvite(Character* pInviter, const String& pInviteeName);

	// Character has accepted a group invite.
	const bool onInviteAccept(Character* pCharacter);

	// Character has declined a group invite.
	const bool onInviteDecline(Character* pCharacter);

	// Character is leaving their group.
	const bool onLeave(Character* pCharacter);

	// Character is removing a member from the group.
	const bool onRemove(Character* pCharacter, const String& pTargetName);

	// Character is transferring group leadership.
	const bool onMakeLeader(Character* pCharacter, const String& pLeaderName);

	// Change is changing the role of a group member.
	const bool onRoleChange(Character* pCharacter, const String& pTargetName, const u32 pRoleID, const u8 pToggle);

private:

	const bool onCreate(Character* pLeader, Character* pMember);
	const bool onJoin(Group* pGroup, Character* pCharacter);

	void check(Group* pGroup);

	bool mInitialised = false;
	ILog* mLog = nullptr;
	ZoneManager* mZoneManager = nullptr;
	
	std::list<Group*> mGroups;
	u32 mNextID = 665;
};
