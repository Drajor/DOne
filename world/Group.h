#pragma once

#include "Types.h"
#include <list>

namespace GroupRole {
	enum : u32 {
		MainTank = 1,
		MainAssist = 2,
		Puller = 3,
	};
}


class Character;
class Zone;
class EQApplicationPacket;

class Group {
public:
	Group(const u32 pID) : mID(pID) {};
	inline const u32 getID() const { return mID; }

	void sendMessage(const String& pSenderName, const String& pMessage, Character* pExclude = nullptr);

	void onCreate(Character* pLeader, Character* pMember);
	void onJoin(Character* pCharacter);
	void onLeave(Character* pCharacter);
	void onRemove(Character* pCharacter);
	void onRoleChange(Character * pCharacter, const u32 pRoleID, const bool pToggle);
	void onMakeLeader(Character * pCharacter);

	void onDisband();

	Character* getMember(const String& pCharacterName) const;

	// Returns the leader of the Group.
	inline Character* getLeader() const { return mLeader; }

	// Returns whether or not the specified Character is the leader of the Group.
	inline const bool isLeader(Character* pCharacter) const { return mLeader == pCharacter; }
	
	// Returns the number of members currently in the Group.
	inline const u32 numMembers() const { return mMembers.size(); }

	// Returns whether or not the Group is full.
	inline const bool isFull() const { return numMembers() == 6; }

	void sendPacket(const EQApplicationPacket* pPacket) const;
	void sendPacket(const EQApplicationPacket* pPacet, Zone* pZone, Character* pExcludeCharacter = nullptr) const;

	// Returns the Group members.
	inline std::list<Character*>& getMembers() { return mMembers; }

	void onEnterZone(Character* pCharacter);
	void onLeaveZone(Character* pCharacter);
private:

	// Sends updated (group) data to a specific Character.
	void sendGroup(Character* pCharacter);

	// Sends updated (group) data to the whole Group.
	void sendGroup();

	// Sends leadership abilities to a specific Character.
	void sendLeadershipAbilities(Character* pCharacter);

	// Sends leadership abilities to the entire Group.
	void sendLeadershipAbilities();

	const u32 mID;
	Character* mLeader = nullptr;
	std::list<Character*> mMembers;

	void setMainTank(Character* pCharacter, const bool pValue);
	void setMainAssist(Character* pCharacter, const bool pValue);
	void setPuller(Character* pCharacter, const bool pValue);
};
