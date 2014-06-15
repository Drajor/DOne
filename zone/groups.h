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
	Group(Mob* pLeader);
	Group(uint32 pGroupID);
	~Group();

	bool addMember(Mob* pNewMember, const char* pNewMemberName = nullptr, uint32 pCharacterID = 0);
	void addMember(const char* pNewMemberName);
	void sendUpdate(uint32 pType, Mob* pMember);
	void sendLeadershipAAUpdate();
	bool delMemberOOZ(const char* pName);
	bool delMember(Mob* pOldMember, bool pIgnoreSender = false);
	void disbandGroup();
	bool isGroupMember(Mob* pClient);
	bool isGroupMember(const char* pName);
	bool process();
	bool isGroup() { return true; }
	void castGroupSpell(Mob* pCaster, uint16 pSpellID);
	void groupBardPulse(Mob* pCaster, uint16 pSpellID);
	void splitExp(uint32 pExp, Mob* pOther);
	void groupMessage(Mob* pSender, uint8 pLanguage, uint8 pLanguageSkill, const char* pMessage);
	void groupMessage_StringID(Mob* pSender, uint32 pType, uint32 pStringID, const char* pMessage, const char* pMessage2 = 0, const char* pMessage3 = 0, const char* pMessage4 = 0, const char* pMessage5 = 0, const char* pMessage6 = 0, const char* pMessage7 = 0, const char* pMessage8 = 0, const char* pMessage9 = 0, uint32 pDistance = 0);
	uint32 getTotalGroupDamage(Mob* pOther);
	void splitMoney(uint32 pCopper, uint32 pSilver, uint32 pGold, uint32 pPlatinum, Client* pSplitter = nullptr);
	inline void setLeader(Mob* pNewLeader){ mLeader = pNewLeader; };
	inline Mob* getLeader(){ return mLeader; };
	char* getLeaderName() { return mMemberNames[0]; };
	void sendHPPacketsTo(Mob* pNewMember);
	void sendHPPacketsFrom(Mob* pNewMember);
	bool updatePlayer(Mob* pUpdate);
	void memberZoned(Mob* pRemove);
	inline bool isLeader(Mob* pMOB) { return pMOB == mLeader; };
	uint8 groupCount();
	uint32 getHighestLevel();
	uint32 getLowestLevel();
	void queuePacket(const EQApplicationPacket* pApp, bool pAckReq = true);
	void teleportGroup(Mob* pSender, uint32 pZoneID, uint16 pInstanceID, float pX, float pY, float pZ, float pHeading);
	uint16 getAvgLevel();
	bool learnMembers();
	void verifyGroup();
	void balanceHP(int32 pPenalty, int32 pRange = 0, Mob* pCaster = nullptr);
	void balanceMana(int32 pPenalty, int32 pRange = 0, Mob* pCaster = nullptr);
	void healGroup(uint32 pAmount, Mob* pCaster, int32 pRange = 0);
	inline void setGroupAAs(GroupLeadershipAA_Struct* pFrom) { memcpy(&mLeaderAbilities, pFrom, sizeof(GroupLeadershipAA_Struct)); }
	inline void getGroupAAs(GroupLeadershipAA_Struct* pInto) { memcpy(pInto, &mLeaderAbilities, sizeof(GroupLeadershipAA_Struct)); }
	void updateGroupAAs();
	void saveGroupLeaderAA();
	void markNPC(Mob* pTarget, int pNumber);
	int8 getNumberNeedingHealing(int8 pHPRatio, bool pIncludePets);
	
	void delegateMainTank(const char* pNewMainTankName, uint8 pToggle = 0);
	void delegateMainAssist(const char* pNewMainAssistName, uint8 pToggle = 0);
	void delegatePuller(const char* pNewPullerName, uint8 pToggle = 0);
	void undelegateMainTank(const char* pOldMainTankName, uint8 pToggle = 0);
	void undelegateMainAssist(const char* pOldMainAssistName, uint8 pToggle = 0);
	void undelegatePuller(const char* pOldPullerName, uint8 pToggle = 0);

	bool isNPCMarker(Client* pClient);
	void setGroupAssistTarget(Mob* pMOB);
	void setGroupTankTarget(Mob* pMOB);
	void setGroupPullerTarget(Mob* pMOB);
	bool hasRole(Mob* pMOB, uint8 pRole);
	void notifyAssistTarget(Client* pClient);
	void notifyTankTarget(Client* pClient);
	void notifyPullerTarget(Client* pClient);
	void delegateMarkNPC(const char* pNewNPCMarkerName);
	void undelegateMarkNPC(const char* pOldNPCMarkerName);
	
	void notifyMainTank(Client* pClient, uint8 pToggle = 0);
	void notifyMainAssist(Client* pClient, uint8 pToggle = 0);
	void notifyPuller(Client* pClient, uint8 pToggle = 0);
	void notifyMarkNPC(Client* pClient);

	inline uint32 getNPCMarkerID() { return mNPCMarkerID; }
	void setMainTank(const char* pNewMainTankName);
	void setMainAssist(const char* pNewMainAssistName);
	void setPuller(const char* pNewPullerName);
	const char* getMainTankName() { return mMainTankName.c_str(); }
	const char* getMainAssistName() { return mMainAssistName.c_str(); }
	const char* getPullerName() { return mPullerName.c_str(); }
	void setNPCMarker(const char* pNewNPCMarkerName);
	void unMarkNPC(uint16 pID);
	void sendMarkedNPCsToMember(Client* pClient, bool pClear = false);
	inline int getLeadershipAA(int pAAID) { return mLeaderAbilities.ranks[pAAID]; }
	void clearAllNPCMarks();
	void queueHPPacketsForNPCHealthAA(Mob* pSender, const EQApplicationPacket* pApp);
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
