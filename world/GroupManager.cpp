#include "GroupManager.h"
#include "LogSystem.h"
#include "ZoneManager.h"
#include "Group.h"
#include "GroupConstants.h"
#include "Character.h"
#include "ZoneConnection.h"

GroupManager::~GroupManager() {
	if (mLog) {
		delete mLog;
		mLog = nullptr;
	}

	for (auto i : mGroups)
		delete i;

	mGroups.clear();
	mZoneManager = nullptr;
}

const bool GroupManager::initialise(ILogFactory* pLogFactory, ZoneManager* pZoneManager) {
	if (mInitialised) return false;
	if (!pLogFactory) return false;
	if (!pZoneManager) return false;

	mZoneManager = pZoneManager;

	// Create and configure logging.
	mLog = pLogFactory->make();
	mLog->setContext("[GroupManager]");
	mLog->status("Initialising.");

	mLog->status("Finished initialising.");
	mInitialised = true;
	return true;
}

const bool GroupManager::onMessage(Character* pCharacter, const String& pMessage) {
	if (!pCharacter) return false;

	auto group = pCharacter->getGroup();
	if (!group) return false;

	// Notify Group.
	group->sendMessage(pCharacter->getName(), pMessage);
	return true;
}

const bool GroupManager::onInvite(Character* pInviter, const String& pInviteeName) {
	if (!pInviter) return false;

	// Check: Prevent self invite.
	if (pInviter->getName() == pInviteeName) {
		mLog->error(pInviter->getName() + " attempted to invite themself.");
		return false;
	}

	// Check: Inviter is leader (if they are already in a group).
	if (pInviter->hasGroup() && !pInviter->isGroupLeader()) {
		mLog->error(pInviter->getName() + " attempted to invite " + pInviteeName + " but is not leader.");
		return false;
	}

	// Check: Character exists.
	auto invitee = mZoneManager->findCharacter(pInviteeName);
	if (!invitee) {
		pInviter->notify(pInviteeName + " was not found.");
		return true;
	}

	// Check: Invitee has a group already.
	if (invitee->hasGroup()) {
		pInviter->notify(pInviteeName + " already has a group.");
		return true;
	}

	// Check: Character has a pending group invite.
	if (invitee->hasGroupInvitation()) {
		pInviter->notify(pInviteeName + " already is considering joining a group.");
		return true;
	}

	// Record invitation.
	auto& invitation = invitee->getGroupInvitation();
	invitation.mInviterName = pInviter->getName();
	invitation.mTimeInvited = Utility::Time::now();

	// Send the invite.
	invitee->getConnection()->sendGroupInvite(pInviter->getName());

	mLog->info(pInviter->getName() + " invited " + pInviteeName + " to join a group.");
	return true;
}

const bool GroupManager::onInviteAccept(Character* pCharacter) {
	if (!pCharacter) return false;

	// Check: Character accepting the invitation actually has an invitation.
	if (!pCharacter->hasGroupInvitation()) {
		mLog->error("group invite while no invitation todo");
		return false;
	}

	auto& invitation = pCharacter->getGroupInvitation();

	auto inviter = mZoneManager->findCharacter(invitation.mInviterName, true); // We can form a group while the inviter is zoning.

	// LD or camped?
	// This is kind of an edge case, without storing invitations sent on the inviting Character this may come up very rarely.
	if (!inviter) {
		pCharacter->notify("Unable to join group.");

		// Clear invitation.
		pCharacter->clearGroupInvitation();
		return true;
	}

	auto group = inviter->getGroup();

	// Handle: Forming a new Group.
	if (!group) onCreate(inviter, pCharacter);

	// Handle: Joining an existing Group.
	else onJoin(group, pCharacter);

	// Clear invitation.
	pCharacter->clearGroupInvitation();
	return true;
}

const bool GroupManager::onInviteDecline(Character* pCharacter) {
	if (!pCharacter) return false;

	// Check: The Character declining the invite has a pending invite.
	if (!pCharacter->hasGroupInvitation()) {
		mLog->error(pCharacter->getName() + " declined invite but they have no invitation.");
		return false;
	}

	// Let the inviter know the invite was declined.
	auto invitation = pCharacter->getGroupInvitation();
	auto inviter = mZoneManager->findCharacter(invitation.mInviterName);
	if (inviter)
		inviter->notify(pCharacter->getName() + " has declined your invitation to join the group.");

	// Clear invitation.
	pCharacter->clearGroupInvitation();
	return true;
}

const bool GroupManager::onRoleChange(Character* pCharacter, const String& pTargetName, const u32 pRoleID, const u8 pToggle) {
	if (!pCharacter) return false;

	// Check: Group is valid.
	auto group = pCharacter->getGroup();
	if (!group){
		mLog->error(pCharacter->getName() + " has no group in " + String(__FUNCTION__));
		return false;
	}

	// Check: Character is leader.
	if (!pCharacter->isGroupLeader()) {
		mLog->error(pCharacter->getName() + " attempted to change roles but they are not leader.");
		return false;
	}

	// Check: Target is a Group member.
	auto character = group->getMember(pTargetName);
	if (!character) {
		mLog->error(pCharacter->getName() + " attempted to change roles for " + pTargetName + " but they are not a member of the group.");
		return false;
	}

	// Check: Role ID is valid.
	if (pRoleID != GroupRole::MainTank && pRoleID != GroupRole::MainAssist && pRoleID != GroupRole::Puller) {
		mLog->error(pCharacter->getName() + " attempted to change an unknown role: " + toString(pRoleID));
		return false;
	}

	// Notify Group.
	group->onRoleChange(character, pRoleID, pToggle == 1 ? true : false);
	return true;
}

void GroupManager::onEnterZone(Character* pCharacter) {
	if (!pCharacter) return;
	
	auto group = pCharacter->getGroup();
	if (!group) return;

	group->onEnterZone(pCharacter);
}

void GroupManager::onLeaveZone(Character* pCharacter) {
	if (!pCharacter) return;

	auto group = pCharacter->getGroup();
	if (!group) return;

	group->onLeaveZone(pCharacter);
}

void GroupManager::onCamp(Character* pCharacter) {
	if (!pCharacter) return;

	// Check: Group is valid.
	auto group = pCharacter->getGroup();
	if (!group) {
		mLog->error(pCharacter->getName() + " has no group in " + String(__FUNCTION__));
		return;
	}

	// Notify Group.
	group->onLeave(pCharacter);

	check(group);
}

void GroupManager::onDeath(Character* pCharacter) {
	//EXPECTED(pCharacter);
	//EXPECTED(pCharacter->getGroup());

	//// TODO: X Died.
}

void GroupManager::onLinkdead(Character* pCharacter) {
	if (!pCharacter) return;

	// Check: Group is valid.
	auto group = pCharacter->getGroup();
	if (!group) {
		mLog->error(pCharacter->getName() + " has no group in " + String(__FUNCTION__));
		return;
	}

	// Notify Group.
	group->onLeave(pCharacter);

	check(group);
}

const bool GroupManager::onCreate(Character* pLeader, Character* pMember) {
	if (!pLeader) return false;
	if (!pMember) return false;

	auto group = new Group(mNextID++);
	mGroups.push_back(group);

	group->onCreate(pLeader, pMember);
	return true;
}

const bool GroupManager::onJoin(Group* pGroup, Character* pCharacter) {
	if (!pGroup) return false;
	if (!pCharacter) return false;

	// Check: Is Group already full?
	if (pGroup->isFull()) {
		pCharacter->notify("Unable to join group as it is full.");
		return true;
	}

	pGroup->onJoin(pCharacter);
	return true;
}

const bool GroupManager::onLeave(Character* pCharacter) {
	if (!pCharacter) return false;

	// Check: Group is valid.
	auto group = pCharacter->getGroup();
	if (!group) {
		mLog->error(pCharacter->getName() + " has no group in " + String(__FUNCTION__));
		return false;
	}
	
	// Notify Group.
	group->onLeave(pCharacter);

	check(group);
	return true;
}

const bool GroupManager::onRemove(Character* pCharacter, const String& pTargetName) {
	if (!pCharacter) return false;

	// Check: Group is valid.
	auto group = pCharacter->getGroup();
	if (!group) {
		mLog->error(pCharacter->getName() + " has no group in " + String(__FUNCTION__));
		return false;
	}

	// Check: Character is leader.
	if (!pCharacter->isGroupLeader()) {
		mLog->error(pCharacter->getName() + " attempted to remove " + pTargetName + " but they are not leader.");
		return false;
	}

	// Check: Target is a Group member.
	auto character = group->getMember(pTargetName);
	if (!character) {
		mLog->error(pCharacter->getName() + " attempted to remove " + pTargetName + " but they are not a group member.");
		return false;
	}

	// Notify Group.
	group->onRemove(character);
	
	check(group);
	return true;
}

const bool GroupManager::onMakeLeader(Character* pCharacter, const String& pLeaderName) {
	if (!pCharacter) return false;

	// Check: Group is valid.
	auto group = pCharacter->getGroup();
	if (!group) {
		mLog->error(pCharacter->getName() + " has no group in " + String(__FUNCTION__));
		return false;
	}

	// Check: Character is leader.
	if (!pCharacter->isGroupLeader()) {
		mLog->error(pCharacter->getName() + " attempted to transfer leadership but they are not leader.");
		return false;
	}

	// Check: Target is a Group member.
	auto character = group->getMember(pLeaderName);
	if (!character) {
		mLog->error(pCharacter->getName() + " attempted to transfer leadership to " + pLeaderName + " but they are not a group member.");
		return false;
	}

	// Check: Target is not self.
	if (character == pCharacter) {
		mLog->error(pCharacter->getName() + " attempted to transfer leadership to themself.");
		return false;
	}

	// Notify Group.
	group->onMakeLeader(character);

	return true;
}

void GroupManager::check(Group* pGroup) {
	if (!pGroup) return;

	// Check: Do we need to disband group?
	if (pGroup->numMembers() == 1) {
		pGroup->onDisband();
		mGroups.remove(pGroup);
		delete pGroup;
	}
}