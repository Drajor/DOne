#pragma once

#include <list>

class Character;

class Group {
public:
	bool isFull() { return mCharacters.size() == 6; }
	bool isMember(Character* pCharacter);
private:
	friend class GroupManager;
	void addMember(Character* pCharacter);
	void setLeader(Character* pCharacter);
	Group();
	~Group();

	Character* mLeader;
	std::list<Character*> mCharacters;
};

class GroupManager {
public:
	void makeGroup(Character* pLeader, Character* pMember);
private:
	std::list<Group*> mGroups;
};