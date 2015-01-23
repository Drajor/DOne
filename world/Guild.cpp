#include "Guild.h"
#include "Data.h"
#include "Character.h"
#include "Zone.h"
#include "ZoneConnection.h"
#include "Payload.h"

Guild::~Guild() {
	if (mData) {
		delete mData;
		mData = nullptr;
	}
}

const bool Guild::initialise(Data::Guild* pData) {
	if (!pData) return false;
	mData = pData;

	for (auto i : pData->mMembers) {
		auto member = new GuildMember(i);
		mMembers.push_back(member);
	}

	return true;
}

const u32 Guild::getID() const { return mData->mID; }
const String& Guild::getName() const { return mData->mName; }
const String& Guild::getMOTD() const { return mData->mMOTD; }
const String& Guild::getMOTDSetter() const { return mData->mMOTDSetter; }
const String& Guild::getURL() const { return mData->mURL; }
const String& Guild::getChannel() const { return mData->mChannel; }

void Guild::onDelete() {
	while (!mOnlineMembers.empty())
		onLeave(*mOnlineMembers.begin());
}

void Guild::onJoin(Character* pCharacter, const u8 pRank) {
	// Create Data::GuildMember.
	auto data = new Data::GuildMember();
	mData->mMembers.push_back(data);

	// Create GuildMember.
	auto member = new GuildMember(data);
	mMembers.push_back(member);

	// Update Character.
	pCharacter->setGuild(this, getID(), pRank, getName());

	// Update GuildMember.
	updateMemberDetails(pCharacter, member);

	mOnlineMembers.push_back(pCharacter);

	// Notify other members.
	sendMessage(SYS_NAME, pCharacter->getName() + " has come online!");

	// Update Character.
	sendGuildInformation(pCharacter);

	// Update 'Guild Window' 'Member List' for all online members.
	for (auto i : mOnlineMembers) {
		if (i->isZoning()) continue;
		i->getConnection()->sendGuildMembers(mMembers);
	}
}

void Guild::onPromote(Character* pCharacter) {
	if (!pCharacter) return;

	auto member = getMember(pCharacter->getName());

	// Update Character.
	pCharacter->setGuild(this, getID(), GuildRanks::Officer, getName());

	// Update GuildMember.
	updateMemberDetails(pCharacter, member);

	// Notify other members.
	sendMessage(SYS_NAME, pCharacter->getName() + " has been promoted to officer!");

	// Update Character.
	sendGuildInformation(pCharacter);

	// Update 'Guild Window' 'Member List' for all online members.
	for (auto i : mOnlineMembers) {
		if (i->isZoning()) continue;
		i->getConnection()->sendGuildMembers(mMembers);
	}
}

void Guild::onDemote(Character* pCharacter) {
	if (!pCharacter) return;

	auto member = getMember(pCharacter->getName());

	// Update Character.
	pCharacter->setGuild(this, getID(), GuildRanks::Member, getName());

	// Update GuildMember.
	updateMemberDetails(pCharacter, member);

	// Notify other members.
	sendMessage(SYS_NAME, pCharacter->getName() + " has been demoted to member!");

	// Update Character.
	sendGuildInformation(pCharacter);

	// Update 'Guild Window' 'Member List' for all online members.
	for (auto i : mOnlineMembers) {
		if (i->isZoning()) continue;
		i->getConnection()->sendGuildMembers(mMembers);
	}
}

void Guild::removeMember(GuildMember* pMember) {
	if (!pMember) return;

	auto data = pMember->getData();

	// Remove Data::GuildMember from Data::Guild
	mData->mMembers.remove(pMember->getData());

	mMembers.remove(pMember);
	delete pMember;
	delete data;
}

void Guild::removeCharacter(Character* pCharacter) {
	if (!pCharacter) return;

	// Remove from online members list.
	mOnlineMembers.remove(pCharacter);

	// Update Character.
	pCharacter->clearGuild();

	if (pCharacter->isZoning() == false) {
		// Update Zone.
		pCharacter->getZone()->onChangeGuild(pCharacter);

		// Update 'Guild Window' 'Member List' for the leaving Character.
		pCharacter->getConnection()->sendGuildMembers(std::list<GuildMember*>()); // Send empty member list to clear guild window.
	}

	// Update 'Guild Window' 'Member List' for all online members.
	for (auto i : mOnlineMembers) {
		if (i->isZoning()) continue;
		i->getConnection()->sendGuildMembers(mMembers);
	}
}

void Guild::onRemove(GuildMember * pMember) {
	if (!pMember) return;

	Character* character = nullptr;
	
	// Check: Character being removed is online.
	for (auto i : mOnlineMembers) {
		if (i->getName() == pMember->getName()) {
			character = i;
			break;
		}
	}

	removeMember(pMember);
	
	if (character)
		removeCharacter(character);
}

void Guild::onLeave(Character* pCharacter) {
	if (!pCharacter) return;

	auto member = getMember(pCharacter->getName());
	removeMember(member);
	removeCharacter(pCharacter);
}

void Guild::onConnect(Character* pCharacter) {
	if (!pCharacter) return;

	// Check: Character still belongs to this guild.
	auto member = getMember(pCharacter->getName());
	if (!member) {
		pCharacter->clearGuild();
		return;
	}

	pCharacter->setGuild(this, getID(), member->getRank(), getName());

	// Update GuildMember.
	updateMemberDetails(pCharacter, member);

	sendMessage(SYS_NAME, pCharacter->getName() + " has come online!");
	mOnlineMembers.push_back(pCharacter);
}

void Guild::onMemberDisconnect(Character* pCharacter) {
	mOnlineMembers.remove(pCharacter);
}

const bool Guild::canDelete(Character* pCharacter) const { return pCharacter->getGuildRank() == GuildRanks::Leader; }
const bool Guild::canInvite(Character* pCharacter) const { return pCharacter->getGuildRank() >= GuildRanks::Officer; }
const bool Guild::canRemove(Character* pCharacter) const { return pCharacter->getGuildRank() >= GuildRanks::Officer; }
const bool Guild::canPromote(Character* pCharacter) const { return pCharacter->getGuildRank() == GuildRanks::Leader; }
const bool Guild::canBePromoted(Character* pCharacter) const { return pCharacter->getGuildRank() == GuildRanks::Member; }
const bool Guild::canDemote(Character* pCharacter) const { return pCharacter->getGuildRank() == GuildRanks::Leader; }
const bool Guild::canBeDemoted(Character* pCharacter) const { return pCharacter->getGuildRank() == GuildRanks::Officer; }

GuildMember* Guild::getMember(const String& pCharacterName) const {
	auto f = [pCharacterName](const GuildMember* pMember) { return pMember->getName() == pCharacterName; };
	auto i = std::find_if(mMembers.begin(), mMembers.end(), f);
	return i == mMembers.end() ? nullptr : *i;
}

void Guild::updateMemberDetails(Character* pCharacter, GuildMember* pMember) {
	if (!pCharacter) return;
	if (!pMember) return;

	pMember->setRank(pCharacter->getGuildRank());
	pMember->setName(pCharacter->getName());
	pMember->setClass(pCharacter->getClass());
	pMember->setLevel(pCharacter->getLevel());
	pMember->setLastSeen(Utility::Time::now());

	auto zone = pCharacter->getZone();
	if (zone) {
		pMember->setZoneID(zone->getID());
		pMember->setInstanceID(zone->getInstanceID());
	}
	else {
		pMember->setZoneID(0);
		pMember->setInstanceID(0);
	}
}

void Guild::sendMessage(const String& pSenderName, const String& pMessage, Character* pExclude) {
	for (auto i : mOnlineMembers) {
		if (i == pExclude) continue;

		// Check: Where a guild member is zoning, queue the message.
		if (i->isZoning()) {
			i->addQueuedMessage(ChannelID::Guild, pSenderName, pMessage);
			continue;
		}
		i->getConnection()->sendGuildMessage(pSenderName, pMessage);
	}
}

void Guild::sendMemberZoneUpdate(GuildMember* pMember) {
	using namespace Payload::Guild;
	if (!pMember) return;

	auto packet = MemberZoneUpdate::construct(getID(), pMember->getName(), pMember->getZoneID(), pMember->getInstanceID(), pMember->getLastSeen());
	for (auto i : mOnlineMembers) {
		if (i->isZoning()) { continue; }
		i->getConnection()->sendPacket(packet);
	}

	delete packet;
}

void Guild::sendMemberLevelUpdate(GuildMember* pMember) {
	using namespace Payload::Guild;
	if (!pMember) return;

	auto packet = MemberLevelUpdate::construct(getID(), pMember->getName(), pMember->getLevel());
	for (auto i : mOnlineMembers) {
		if (i->isZoning()) { continue; }
		i->getConnection()->sendPacket(packet);
	}

	delete packet;
}

void Guild::onEnterZone(Character* pCharacter) {
	if (!pCharacter) return;
	auto member = getMember(pCharacter->getName());
	if (!member) return;

	// Update GuildMember.
	updateMemberDetails(pCharacter, member);

	// Update Character entering Zone.
	sendGuildInformation(pCharacter);

	// Update other guild members.
	sendMemberZoneUpdate(member);
}

void Guild::onLeaveZone(Character* pCharacter) {
	if (!pCharacter) return;
	auto member = getMember(pCharacter->getName());
	if (!member) return;

	// Update GuildMember.
	updateMemberDetails(pCharacter, member);

	// Update other guild members.
	sendMemberZoneUpdate(member);
}

void Guild::onCamp(Character* pCharacter) {
	if (!pCharacter) return;
	auto member = getMember(pCharacter->getName());
	if (!member) return;

	// Update GuildMember.
	updateMemberDetails(pCharacter, member);

	mOnlineMembers.remove(pCharacter);

	// Update other guild members.
	sendMemberZoneUpdate(member);

	sendMessage(SYS_NAME, pCharacter->getName() + " has gone offline (Camped).");
}

void Guild::onLinkdead(Character* pCharacter) {
	if (!pCharacter) return;
	auto member = getMember(pCharacter->getName());
	if (!member) return;

	// Update GuildMember.
	updateMemberDetails(pCharacter, member);

	mOnlineMembers.remove(pCharacter);

	// Update other guild members.
	sendMemberZoneUpdate(member);
	
	sendMessage(SYS_NAME, pCharacter->getName() + " has gone offline (LinkDead).", pCharacter);
}

void Guild::sendGuildInformation(Character* pCharacter) {
	if (!pCharacter) return;

	auto connection = pCharacter->getConnection();
	connection->sendGuildRank(pCharacter->getGuildRank());
	connection->sendGuildMembers(mMembers);
	connection->sendGuildURL(getURL());
	connection->sendGuildChannel(getChannel());
	connection->sendGuildMOTD(getMOTD(), getMOTDSetter());
}

void Guild::sendMOTD() {
	using namespace Payload::Guild;
	auto packet = MOTD::construct(getMOTDSetter(), getMOTD());
	for (auto i : mOnlineMembers) {
		if (i->isZoning()) { continue; }
		//strcpy(payload->mCharacterName, i->getName().c_str()); // TODO: Need to test whether this is actually needed by the client.
		i->getConnection()->sendPacket(packet);
	}

	delete packet;
}

GuildMember::GuildMember(Data::GuildMember* pData) : mData(pData)
{

}

GuildMember::~GuildMember() {

}

const String& GuildMember::getName() const { return mData->mName; }
const u8 GuildMember::getRank() const { return mData->mRank; }
const u8 GuildMember::getLevel() const { return mData->mLevel; }
const u8 GuildMember::getClass() const { return mData->mClass; }
const bool GuildMember::isBanker() const { return mData->mBanker; }
const bool GuildMember::isTributeEnabled() const { return mData->mTributeEnabled; }
const bool GuildMember::isAlt() const { return mData->mAlt; }
const u32 GuildMember::getLastSeen() const { return mData->mTimeLastOn; }
const u32 GuildMember::getTotalTribute() const { return mData->mLastTribute; }
const u32 GuildMember::getLastTribute() const { return mData->mLastTribute; }
const String& GuildMember::getPublicNote() const { return mData->mPublicNote; }
const u32 GuildMember::getFlags() const { return isBanker() ? 1 : 0 + isAlt() ? 2 : 0; }

void GuildMember::setName(const String& pCharacterName) { mData->mName = pCharacterName; }
void GuildMember::setRank(const u8 pRank) { mData->mRank = pRank; }
void GuildMember::setLevel(const u8 pLevel) { mData->mLevel = pLevel; }
void GuildMember::setClass(const u8 pClass) { mData->mClass = pClass; }
void GuildMember::setIsBanker(const bool pIsBanker) { mData->mBanker = pIsBanker; }
void GuildMember::setIsTributeEnabled(const bool pIsTributeEnabled) { mData->mTributeEnabled = pIsTributeEnabled; }
void GuildMember::setIsAlt(const bool pIsAlt) { mData->mAlt = pIsAlt; }
void GuildMember::setLastSeen(const u32 pLastSeen) { mData->mTimeLastOn = pLastSeen; }
