#pragma once

#include "Types.h"
#include <list>

class ZoneManager;
class Character;
class Zone;
class Group;

class GroupManager {
public:
	const bool initialise(ZoneManager* pZoneManager);

	// Character Packet Events.
	void handleInviteSent(Character* pCharacter, String pInviteName);
	void handleAcceptInvite(Character* pCharacter, String pInviterName);
	void handleDeclineInvite(Character* pCharacter, String pInviterName);
	void handleDisband(Character* pCharacter, String pRemoveName);
	void handleMakeLeader(Character* pCharacter, const String& pLeaderName);
	void handleMessage(Character* pCharacter, const String pMessage);
	
	// Character Events.
	void onEnterZone(Character* pCharacter);
	void onLeaveZone(Character* pCharacter);
	void onCamp(Character* pCharacter);
	void onLinkdead(Character* pCharacter);
	void onDeath(Character* pCharacter);

private:

	bool mInitialised = false;
	ZoneManager* mZoneManager = nullptr;

	void makeGroup(Character* pLeader, Character* pMember);
	void joinGroup(Group* pGroup, Character* pCharacter);

	bool _isLeader(Character* pCharacter);
	void _disbandGroup(Group* pGroup);
	void _postMemberRemoval(Group* pGroup);

	void _sendMessage(Group* pGroup, String pSenderName, String pMessage);
	void _sendZoneMessage(Group* pGroup, Zone* pZone, String pSenderName, String pMessage, Character* pExcludeCharacter = nullptr);
	
	std::list<Group*> mGroups;
};
