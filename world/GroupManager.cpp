#include "GroupManager.h"
#include "Character.h"
#include "Zone.h"
#include "ZoneClientConnection.h"
#include "LogSystem.h"

void GroupManager::makeGroup(Character* pLeader, Character* pMember) {
	Group* group = new Group();
	group->addMember(pLeader);
	group->addMember(pMember);
	group->setLeader(pLeader);

	pLeader->getConnection()->sendGroupCreate();
	pLeader->getConnection()->sendGroupAcknowledge();
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

	mCharacters.push_back(pCharacter);
	pCharacter->setGroup(this);
}

void Group::setLeader(Character* pCharacter) {
	mLeader = pCharacter;
	mLeader->getConnection()->sendGroupLeaderChange(mLeader->getName());
}

bool Group::isMember(Character* pCharacter) {
	for (auto i : mCharacters) {
		if (i == pCharacter) return true;
	}

	return false;
}
