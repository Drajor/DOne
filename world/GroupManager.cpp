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

void GroupManager::makeGroup(Character* pLeader, Character* pMember) {
	Group* group = new Group();
	group->addMember(pLeader);
	group->addMember(pMember);
	group->setLeader(pLeader);

	// Notify leader(inviter) that a group has been created.
	pLeader->getConnection()->sendGroupCreate();
	// Notify leader(inviter) that they are the group leader.
	pLeader->getConnection()->sendGroupLeaderChange(pLeader->getName());
	// Ping?
	pLeader->getConnection()->sendGroupAcknowledge();
	// Notify leader(inviter) that the member(invitee) has agreed to group.
	pLeader->getConnection()->sendGroupFollow(pLeader->getName(), pMember->getName());
	// Ping?
	pMember->getConnection()->sendGroupAcknowledge();

	pLeader->getConnection()->sendGroupJoin(pMember->getName());
	//pMember->getConnection()->sendGroupJoin(pLeader->getName());

	//auto hpPacket = new EQApplicationPacket(OP_MobHealth, sizeof(SpawnHPUpdate_Struct2));
	//auto hpPayload = reinterpret_cast<SpawnHPUpdate_Struct2*>(hpPacket->pBuffer);
	//
	//hpPayload->spawn_id = pLeader->getSpawnID();
	//hpPayload->hp = 30;

	//pMember->getConnection()->sendPacket(hpPacket);
	//safe_delete(hpPacket);

	std::list<std::string> memberNames;
	group->getMemberNames(memberNames, pMember->getName());
	pMember->getConnection()->sendGroupUpdate(memberNames);

	//app->SetOpcode(OP_MobHealth);
	//app->size = sizeof(SpawnHPUpdate_Struct2);
}

Group::Group() : mLeader(nullptr) {

}

Group::~Group() {

}

void Group::addMember(Character* pCharacter) {
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
}

void Group::setLeader(Character* pCharacter) {
	mLeader = pCharacter;
}

bool Group::isMember(Character* pCharacter) {
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
