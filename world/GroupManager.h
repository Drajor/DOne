#pragma once

#include <list>
#include <string>

class Character;

class Group {
public:
	bool isFull() { return mMembers.size() == 6; }
	bool isMember(Character* pCharacter);
	void getMemberNames(std::list<std::string>& pMemberNames, std::string pExcludeCharacterName);
private:
	friend class GroupManager;
	void addMember(Character* pCharacter);
	void setLeader(Character* pCharacter);
	Group();
	~Group();

	Character* mLeader;
	std::list<Character*> mMembers;
};

class GroupManager {
public:
	void makeGroup(Character* pLeader, Character* pMember);
private:
	std::list<Group*> mGroups;
};