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

	sendMemberList();
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

	// Update Zone.
	pCharacter->getZone()->onChangeGuild(pCharacter);

	sendMemberList();
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

	// Update Zone.
	pCharacter->getZone()->onChangeGuild(pCharacter);

	sendMemberList();
}

void Guild::onMakeLeader(Character* pCharacter, GuildMember * pMember) {
	if (!pCharacter) return;
	if (!pMember) return;

	Character* character = getCharacter(pMember->getName());

	// Update previous leader Character.
	pCharacter->setGuild(this, getID(), GuildRanks::Officer, getName());
	updateMemberDetails(pCharacter, getMember(pCharacter->getName()));
	
	// Update Zone Character is in.
	pCharacter->getZone()->onChangeGuild(pCharacter);

	// Update new leader Character.
	character->setGuild(this, getID(), GuildRanks::Leader, getName());
	updateMemberDetails(character, pMember);

	// Update Zone Character is in.
	character->getZone()->onChangeGuild(character);

	// Notify other members.
	sendMessage(SYS_NAME, pMember->getName() + " is now the leader of the guild!");

	sendMemberList();
}

void Guild::onSetMOTD(Character* pCharacter, const String& pMOTD) {
	if (!pCharacter) return;

	mData->mMOTD = pMOTD;
	mData->mMOTDSetter = pCharacter->getName();

	sendMOTD();
}

void Guild::onMOTDRequest(Character* pCharacter) {
	if (!pCharacter) return;

	pCharacter->getConnection()->sendGuildMOTDResponse(getMOTD(), getMOTDSetter());
}

void Guild::onSetURL(const String& pURL) {
	mData->mURL = pURL;

	// Notify members.
	sendMessage(SYS_NAME, "URL has been updated!");

	sendURL();
}

void Guild::onSetChannel(const String& pChannel) {
	mData->mChannel = pChannel;

	// Notify members.
	sendMessage(SYS_NAME, "Channel has been updated!");

	sendChannel();
}

void Guild::onSetPublicNote(GuildMember* pMember, const String& pPublicNote) {
	if (!pMember) return;

	pMember->setPublicNote(pPublicNote);

	// Notify members.
	sendMessage(SYS_NAME, "Public note has been updated! (" + pMember->getName() + ")");

	sendMemberList();
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
		auto packet = Payload::makeGuildMemberList(std::list<GuildMember*>()); // Send empty member list to clear guild window.
		pCharacter->getConnection()->sendPacket(packet);
		delete packet;
	}
}

void Guild::onRemove(GuildMember * pMember) {
	if (!pMember) return;

	// NOTE: Character being removed may not be online.
	auto character = getCharacter(pMember->getName());
	if (character)
		removeCharacter(character);

	removeMember(pMember);
	sendMemberList();
}

void Guild::onLeave(Character* pCharacter) {
	if (!pCharacter) return;

	removeCharacter(pCharacter);

	auto member = getMember(pCharacter->getName());
	removeMember(member);
	sendMemberList();
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
const bool Guild::canSetMOTD(Character* pCharacter) const { return pCharacter->getGuildRank() >= GuildRanks::Officer; }
const bool Guild::canSetURL(Character* pCharacter) const { return pCharacter->getGuildRank() == GuildRanks::Leader; }
const bool Guild::canSetChannel(Character* pCharacter) const { return pCharacter->getGuildRank() == GuildRanks::Leader; }
const bool Guild::canSetPublicNotes(Character* pCharacter) const { return pCharacter->getGuildRank() >= GuildRanks::Officer; }


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
	sendPacket(packet);
	delete packet;
}

void Guild::sendMemberLevelUpdate(GuildMember* pMember) {
	using namespace Payload::Guild;
	if (!pMember) return;

	auto packet = MemberLevelUpdate::construct(getID(), pMember->getName(), pMember->getLevel());
	sendPacket(packet);
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

	sendMemberList(pCharacter);
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

void Guild::onMemberDelete(GuildMember* pMember) {
	if (!pMember) return;

	sendMessage(SYS_NAME, pMember->getName() + " has been deleted.");
	removeMember(pMember);
	sendMemberList();

	// TODO: Handle when Guild Leader deletes themself.
}

void Guild::sendGuildInformation(Character* pCharacter) {
	if (!pCharacter) return;

	auto connection = pCharacter->getConnection();
	connection->sendGuildRank(pCharacter->getGuildRank());
	sendURL(pCharacter);
	sendChannel(pCharacter);
	sendMOTD(pCharacter);
}

void Guild::sendMOTD(Character* pCharacter) {
	if (!pCharacter) return;
	using namespace Payload::Guild;

	auto packet = MOTD::construct(getMOTDSetter(), getMOTD());
	pCharacter->getConnection()->sendPacket(packet);
	delete packet;
}

void Guild::sendMOTD() {
	using namespace Payload::Guild;
	
	auto packet = MOTD::construct(getMOTDSetter(), getMOTD());
	sendPacket(packet);
	delete packet;
}

void Guild::sendMemberList(Character* pCharacter) {
	if (!pCharacter) return;

	auto packet = Payload::makeGuildMemberList(mMembers);
	pCharacter->getConnection()->sendPacket(packet);
	delete packet;
}

void Guild::sendMemberList() {
	auto packet = Payload::makeGuildMemberList(mMembers);
	sendPacket(packet);
	delete packet;
}

void Guild::sendURL(Character* pCharacter) {
	using namespace Payload::Guild;
	if (!pCharacter) return;

	auto packet = GuildUpdate::construct(GuildUpdateAction::URL, getURL());
	pCharacter->getConnection()->sendPacket(packet);
	delete packet;
}

void Guild::sendURL() {
	using namespace Payload::Guild;

	auto packet = GuildUpdate::construct(GuildUpdateAction::URL, getURL());
	sendPacket(packet);
	delete packet;
}

void Guild::sendChannel(Character* pCharacter) {
	using namespace Payload::Guild;
	if (!pCharacter) return;

	auto packet = GuildUpdate::construct(GuildUpdateAction::Channel, getChannel());
	pCharacter->getConnection()->sendPacket(packet);
	delete packet;
}

void Guild::sendChannel() {
	using namespace Payload::Guild;

	auto packet = GuildUpdate::construct(GuildUpdateAction::Channel, getChannel());
	sendPacket(packet);
	delete packet;
}

Character* Guild::getCharacter(const String& pCharacterName) {
	for (auto i : mOnlineMembers) {
		if (i->getName() == pCharacterName)
			return i;
	}

	return nullptr;
}

void Guild::sendPacket(const EQApplicationPacket* pPacket) const {
	for (auto i : mOnlineMembers) {
		if (i->isZoning()) continue;
		i->getConnection()->sendPacket(pPacket);
	}
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
void GuildMember::setPublicNote(const String& pPublicNote) { mData->mPublicNote = pPublicNote; }
