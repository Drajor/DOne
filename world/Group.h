#pragma once

#include "Types.h"
#include <list>

class GroupManager;
class Character;

class Group {
	friend class GroupManager;
public:
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
	inline Character* getLeader() { return mLeader; }

private:
	Character* mLeader;
	std::list<Character*> mMembers;
};