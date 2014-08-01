#pragma once

#include <list>
#include <string>

class Character;

class Group {
	friend class GroupManager;

	Group(Character* pLeader, Character* pMember);
	~Group() {};

	void getMemberNames(std::list<std::string>& pMemberNames, std::string pExcludeCharacterName);
	bool isMember(Character* pCharacter);
	void addMember(Character* pCharacter);
	void removeMember(Character* pCharacter);
	bool hasLeader() { return mLeader != nullptr; }
	void sendMemberLeaveMessage(std::string pLeaverName);
	void sendGroupLeaderChange();
	bool needsDisbanding() { return mMembers.size() == 1; }

	Character* mLeader;
	std::list<Character*> mMembers;
};

class GroupManager {
public:
	void makeGroup(Character* pLeader, Character* pMember);

	// pCharacter is making the request to remove pRemoveCharacter.
	void removeMemberRequest(Character* pCharacter, Character* pRemoveCharacter);
	void handleGroupMessage(Character* pCharacter, const std::string pMessage);
	void handleCharacterLinkDead(Character* pCharacter);
	void handleCharacterCamped(Character* pCharacter);
	void handleMakeLeaderRequest(Character* pCharacter, Character* pNewLeader);
private:

	void _sendMessage(Group* pGroup, std::string pSenderName, std::string pMessage);
	void _disbandGroup(Group* pGroup);
	void _postMemberRemoval(Group* pGroup);
	
	std::list<Group*> mGroups;
};