#include "GroupManager.h"
#include "ZoneManager.h"
#include "Utility.h"
#include "Character.h"
#include "Zone.h"
#include "ZoneClientConnection.h"
#include "LogSystem.h"

#include "../common/EQPacket.h"
#include "../common/eq_packet_structs.h"

void GroupManager::makeGroup(Character* pLeader, Character* pMember) {
	EXPECTED(pLeader);
	EXPECTED(pMember);
	EXPECTED(!(pLeader->hasGroup() || pMember->hasGroup())); // Both Characters must not have groups.

	mGroups.push_back(new Group(pLeader, pMember));
}

void GroupManager::joinGroup(Group* pGroup, Character* pCharacter) {
	EXPECTED(pGroup);
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->hasGroup() == false);

	// UNTESTED!

	pCharacter->getConnection()->sendGroupAcknowledge();

	pGroup->add(pCharacter);
	pGroup->sendGroupUpdate();
}

void GroupManager::_disbandGroup(Group* pGroup) {
	EXPECTED(pGroup);
	EXPECTED(pGroup->mMembers.size() == 1); // Only disband groups with one member left.

	Character* lastMember = *pGroup->mMembers.begin();
	pGroup->mMembers.clear();

	lastMember->setGroup(nullptr);
	lastMember->getConnection()->sendGroupDisband();

	mGroups.remove(pGroup);
	delete pGroup;
}

void GroupManager::handleMessage(Character* pCharacter, const String pMessage) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->getGroup());

	_sendMessage(pCharacter->getGroup(), pCharacter->getName(), pMessage);
}

void GroupManager::_sendMessage(Group* pGroup, String pSenderName, String pMessage) {
	EXPECTED(pGroup);

	for (auto i : pGroup->mMembers) {
		// Check: Where a group member is zoning, queue the message.
		if (i->isZoning()) {
			i->addQueuedMessage(ChannelID::CH_GROUP, pSenderName, pMessage);
			continue;
		}
		i->getConnection()->sendGroupMessage(pSenderName, pMessage);
	}
}

void GroupManager::_sendZoneMessage(Group* pGroup, Zone* pZone, String pSenderName, String pMessage, Character* pExcludeCharacter) {
	EXPECTED(pGroup);
	EXPECTED(pZone);

	// NOTE: Zoning members will have a null zone pointer.
	for (auto i : pGroup->mMembers) {
		if (i->getZone() == pZone && i != pExcludeCharacter)
			i->getConnection()->sendGroupMessage(SYS_NAME, pMessage);
	}
}

void GroupManager::handleMakeLeader(Character* pCharacter, const String& pLeaderName) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->hasGroup());
	EXPECTED(_isLeader(pCharacter)); // Check: Initiator is group leader.

	// Try to find the Character being invited.
	auto character = ZoneManager::getInstance().findCharacter(pLeaderName);
	if (!character) { return; }

	EXPECTED(character->getGroup() == pCharacter->getGroup()); // Check: Both Characters are in the same group.

	Group* group = pCharacter->getGroup();
	group->mLeader = character;
	group->sendGroupLeaderChange();
}

void GroupManager::_postMemberRemoval(Group* pGroup) {
	EXPECTED(pGroup);

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

void GroupManager::handleInviteSent(Character* pCharacter, String pInviteName) {
	EXPECTED(pCharacter);

	// Try to find the Character being invited.
	auto character = ZoneManager::getInstance().findCharacter(pInviteName);
	if (!character) {
		pCharacter->getConnection()->sendMessage(MessageType::Red, "Player " + pInviteName + " was not found.");
		return;
	}
	// Check if the Character being invited already has a group.
	if (character->hasGroup()) {
		// TODO: Check which message is sent.
		pCharacter->getConnection()->sendMessage(MessageType::Red, "Player " + pInviteName + " already has a group.");
		return;
	}

	// Send the invite.
	character->getConnection()->sendGroupInvite(pCharacter->getName());
}

void GroupManager::handleAcceptInvite(Character* pCharacter, String pInviterName) {
	EXPECTED(pCharacter);

	// Try to find the Character who invited.
	auto character = ZoneManager::getInstance().findCharacter(pInviterName);
	if (!character) { return; } // This is really edge case and may never occur.

	// Starting a new group.
	if (!character->hasGroup()) {
		makeGroup(character, pCharacter);
	}
	// Joining an existing group.
	else {
		joinGroup(character->getGroup(), pCharacter);
	}
}

void GroupManager::handleDeclineInvite(Character* pCharacter, String pInviterName) {
	EXPECTED(pCharacter);
	
	// Try to find the Character who invited.
	auto character = ZoneManager::getInstance().findCharacter(pInviterName);

	if (!character) { return; } // Ignore.
	if (character->isZoning()) { return; } // Ignore.

	character->message(MessageType::White, pCharacter->getName() + " rejects your offer to join the group.");
}

void GroupManager::handleDisband(Character* pCharacter, String pRemoveName) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->hasGroup());

	Group* group = pCharacter->getGroup();
	const bool removerIsLeader = _isLeader(pCharacter);

	// Try to find the Character to remove.
	Character* character = pCharacter->getGroup()->getMember(pRemoveName);
	if (!character) {
		// Not a member of the group.
		return;
	}

	// Character is currently zoning.
	if (character->isZoning()) {
		// Ignore for now. // TODO:
		return;
	}
	// Removing self.
	if (pCharacter == character) {
		group->remove(character);
		character->getConnection()->sendGroupLeave(character->getName()); // Notify Character leaving.
		group->sendMemberLeaveMessage(character->getName()); // Notify remaining group members.

		_postMemberRemoval(group); // Disband or change leader if required.
		return;
	}
	// Removing other member.
	else {
		EXPECTED(removerIsLeader);

		group->remove(character);
		character->getConnection()->sendGroupLeave(character->getName()); // Notify Character leaving.
		group->sendMemberLeaveMessage(character->getName()); // Notify remaining group members.

		_postMemberRemoval(group); // Disband or change leader if required.
		return;
	}
}

bool GroupManager::_isLeader(Character* pCharacter) {
	EXPECTED_BOOL(pCharacter);
	return pCharacter->getGroup()->getLeader() == pCharacter;
}

void GroupManager::onEnterZone(Character* pCharacter) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->hasGroup());
	EXPECTED(pCharacter->getZone()); // Check: Sanity

	// Send the Character group member names.
	Group* group = pCharacter->getGroup();
	group->sendGroupUpdate(pCharacter);

	// Notify group members in the same zone that pCharacter has entered.
	_sendZoneMessage(group, pCharacter->getZone(), SYS_NAME, pCharacter->getName() + " has entered the zone.", pCharacter);
}

void GroupManager::onLeaveZone(Character* pCharacter) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->hasGroup());
	EXPECTED(pCharacter->getZone()); // Check: Sanity

	// Notify group members in the same zone that pCharacter has left.
	_sendZoneMessage(pCharacter->getGroup(), pCharacter->getZone(), SYS_NAME, pCharacter->getName() + " has left the zone.", pCharacter);
}

void GroupManager::onCamp(Character* pCharacter) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->getGroup());

	Group* group = pCharacter->getGroup();
	group->remove(pCharacter);
	group->sendMemberLeaveMessage(pCharacter->getName()); // Notify remaining group members.

	_postMemberRemoval(group);
}

void GroupManager::onDeath(Character* pCharacter) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->getGroup());

	// TODO: X Died.
}

void GroupManager::onLinkdead(Character* pCharacter) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->getGroup());

	Group* group = pCharacter->getGroup();
	group->remove(pCharacter);
	_sendMessage(group, SYS_NAME, pCharacter->getName() + " has gone Linkdead."); // Notify group of LD.
	group->sendMemberLeaveMessage(pCharacter->getName()); // Notify remaining group members.

	_postMemberRemoval(group);
}

Group::Group(Character* pLeader, Character* pMember) : mLeader(pLeader) {
	EXPECTED(pLeader);
	EXPECTED(pMember);
	// NOTE: Error Conditions are ignored here as CTOR is private.
	
	add(pLeader);
	add(pMember);

	pLeader->getConnection()->sendGroupCreate();
	pLeader->getConnection()->sendGroupLeaderChange(pLeader->getName());
	pLeader->getConnection()->sendGroupAcknowledge();
	pLeader->getConnection()->sendGroupFollow(pLeader->getName(), pMember->getName());

	pMember->getConnection()->sendGroupAcknowledge();
	pLeader->getConnection()->sendGroupJoin(pMember->getName());

	sendGroupUpdate(pMember);
}

void Group::add(Character* pCharacter) {
	EXPECTED(pCharacter);
	EXPECTED((pCharacter->hasGroup() == false)); // Check: Character does not have a group.
	EXPECTED((mMembers.size() < Limits::Group::MAX_MEMBERS)); // Check: Group is not already full.

	mMembers.push_back(pCharacter);
	pCharacter->setGroup(this);
}

void Group::remove(Character* pCharacter) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->getGroup() == this); // Check: Pointer matching (sanity).
	EXPECTED(isMember(pCharacter)); // Check: Character is already a member of this group.

	mMembers.remove(pCharacter);
	pCharacter->setGroup(nullptr);

	// Handle: Leader being removed.
	if (mLeader == pCharacter) mLeader = nullptr;
}

void Group::sendMemberLeaveMessage(String pLeaverName) {
	// TODO: Zoning members.
	for (auto i : mMembers)
		i->getConnection()->sendGroupLeave(pLeaverName);
}

void Group::sendGroupLeaderChange() {
	EXPECTED(mLeader); // Check: mLeader pointer is valid.

	// TODO: Zoning members.
	for (auto i : mMembers)
		i->getConnection()->sendGroupLeaderChange(mLeader->getName());
}

bool Group::isMember(Character* pCharacter) {
	EXPECTED_BOOL(pCharacter);

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

Character* Group::getMember(const String& pCharacterName) {
	for (auto i : mMembers) {
		if (i->getName() == pCharacterName)
			return i;
	}

	return nullptr;
}

void Group::sendGroupUpdate(Character* pCharacter) {
	std::list<String> memberNames;
	getMemberNames(memberNames, pCharacter->getName());
	pCharacter->getConnection()->sendGroupUpdate(memberNames);
}

void Group::sendGroupUpdate() {
	std::list<String> memberNames;
	getMemberNames(memberNames, "");
	for (auto i : mMembers) {
		if (i->isZoning() == false) {
			i->getConnection()->sendGroupUpdate(memberNames);
		}
	}
	
}


