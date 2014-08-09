#include "GroupManager.h"
#include "Utility.h"
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

static const int MAX_NUM_GROUP_MEMBERS = 6;

void GroupManager::makeGroup(Character* pLeader, Character* pMember) {
	ARG_PTR_CHECK(pLeader);
	ARG_PTR_CHECK(pMember);
	ERROR_CONDITION(!(pLeader->hasGroup() || pMember->hasGroup())); // Both Characters must not have groups.

	mGroups.push_back(new Group(pLeader, pMember));
}

void GroupManager::removeMemberRequest(Character* pCharacter, Character* pRemoveCharacter) {
	ARG_PTR_CHECK(pCharacter);
	ARG_PTR_CHECK(pRemoveCharacter);
	ERROR_CONDITION(pCharacter->getGroup() && pRemoveCharacter->getGroup()); // Both Characters must have valid groups.
	ERROR_CONDITION((pCharacter->getGroup() == pRemoveCharacter->getGroup())); // Both Characters must be in the same group.
	
	// Character is removing them self from the group.
	if (pCharacter == pRemoveCharacter) {
		Group* group = pCharacter->getGroup();
		group->removeMember(pRemoveCharacter);
		pRemoveCharacter->getConnection()->sendGroupLeave(pRemoveCharacter->getName()); // Notify Character leaving.
		group->sendMemberLeaveMessage(pRemoveCharacter->getName()); // Notify remaining group members.

		_postMemberRemoval(group);
	}
	// One Character is trying to remove another.
	else {

	}
}

void GroupManager::handleCharacterLinkDead(Character* pCharacter) {
	ARG_PTR_CHECK(pCharacter);
	ERROR_CONDITION(pCharacter->getGroup()); // Character must have a valid group.

	Group* group = pCharacter->getGroup();
	group->removeMember(pCharacter);
	_sendMessage(group, "[System]", pCharacter->getName() + " has gone LD."); // Notify group of LD.
	group->sendMemberLeaveMessage(pCharacter->getName()); // Notify remaining group members.

	_postMemberRemoval(group);
}

void GroupManager::handleCharacterCamped(Character* pCharacter) {
	ARG_PTR_CHECK(pCharacter);
	ERROR_CONDITION(pCharacter->getGroup()); // Character must have a valid group.

	Group* group = pCharacter->getGroup();
	group->removeMember(pCharacter);
	group->sendMemberLeaveMessage(pCharacter->getName()); // Notify remaining group members.

	_postMemberRemoval(group);
}

void GroupManager::_disbandGroup(Group* pGroup) {
	ARG_PTR_CHECK(pGroup);
	ERROR_CONDITION(pGroup->mMembers.size() == 1); // Only disband groups with one member left.

	Character* lastMember = *pGroup->mMembers.begin();
	pGroup->mMembers.clear();

	lastMember->setGroup(nullptr);
	lastMember->getConnection()->sendGroupDisband();

	mGroups.remove(pGroup);
	delete pGroup;
}

void GroupManager::handleGroupMessage(Character* pCharacter, const String pMessage) {
	ARG_PTR_CHECK(pCharacter);
	ERROR_CONDITION(pCharacter->getGroup()); // Character must have a valid group.

	_sendMessage(pCharacter->getGroup(), pCharacter->getName(), pMessage);
}

void GroupManager::_sendMessage(Group* pGroup, String pSenderName, String pMessage) {
	ARG_PTR_CHECK(pGroup);

	for (auto i : pGroup->mMembers) {
		// Check: Where a group member is zoning, queue the message.
		if (i->isZoning()) {
			i->addQueuedMessage(ChannelID::CH_GROUP, pSenderName, pMessage);
			continue;
		}
		i->getConnection()->sendGroupMessage(pSenderName, pMessage);
	}
}

void GroupManager::handleMakeLeaderRequest(Character* pCharacter, Character* pNewLeader) {
	ARG_PTR_CHECK(pCharacter);
	ARG_PTR_CHECK(pNewLeader);
	ERROR_CONDITION(pCharacter->getGroup() && pNewLeader->getGroup()); // Both Characters must have valid groups.
	ERROR_CONDITION((pCharacter->getGroup() == pNewLeader->getGroup())); // Both Characters must be in the same group.
	ERROR_CONDITION((pCharacter == pCharacter->getGroup()->mLeader)); // The requesting Character is the group leader.

	Group* group = pCharacter->getGroup();
	group->mLeader = pNewLeader;
	group->sendGroupLeaderChange();
}

void GroupManager::_postMemberRemoval(Group* pGroup) {
	ARG_PTR_CHECK(pGroup);

	// Check: Group needs disbanding.
	if (pGroup->needsDisbanding()) {
		_disbandGroup(pGroup);
		return;
	}

	// Check: Group needs a new leader.
	if (!pGroup->hasLeader()) {
		pGroup->mLeader = *pGroup->mMembers.begin();
		pGroup->sendGroupLeaderChange();
	}
}

Group::Group(Character* pLeader, Character* pMember) : mLeader(pLeader) {
	ARG_PTR_CHECK(pLeader); ARG_PTR_CHECK(pMember);
	// NOTE: Error Conditions are ignored here as CTOR is private.
	
	addMember(pLeader);
	addMember(pMember);

	pLeader->getConnection()->sendGroupCreate();
	pLeader->getConnection()->sendGroupLeaderChange(pLeader->getName());
	pLeader->getConnection()->sendGroupAcknowledge();
	pLeader->getConnection()->sendGroupFollow(pLeader->getName(), pMember->getName());

	pMember->getConnection()->sendGroupAcknowledge();
	pLeader->getConnection()->sendGroupJoin(pMember->getName());

	std::list<String> memberNames;
	getMemberNames(memberNames, pMember->getName());
	pMember->getConnection()->sendGroupUpdate(memberNames);
}

void Group::addMember(Character* pCharacter) {
	ARG_PTR_CHECK(pCharacter);
	ERROR_CONDITION((pCharacter->getGroup() == nullptr)); // Check: Character does not have a group.
	ERROR_CONDITION((mMembers.size() < MAX_NUM_GROUP_MEMBERS)); // Check: Group is not already full.

	mMembers.push_back(pCharacter);
	pCharacter->setGroup(this);

	// TODO: Update the rest of the group.
}

void Group::removeMember(Character* pCharacter) {
	ARG_PTR_CHECK(pCharacter);
	ERROR_CONDITION((pCharacter->getGroup() == this)); // Check: Pointer matching (sanity).
	ERROR_CONDITION(isMember(pCharacter)); // Check: Character is already a member of this group.

	mMembers.remove(pCharacter);
	pCharacter->setGroup(nullptr);

	// Group leader is leaving.
	if (mLeader == pCharacter) mLeader = nullptr;
}

void Group::sendMemberLeaveMessage(String pLeaverName) {
	// TODO: Zoning members.
	for (auto i : mMembers)
		i->getConnection()->sendGroupLeave(pLeaverName);
}

void Group::sendGroupLeaderChange() {
	ERROR_CONDITION(mLeader); // Check: mLeader pointer is valid.

	// TODO: Zoning members.
	for (auto i : mMembers)
		i->getConnection()->sendGroupLeaderChange(mLeader->getName());
}

bool Group::isMember(Character* pCharacter) {
	ARG_PTR_CHECK_BOOL(pCharacter);

	for (auto i : mMembers) {
		if (i == pCharacter) return true;
	}

	return false;
}

void Group::getMemberNames(std::list<String>& pMemberNames, String pExcludeCharacterName) {
	for (auto i : mMembers) {
		if (i->getName() != pExcludeCharacterName)
			pMemberNames.push_back(i->getName());
	}
}


