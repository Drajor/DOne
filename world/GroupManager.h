#pragma once

#include <list>
#include <string>

class Character;

class Group {
public:
	bool isFull() { return mMembers.size() == 6; }
	int getNumMembers() { return mMembers.size(); }
	bool isMember(Character* pCharacter);
	void getMemberNames(std::list<std::string>& pMemberNames, std::string pExcludeCharacterName);
private:
	friend class GroupManager;
	void addMember(Character* pCharacter);
	void removeMember(Character* pCharacter);
	void setLeader(Character* pCharacter);
	void sendMemberLeaveMessage(std::string pLeaverName);
	void sendGroupLeaderChange();
	bool needsDisbanding() { return mMembers.size() == 1; }
	Group(Character* pLeader, Character* pMember);
	~Group();

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
	void handleMakeLeaderRequest(Character* pCharacter, Character* pNewLeader);
private:

	void _sendMessage(Group* pGroup, std::string pSenderName, std::string pMessage);
	void _disbandGroup(Group* pGroup);
	
	std::list<Group*> mGroups;
};