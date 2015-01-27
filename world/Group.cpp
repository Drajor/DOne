#include "Group.h"
#include "Character.h"
#include "ZoneConnection.h"

void Group::sendMessage(const String& pSenderName, const String& pMessage, Character* pExclude) {
	for (auto i : mMembers) {
		if (i == pExclude) continue;

		// Check: Where a group member is zoning, queue the message.
		if (i->isZoning()) {
			i->addQueuedMessage(ChannelID::Group, pSenderName, pMessage);
			continue;
		}
		i->getConnection()->sendGroupMessage(pSenderName, pMessage);
	}
}

void Group::onCreate(Character* pLeader, Character* pMember) {
	using namespace Payload::Group;
	if (!pLeader) return;
	if (!pMember) return;

	mLeader = pLeader;
	mMembers.push_back(pLeader);
	mMembers.push_back(pMember);

	pLeader->setGroup(this);
	pMember->setGroup(this);

	sendGroup();
	sendLeadershipAbilities();
}

void Group::onJoin(Character* pCharacter) {
	if (!pCharacter) return;

	pCharacter->setGroup(this);
	mMembers.push_back(pCharacter);

	sendGroup();
	sendLeadershipAbilities(pCharacter);
}

void Group::onRoleChange(Character * pCharacter, const u32 pRoleID, const bool pToggle) {
	using namespace Payload::Group;
	if (!pCharacter) return;

	switch (pRoleID) {
	case GroupRole::MainTank:
		setMainTank(pCharacter, pToggle);
		break;
	case GroupRole::MainAssist:
		setMainAssist(pCharacter, pToggle);
		break;
	case GroupRole::Puller:
		setPuller(pCharacter, pToggle);
		break;
	default:
		// This should have been checked 
		return;
		break;
	}

	// Update group members.
	auto packet = Roles::construct(pCharacter->getName(), mLeader->getName(), pRoleID, pToggle ? 1 : 0);
	sendPacket(packet);
	delete packet;
}

Character* Group::getMember(const String& pCharacterName) const {
	auto f = [pCharacterName](const Character* pMember) { return pMember->getName() == pCharacterName; };
	auto i = std::find_if(mMembers.begin(), mMembers.end(), f);
	return i == mMembers.end() ? nullptr : *i;
}

void Group::sendPacket(const EQApplicationPacket* pPacket) const {
	for (auto i : mMembers) {
		if (i->isZoning()) continue;
		i->getConnection()->sendPacket(pPacket);
	}
}

void Group::sendPacket(const EQApplicationPacket* pPacket, Zone* pZone, Character* pExcludeCharacter) const {
	for (auto i : mMembers) {
		if (i->isZoning()) continue;
		if (i == pExcludeCharacter) continue;
		if (i->getZone() != pZone) continue;;

		i->getConnection()->sendPacket(pPacket);
	}
}

void Group::setMainTank(Character* pCharacter, const bool pValue) {
	// Getting turned on.
	if (pValue) {
		for (auto i : mMembers) i->setIsGroupMainTank(false);
		pCharacter->setIsGroupMainTank(true);
	}
	// Getting turned off.
	else {
		pCharacter->setIsGroupMainTank(false);
	}
}

void Group::setMainAssist(Character* pCharacter, const bool pValue) {
	// Getting turned on.
	if (pValue) {
		for (auto i : mMembers) i->setIsGroupMainAssist(false);
		pCharacter->setIsGroupMainAssist(true);
	}
	// Getting turned off.
	else {
		pCharacter->setIsGroupMainAssist(false);
	}
}

void Group::setPuller(Character* pCharacter, const bool pValue) {
	// Getting turned on.
	if (pValue) {
		for (auto i : mMembers) i->setIsGroupPuller(false);
		pCharacter->setIsGroupPuller(true);
	}
	// Getting turned off.
	else {
		pCharacter->setIsGroupPuller(false);
	}
}

void Group::onLeave(Character* pCharacter) {
	if (!pCharacter) return;
	pCharacter->clearGroup();
	mMembers.remove(pCharacter);

	auto packet = Payload::Group::DisbandYou::construct(pCharacter->getName(), pCharacter->getName());
	pCharacter->getConnection()->sendPacket(packet);
	delete packet;

	packet = Payload::Group::DisbandOther::construct(pCharacter->getName());
	sendPacket(packet);
	delete packet;
}

void Group::onRemove(Character* pCharacter) {
	if (!pCharacter) return;
	pCharacter->clearGroup();
	mMembers.remove(pCharacter);

	auto packet = Payload::Group::DisbandYou::construct(pCharacter->getName(), mLeader->getName());
	pCharacter->getConnection()->sendPacket(packet);
	delete packet;

	packet = Payload::Group::DisbandOther::construct(pCharacter->getName());
	sendPacket(packet);
	delete packet;
}

void Group::onDisband() {
	for (auto i : mMembers) {
		i->clearGroup();
		if (i->isZoning()) continue;

		auto packet = Payload::Group::DisbandYou::construct(i->getName(), i->getName());
		i->getConnection()->sendPacket(packet);
		delete packet;
	}
	mMembers.clear();
}

void Group::onMakeLeader(Character * pCharacter) {
	if (!pCharacter) return;
	mLeader = pCharacter;

	auto packet = Payload::Group::LeaderUpdate::construct(pCharacter->getName());
	sendPacket(packet);
	delete packet;
}

void Group::sendGroup(Character* pCharacter) {
	using namespace Payload::Group;

	auto packet = Payload::makeGroupMemberList(this);
	pCharacter->getConnection()->sendPacket(packet);
	delete packet;
}

void Group::sendGroup() {
	using namespace Payload::Group;

	auto packet = Payload::makeGroupMemberList(this);
	sendPacket(packet);
	delete packet;
}

void Group::sendLeadershipAbilities(Character* pCharacter) {
	using namespace Payload::Group;

	auto packet = LeadershipAbilities::construct();
	pCharacter->getConnection()->sendPacket(packet);
	delete packet;
}

void Group::sendLeadershipAbilities() {
	using namespace Payload::Group;

	auto packet = LeadershipAbilities::construct();
	sendPacket(packet);
	delete packet;
}

void Group::onEnterZone(Character* pCharacter) {
	if (!pCharacter) return;

	// TODO: It may be possible to send these in CharacterProfile.
	sendGroup(pCharacter);
	sendLeadershipAbilities(pCharacter);

	auto packet = ZoneConnection::makeChannelMessage(ChannelID::Group, SYS_NAME, pCharacter->getName() + " has entered the zone!");
	sendPacket(packet, pCharacter->getZone(), pCharacter);
	delete packet;
}

void Group::onLeaveZone(Character* pCharacter) {
	if (!pCharacter) return;

	auto packet = ZoneConnection::makeChannelMessage(ChannelID::Group, SYS_NAME, pCharacter->getName() + " has left the zone!");
	sendPacket(packet, pCharacter->getZone(), pCharacter);
	delete packet;
}
