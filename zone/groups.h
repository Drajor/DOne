/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
	*/
#ifndef GROUPS_H
#define GROUPS_H

#include "../common/types.h"
#include "../common/linked_list.h"
#include "../common/emu_opcodes.h"
#include "../common/eq_packet_structs.h"
#include "entity.h"
#include "mob.h"
#include "../common/features.h"
#include "../common/servertalk.h"

#define MAX_MARKED_NPCS 3

enum { RoleAssist = 1, RoleTank = 2, RolePuller = 4 };

class GroupIDConsumer {
public:
	GroupIDConsumer() { id = 0; }
	GroupIDConsumer(uint32 gid) { id = gid; }
	inline const uint32 GetID()	const { return id; }

protected:
	friend class EntityList;
	//use of this function is highly discouraged
	inline void SetID(uint32 set_id) { id = set_id; }
private:
	uint32 id;
};

class Group : public GroupIDConsumer {
public:
	Group(Mob* leader);
	Group(uint32 gid);
	~Group();

	bool addMember(Mob* newmember, const char* NewMemberName = nullptr, uint32 CharacterID = 0);
	void addMember(const char* NewMemberName);
	void sendUpdate(uint32 type, Mob* member);
	void sendLeadershipAAUpdate();
	void sendWorldGroup(uint32 zone_id, Mob* zoningmember);
	bool delMemberOOZ(const char *Name);
	bool delMember(Mob* oldmember, bool ignoresender = false);
	void disbandGroup();
	bool isGroupMember(Mob* client);
	bool isGroupMember(const char *Name);
	bool process();
	bool isGroup()			{ return true; }
	void castGroupSpell(Mob* caster, uint16 spellid);
	void groupBardPulse(Mob* caster, uint16 spellid);
	void splitExp(uint32 exp, Mob* other);
	void groupMessage(Mob* sender, uint8 language, uint8 lang_skill, const char* message);
	void groupMessage_StringID(Mob* sender, uint32 type, uint32 string_id, const char* message, const char* message2 = 0, const char* message3 = 0, const char* message4 = 0, const char* message5 = 0, const char* message6 = 0, const char* message7 = 0, const char* message8 = 0, const char* message9 = 0, uint32 distance = 0);
	uint32 getTotalGroupDamage(Mob* other);
	void splitMoney(uint32 copper, uint32 silver, uint32 gold, uint32 platinum, Client *splitter = nullptr);
	inline void setLeader(Mob* newleader){ mLeader = newleader; };
	inline Mob* getLeader(){ return mLeader; };
	char* getLeaderName() { return mMemberNames[0]; };
	void sendHPPacketsTo(Mob* newmember);
	void sendHPPacketsFrom(Mob* newmember);
	bool updatePlayer(Mob* update);
	void memberZoned(Mob* removemob);
	inline bool isLeader(Mob* leadertest) { return leadertest == mLeader; };
	uint8 groupCount();
	uint32 getHighestLevel();
	uint32 getLowestLevel();
	void queuePacket(const EQApplicationPacket *app, bool ack_req = true);
	void teleportGroup(Mob* sender, uint32 zoneID, uint16 instance_id, float x, float y, float z, float heading);
	uint16 getAvgLevel();
	bool learnMembers();
	void verifyGroup();
	void balanceHP(int32 penalty, int32 range = 0, Mob* caster = nullptr);
	void balanceMana(int32 penalty, int32 range = 0, Mob* caster = nullptr);
	void healGroup(uint32 heal_amt, Mob* caster, int32 range = 0);
	inline void setGroupAAs(GroupLeadershipAA_Struct *From) { memcpy(&mLeaderAbilities, From, sizeof(GroupLeadershipAA_Struct)); }
	inline void getGroupAAs(GroupLeadershipAA_Struct *Into) { memcpy(Into, &mLeaderAbilities, sizeof(GroupLeadershipAA_Struct)); }
	void updateGroupAAs();
	void saveGroupLeaderAA();
	void markNPC(Mob* Target, int Number);
	int8 getNumberNeedingHealedInGroup(int8 hpr, bool includePets);
	void delegateMainTank(const char *NewMainAssistName, uint8 toggle = 0);
	void delegateMainAssist(const char *NewMainAssistName, uint8 toggle = 0);
	void delegatePuller(const char *NewMainAssistName, uint8 toggle = 0);
	void undelegateMainTank(const char *OldMainAssistName, uint8 toggle = 0);
	void undelegateMainAssist(const char *OldMainAssistName, uint8 toggle = 0);
	void undelegatePuller(const char *OldMainAssistName, uint8 toggle = 0);
	bool isNPCMarker(Client *c);
	void setGroupAssistTarget(Mob *m);
	void setGroupTankTarget(Mob *m);
	void setGroupPullerTarget(Mob *m);
	bool hasRole(Mob *m, uint8 Role);
	void notifyAssistTarget(Client *c);
	void notifyTankTarget(Client *c);
	void notifyPullerTarget(Client *c);
	void delegateMarkNPC(const char *NewNPCMarkerName);
	void unDelegateMarkNPC(const char *OldNPCMarkerName);
	void notifyMainTank(Client *c, uint8 toggle = 0);
	void notifyMainAssist(Client *c, uint8 toggle = 0);
	void notifyPuller(Client *c, uint8 toggle = 0);
	void notifyMarkNPC(Client *c);
	inline uint32 getNPCMarkerID() { return mNPCMarkerID; }
	void setMainTank(const char *NewMainTankName);
	void setMainAssist(const char *NewMainAssistName);
	void setPuller(const char *NewPullerName);
	const char* getMainTankName() { return mMainTankName.c_str(); }
	const char* getMainAssistName() { return mMainAssistName.c_str(); }
	const char* getPullerName() { return mPullerName.c_str(); }
	void setNPCMarker(const char *NewNPCMarkerName);
	void unMarkNPC(uint16 ID);
	void sendMarkedNPCsToMember(Client *c, bool Clear = false);
	inline int getLeadershipAA(int AAID) { return mLeaderAbilities.ranks[AAID]; }
	void clearAllNPCMarks();
	void queueHPPacketsForNPCHealthAA(Mob* sender, const EQApplicationPacket* app);
	void changeLeader(Mob* pNewLeader);
	const char* getClientNameByIndex(uint8 pIndex);
	void updateXTargetMarkedNPC(uint32 pNumber, Mob* pMOB);

	Mob* mMembers[MAX_GROUP_MEMBERS];// TODO: Fix the laziness that requires this to be public.
	char mMemberNames[MAX_GROUP_MEMBERS][64]; // TODO: Fix the laziness that requires this to be public.

private:

	uint8 mMemberRoles[MAX_GROUP_MEMBERS];
	bool mDisbandCheck;
	bool mCastSpell;
	Mob* mLeader;
	GroupLeadershipAA_Struct mLeaderAbilities;
	std::string	mMainTankName;
	std::string	mMainAssistName;
	std::string	mPullerName;
	std::string	mNPCMarkerName;
	uint16	mNPCMarkerID;
	uint16	mAssistTargetID;
	uint16	mTankTargetID;
	uint16	mPullerTargetID;
	uint16	mMarkedNPCs[MAX_MARKED_NPCS];

};

#endif
