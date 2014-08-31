#pragma once

#include "Constants.h"
#include "Singleton.h"

class Character;
class Zone;
class Group;

class GroupManager : public Singleton<GroupManager> {
private:
	friend class Singleton<GroupManager>;
	GroupManager() {};
	~GroupManager() {};
	GroupManager(GroupManager const&); // Do not implement.
	void operator=(GroupManager const&); // Do not implement.
public:

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
	void makeGroup(Character* pLeader, Character* pMember);
	void joinGroup(Group* pGroup, Character* pCharacter);

	bool _isLeader(Character* pCharacter);
	void _disbandGroup(Group* pGroup);
	void _postMemberRemoval(Group* pGroup);

	void _sendMessage(Group* pGroup, String pSenderName, String pMessage);
	void _sendZoneMessage(Group* pGroup, Zone* pZone, String pSenderName, String pMessage, Character* pExcludeCharacter = nullptr);
	
	std::list<Group*> mGroups;
};

class Group {
	friend class GroupManager;

	Group(Character* pLeader, Character* pMember);
	~Group() {};

	void sendGroupUpdate(Character* pCharacter);
	void sendGroupUpdate();
	void sendMemberLeaveMessage(String pLeaverName);
	void sendGroupLeaderChange();

	// Adds a Character to the Group.
	void add(Character* pCharacter);
	// Removes a Character from the group.
	void remove(Character* pCharacter);

	void getMemberNames(std::list<String>& pMemberNames, String pExcludeCharacterName);
	bool isMember(Character* pCharacter);
	bool hasLeader() { return mLeader != nullptr; }
	bool needsDisbanding() { return mMembers.size() == 1; }

	Character* getMember(const String& pCharacterName);
	Character* getLeader() { return mLeader; }
	Character* mLeader;
	std::list<Character*> mMembers;
};