#include "GroupManager.h"
#include "Character.h"
#include "Zone.h"
#include "ZoneClientConnection.h"
#include "LogSystem.h"

#include "../common/EQPacket.h"
#include "../common/eq_packet_structs.h"

/*
1414 %1 tells the group,%2 '%3'
1395 You gain party experience!!
1399 %1 has joined the group.
5040 You are now the leader of your group.
5041 %1 is now the leader of your group.
5042 %1 is not in your group.
5043 /makeleader
5044 The leader's group was disbanded before you accepted the invitation.



N(OP_GroupUpdate),
N(OP_GroupInvite),
N(OP_GroupDisband),
N(OP_GroupInvite2),
N(OP_GroupFollow),
N(OP_GroupFollow2),
N(OP_GroupCancelInvite),

N(OP_GroupAcknowledge),
N(OP_GroupDelete),

N(OP_GroupUpdateB),
N(OP_GroupDisbandYou),
N(OP_GroupDisbandOther),
N(OP_GroupLeaderChange),
N(OP_GroupLeadershipAAUpdate),
N(OP_GroupRoles),

N(OP_GroupMakeLeader),

N(OP_AssistGroup),

N(OP_GroupUpdateLeaderAA),
N(OP_DoGroupLeadershipAbility),
N(OP_SetGroupTarget),
*/

// Trying it out.
#define ARG_STR(pARG) #pARG
#define ARG_PTR_CHECK(pARG) if(pARG == nullptr) { std::stringstream ss; ss << "[ARG_PTR_CHECK] ("<< ARG_STR(pARG) << ") Failed in" << __FUNCTION__; Log::error(ss.str()); return; }
#define ARG_PTR_CHECK_BOOL(pARG) if(pARG == nullptr) { std::stringstream ss; ss << "[ARG_PTR_CHECK] ("<< ARG_STR(pARG) << ") Failed in" << __FUNCTION__; Log::error(ss.str()); return false; }

void GroupManager::makeGroup(Character* pLeader, Character* pMember) {
	ARG_PTR_CHECK(pLeader); ARG_PTR_CHECK(pMember);

	Group* group = new Group(pLeader, pMember);
	mGroups.push_back(group);
}

void GroupManager::removeMemberRequest(Character* pCharacter, Character* pRemoveCharacter) {
	ARG_PTR_CHECK(pCharacter); ARG_PTR_CHECK(pRemoveCharacter);
	
	// Check: Characters are in the same group.
	if (pCharacter->getGroup() != pRemoveCharacter->getGroup()) {
		Log::error("[Group Manager] Group mismatch in removeMemberRequest.");
		return;
	}

	// Character is removing them self from the group.
	if (pCharacter == pRemoveCharacter) {
		Group* group = pCharacter->getGroup();
		group->removeMember(pRemoveCharacter);

		// Tidy up if group is disbanded.
		if (group->mIsDisbanded) {
			mGroups.remove(group);
			delete group;
		}
	}
}

void GroupManager::chatSent(Character* pCharacter, const std::string pMessage) {
	for (auto i : pCharacter->getGroup()->mMembers) {
		// Check: If group member is currently zoning.
		if (i->isZoning()) {
			i->queueGroupMessage(pCharacter->getName(), pMessage);
			continue;
		}
		i->getConnection()->sendGroupChat(pCharacter->getName(), pMessage);
	}
}

Group::Group(Character* pLeader, Character* pMember) : mLeader(pLeader), mIsDisbanded(false) {
	ARG_PTR_CHECK(pLeader); ARG_PTR_CHECK(pMember);
	
	mMembers.push_back(pLeader);
	mMembers.push_back(pMember);
	pLeader->setGroup(this);
	pMember->setGroup(this);

	pLeader->getConnection()->sendGroupCreate();
	pLeader->getConnection()->sendGroupLeaderChange(pLeader->getName());
	pLeader->getConnection()->sendGroupAcknowledge();
	pLeader->getConnection()->sendGroupFollow(pLeader->getName(), pMember->getName());

	pMember->getConnection()->sendGroupAcknowledge();
	pLeader->getConnection()->sendGroupJoin(pMember->getName());

	std::list<std::string> memberNames;
	getMemberNames(memberNames, pMember->getName());
	pMember->getConnection()->sendGroupUpdate(memberNames);
}

Group::~Group() {

}

void Group::addMember(Character* pCharacter) {
	ARG_PTR_CHECK(pCharacter);
	
	// Check: Group is not already full.
	if (isFull()) {
		Log::error("[Group] Group is full and attempting to add another member.");
		return;
	}
	// Check: pCharacter is not already in this group.
	if (isMember(pCharacter)) {
		Log::error("[Group] Attempting to add character to a group that it is already a member of.");
		return;
	}
	// Check: pCharacter is not already grouped.
	if (pCharacter->hasGroup()) {
		Log::error("[Group] Attempting to add character to a group that is already grouped.");
		return;
	}

	mMembers.push_back(pCharacter);
	pCharacter->setGroup(this);

	// TODO: Update the rest of the group.
}

void Group::removeMember(Character* pCharacter) {
	ARG_PTR_CHECK(pCharacter);

	bool removed = false;
	for (auto i : mMembers) {
		if (i == pCharacter) {
			removed = true;
			break;
		}
	}

	if (!removed) {
		// TODO: Log
		return;
	}

	mMembers.remove(pCharacter);
	pCharacter->setGroup(nullptr);

	// Tell the leaving character that they left the group.
	pCharacter->getConnection()->sendGroupLeave(pCharacter->getName());

	// Tell the remaining members that pCharacter left the group.
	for (auto i : mMembers)
		i->getConnection()->sendGroupLeave(pCharacter->getName());
	
	// Check if group needs to disband.
	if (getNumMembers() == 1) {
		Character* lastMember = *mMembers.begin();
		mMembers.clear();

		lastMember->setGroup(nullptr);
		lastMember->getConnection()->sendGroupDisband();
		
		mIsDisbanded = true;
		return;
	}

	// Group leader is leaving.
	if (mLeader == pCharacter) {

	}
}

void Group::setLeader(Character* pCharacter) {
	ARG_PTR_CHECK(pCharacter);

	mLeader = pCharacter;
}

bool Group::isMember(Character* pCharacter) {
	ARG_PTR_CHECK_BOOL(pCharacter);

	for (auto i : mMembers) {
		if (i == pCharacter) return true;
	}

	return false;
}

void Group::getMemberNames(std::list<std::string>& pMemberNames, std::string pExcludeCharacterName) {
	for (auto i : mMembers) {
		if (i->getName() != pExcludeCharacterName)
			pMemberNames.push_back(i->getName());
	}
}
