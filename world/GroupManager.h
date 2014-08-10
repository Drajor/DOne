#pragma once

#include "Constants.h"

class Character;

class Group {
	friend class GroupManager;

	Group(Character* pLeader, Character* pMember);
	~Group() {};

	void getMemberNames(std::list<String>& pMemberNames, String pExcludeCharacterName);
	bool isMember(Character* pCharacter);
	void addMember(Character* pCharacter);
	void removeMember(Character* pCharacter);
	bool hasLeader() { return mLeader != nullptr; }
	void sendMemberLeaveMessage(String pLeaverName);
	void sendGroupLeaderChange();
	bool needsDisbanding() { return mMembers.size() == 1; }

	Character* mLeader;
	std::list<Character*> mMembers;
};

class GroupManager {
public:
	static GroupManager& getInstance() {
		static GroupManager instance;
		return instance;
	}

	void makeGroup(Character* pLeader, Character* pMember);

	// pCharacter is making the request to remove pRemoveCharacter.
	void removeMemberRequest(Character* pCharacter, Character* pRemoveCharacter);
	void handleGroupMessage(Character* pCharacter, const String pMessage);
	void handleCharacterLinkDead(Character* pCharacter);
	void handleCharacterCamped(Character* pCharacter);
	void handleMakeLeaderRequest(Character* pCharacter, Character* pNewLeader);
private:

	void _sendMessage(Group* pGroup, String pSenderName, String pMessage);
	void _disbandGroup(Group* pGroup);
	void _postMemberRemoval(Group* pGroup);
	
	std::list<Group*> mGroups;

	GroupManager() {};
	~GroupManager() {};
	GroupManager(GroupManager const&);
	void operator=(GroupManager const&);
};