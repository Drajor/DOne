#include "GuildManager.h"
#include "IDataStore.h"
#include "Data.h"

#include "Guild.h"
#include "Utility.h"
#include "Limits.h"
#include "Character.h"
#include "ZoneConnection.h"
#include "../common/EQPacket.h"
#include "Payload.h"
#include "ZoneManager.h"
#include "Zone.h"

GuildManager::~GuildManager() {
	for (auto i : mGuilds) {
		delete i;
	}
	mGuilds.clear();

	for (auto i : mData) {
		delete i;
	}
	mData.clear();

	if (mLog) {
		delete mLog;
		mLog = nullptr;
	}
}

const bool GuildManager::initialise(IDataStore* pDataStore, ILogFactory* pLogFactory) {
	if (mInitialised) return false;
	if (!pDataStore) return false;
	if (!pLogFactory) return false;
	
	mLogFactory = pLogFactory;
	mDataStore = pDataStore;
	
	mLog = mLogFactory->make();
	mLog->setContext("[GuildManager]");
	mLog->status("Initialising.");

	// Load data for guilds.
	if (!mDataStore->loadGuilds(mData)) {
		mLog->error("DataStore::loadGuilds failed.");
		return false;
	}

	for (auto i : mData) {
		auto guild = new Guild();
		if (!guild->initialise(i)) {
			return false;
		}
		mGuilds.push_back(guild);
	}

	mLog->info("Loaded data for " + toString(mGuilds.size()) + " Guilds.");
	mLog->status("Finished initialising.");
	mInitialised = true;
	return true;
}

void GuildManager::onCharacterDelete(const String& pCharacterName) {
	// Check: Does this Character belong to a Guild?
	auto guild = findGuildByMemberName(pCharacterName);
	if (!guild) return;

	auto member = guild->getMember(pCharacterName);

	// Notify Guild.
	guild->onMemberDelete(member);

	// Save.
	if (!save()) mLog->error("Save failed in " + String(__FUNCTION__));

	mLog->info("Removed deleted Character " + pCharacterName + " from " + guild->getName());
}

const bool GuildManager::onCreate(Character* pCharacter, const String& pGuildName) {
	if (!pCharacter) return false;
	if (pCharacter->hasGuild()) return false;
	
	// Check: Guild name is valid.
	if (!Limits::Guild::nameValid(pGuildName)) return false;

	// Check: Guild name is already in use.
	if (exists(pGuildName)) return false;

	// Create Data::Guild
	auto data = new Data::Guild();
	data->mName = pGuildName;
	data->mID = getNextGuildID();
	
	// Create Guild.
	Guild* guild = new Guild();
	if (!guild->initialise(data)) {
		mLog->error("Guild: " + data->mName + " failed to initialise.");
		delete guild;

		return false;
	}

	mData.push_back(data);
	mGuilds.push_back(guild);
	
	guild->onJoin(pCharacter, GuildRanks::Leader);

	// Save.
	if (!save()) mLog->error("Save failed in " + String(__FUNCTION__));

	mLog->info(guild->getName() + " created by " + pCharacter->getName());
	return true;
}

const bool GuildManager::onDelete(Character* pCharacter) {
	if (!pCharacter) return false;

	// Check: Guild is valid.
	auto guild = pCharacter->getGuild();
	if (!guild){
		mLog->error(pCharacter->getName() + " has no guild in " + __FUNCTION__);
		return false;
	}

	// Check: Can Character delete the Guild?
	if (!guild->canDelete(pCharacter)) return false;
	
	// Notify Guild.
	guild->onDelete();

	mData.remove(guild->getData());
	mGuilds.remove(guild);
	
	mLog->info(guild->getName() + " deleted by " + pCharacter->getName());
	delete guild;

	// Save.
	if (!save()) mLog->error("Save failed in " + String(__FUNCTION__));

	return true;
}

const bool GuildManager::onRemove(Character* pRemover, const String& pRemoveeName) {
	if (!pRemover) return false;
	if (pRemover->getName() == pRemoveeName) return false; // This should be handled by GuildManager::onLeave
	
	// Check: Guild is valid.
	auto guild = pRemover->getGuild();
	if (!guild) {
		mLog->error(pRemover->getName() + " has no guild in " + __FUNCTION__);
		return false;
	}

	// Check: Character being removed is a member of the Guild.
	auto member = guild->getMember(pRemoveeName);
	if (!member) return false; // This is not an error because /guildremove allows users to send any name they want.

	// Check: Character is allowed to remove other Guild members.
	if (!guild->canRemove(pRemover)) {
		mLog->error(pRemover->getName() + " attempted to remove " + member->getName());
		return false;
	}

	// Check: Character being removed is a lower or equal rank to the Character removing.
	if (pRemover->getGuildRank() < member->getRank()) {
		mLog->error(pRemover->getName() + " rank: " + toString(pRemover->getGuildRank()) + " attempted to remove " + member->getName() + " rank: " + toString(member->getRank()));
		return false;
	}

	// Notify Guild.
	guild->onRemove(member);

	// Save.
	if (!save()) mLog->error("Save failed in " + String(__FUNCTION__));

	mLog->info(pRemover->getName() + " removed " + pRemoveeName + " from " + guild->getName());
	return true;
}

const bool GuildManager::onLeave(Character* pCharacter) {
	if (!pCharacter) return false;

	// Check: Guild is valid.
	auto guild = pCharacter->getGuild();
	if (!guild){
		mLog->error(pCharacter->getName() + " has no guild in " + String(__FUNCTION__));
		return false;
	}

	// Check: Not the Guild leader.
	if (pCharacter->getGuildRank() == GuildRanks::Leader) {
		mLog->error(pCharacter->getName() + " is the guild leader in " + String(__FUNCTION__));
		return false;
	}

	// Notify Guild.
	guild->onLeave(pCharacter);

	// Save.
	if (!save()) mLog->error("Save failed in " + String(__FUNCTION__));

	mLog->info(pCharacter->getName() + " left " + guild->getName());
	return true;
}

const bool GuildManager::onInvite(Character* pInviter, Character* pInvitee) {
	if (!pInviter) return false;
	if (!pInvitee) return false;

	// Check: The Character inviting has a guild.
	auto guild = pInviter->getGuild();
	if (!guild) return false;

	// Check: The Character inviting is allowed to invite others.
	if (!guild->canInvite(pInviter)) return false;

	// Check: The Character being invited does not already have a Guild.
	if (pInvitee->hasGuild()) return false;

	// Check: The Character being invited does not already have a pending invite.
	if (pInvitee->hasPendingGuildInvite()) return false;
	
	mLog->info(pInviter->getName() + " invited " + pInvitee->getName() + " to join " + guild->getName());
	return true;
}

const bool GuildManager::onInviteAccept(Character* pCharacter) {
	if (!pCharacter) return false;
	if (pCharacter->hasGuild()) return false;
	if (!pCharacter->hasPendingGuildInvite()) return false;

	auto guild = _findByID(pCharacter->getPendingGuildInviteID());
	if (!guild) {
		// TODO: Log.
		return false;
	}

	// Notify Guild.
	guild->onJoin(pCharacter, GuildRanks::Member);

	// Save.
	if (!save()) mLog->error("Save failed in " + String(__FUNCTION__));

	mLog->info(pCharacter->getName() + " accepted " + pCharacter->getPendingGuildInviteName() + "'s invitation to join " + guild->getName());
	return true;
}

Guild* GuildManager::search(const String& pGuildName) const {
	for (auto i : mGuilds) {
		if (i->getName() == pGuildName)
			return i;
	}

	return nullptr;
}

const bool GuildManager::save() {
	return mDataStore->saveGuilds(mData);
}

const u32 GuildManager::getNextGuildID() {
	return ++mNextID;
}

GuildSearchResults GuildManager::getAllGuilds() {
	GuildSearchResults results;
	for (auto i : mGuilds) {
		results.push_back({i->getID(), i->getName()});
	}

	return results;
}

void GuildManager::onConnect(Character* pCharacter, const u32 pGuildID) {
	auto guild = _findByID(pGuildID);

	// Check: Guild does not exist. It was probably deleted.
	if (!guild) {
		mLog->info(pCharacter->getName() + " no longer member of GuildID: " + toString(pGuildID) + ", removing them.");
		pCharacter->clearGuild();
		return;
	}

	// Notify Guild.
	guild->onConnect(pCharacter);
}

Guild* GuildManager::_findByID(const u32 pID) {
	for (auto i : mGuilds) {
		if (i->getID() == pID)
			return i;
	}

	return nullptr;
}

void GuildManager::onEnterZone(Character* pCharacter) {
	if (!pCharacter) return;

	auto guild = pCharacter->getGuild();
	if (!guild) return;

	// Notify Guild.
	guild->onEnterZone(pCharacter);
}

void GuildManager::onLeaveZone(Character* pCharacter) {
	if (!pCharacter) return;

	auto guild = pCharacter->getGuild();
	if (!guild) return;

	// Notify Guild.
	guild->onLeaveZone(pCharacter);
}

void GuildManager::onCamp(Character* pCharacter){
	if (!pCharacter) return;

	auto guild = pCharacter->getGuild();
	if (!guild) return;

	// Notify Guild.
	guild->onCamp(pCharacter);

	//// Update member.
	//GuildMember* member = pCharacter->getGuild()->getMember(pCharacter->getName());
	//EXPECTED(member);
	//member->mZoneID = 0;
	//member->mInstanceID = 0;
	//member->mTimeLastOn = 0; // TODO: Time
	//member->mCharacter = nullptr;

	//pCharacter->getGuild()->mOnlineMembers.remove(pCharacter);
	//_sendMessage(pCharacter->getGuild(), SYS_NAME, pCharacter->getName() + " has gone offline (Camped).");

	//// Update other members.
	//_sendMemberZoneUpdate(pCharacter->getGuild(), member);
}

void GuildManager::onLinkdead(Character* pCharacter) {
	if (!pCharacter) return;

	auto guild = pCharacter->getGuild();
	if (!guild) return;

	// Notify Guild.
	guild->onLinkdead(pCharacter);

	//EXPECTED(pCharacter);
	//EXPECTED(pCharacter->hasGuild());

	//// Update member.
	//GuildMember* member = pCharacter->getGuild()->getMember(pCharacter->getName());
	//EXPECTED(member);
	//member->mZoneID = 0;
	//member->mInstanceID = 0;
	//member->mTimeLastOn = 0; // TODO: Time
	//member->mCharacter = nullptr;

	//pCharacter->getGuild()->mOnlineMembers.remove(pCharacter);
	//_sendMessage(pCharacter->getGuild(), SYS_NAME, pCharacter->getName() + " has gone offline (Linkdead).", pCharacter);

	//// Update other members.
	//_sendMemberZoneUpdate(pCharacter->getGuild(), member);
}

void GuildManager::onLevelChange(Character* pCharacter) {
	//EXPECTED(pCharacter);
	//EXPECTED(pCharacter->hasGuild());

	//// Update member details.
	//GuildMember* member = pCharacter->getGuild()->getMember(pCharacter->getName());
	//EXPECTED(member);

	//uint32 previousLevel = member->mLevel;
	//member->mLevel = pCharacter->getLevel();

	//// Notify guild members.
	//if (member->mLevel > previousLevel) {
	//	StringStream ss; ss << pCharacter->getName() << " is now level " << member->mLevel << "!";
	//	_sendMessage(pCharacter->getGuild(), SYS_NAME, ss.str());
	//}
	//// Update other members.
	//// NOTE: This does not work.....
	////_sendMemberLevelUpdate(member->mGuild, member);

	//// TODO: Use _sendMembers until a better way is found.
	//_sendMembers(member->mGuild);
}


void GuildManager::onMessage(Character* pCharacter, const String& pMessage) {
	if (!pCharacter) return;

	auto guild = pCharacter->getGuild();
	if (!guild) return;

	guild->sendMessage(pCharacter->getName(), pMessage);
}

//void GuildManager::_sendMessage(Guild* pGuild, const String& pSenderName, const String& pMessage, Character* pExclude) {
//	EXPECTED(pGuild);
//
//	for (auto i : pGuild->mOnlineMembers) {
//		if (i == pExclude) continue;;
//
//		// Check: Where a guild member is zoning, queue the message.
//		if (i->isZoning()) {
//			i->addQueuedMessage(ChannelID::Guild, pSenderName, pMessage);
//			continue;
//		}
//		i->getConnection()->sendGuildMessage(pSenderName, pMessage);
//	}
//}

const bool GuildManager::onSetMOTD(Character* pCharacter, const String& pMOTD) {
	if (!pCharacter) return false;

	// Check: Character has a guild.
	auto guild = pCharacter->getGuild();
	if (!guild) {
		mLog->error(pCharacter->getName() + " attempted to set MOTD but they are not in a guild.");
		return false;
	}

	// Check: Character is allowed to set the MOTD.
	if (!guild->canSetMOTD(pCharacter)){
		mLog->error(pCharacter->getName() + " attempted to set MOTD but they are not allowed.");
		return false;
	}

	// Notify Guild.
	guild->onSetMOTD(pCharacter, pMOTD);

	// Save.
	if (!save()) mLog->error("Save failed in " + String(__FUNCTION__));

	return true;
}

const bool GuildManager::onGetMOTD(Character* pCharacter) {
	if (!pCharacter) return false;

	return true;
	//_sendMOTDReply(pCharacter);
}

//void GuildManager::_sendURL(Guild* pGuild) {
//	EXPECTED(pGuild);
//
//	auto outPacket = new EQApplicationPacket(OP_GuildUpdateURLAndChannel, sizeof(Payload::Guild::GuildUpdate));
//	auto payload = reinterpret_cast<Payload::Guild::GuildUpdate*>(outPacket->pBuffer);
//	payload->mAction = Payload::Guild::GuildUpdate::Action::GUILD_URL;
//	strcpy(&payload->mText[0], pGuild->mURL.c_str());
//
//	for (auto i : pGuild->mOnlineMembers) {
//		if (i->isZoning()) { continue; }
//		i->getConnection()->sendPacket(outPacket);
//	}
//
//	safe_delete(outPacket);
//}

//void GuildManager::_sendChannel(Guild* pGuild) {
//	EXPECTED(pGuild);
//
//	auto outPacket = new EQApplicationPacket(OP_GuildUpdateURLAndChannel, sizeof(Payload::Guild::GuildUpdate));
//	auto payload = reinterpret_cast<Payload::Guild::GuildUpdate*>(outPacket->pBuffer);
//	payload->mAction = Payload::Guild::GuildUpdate::Action::GUILD_CHANNEL;
//	strcpy(&payload->mText[0], pGuild->mChannel.c_str());
//
//	for (auto i : pGuild->mOnlineMembers) {
//		if (i->isZoning()) { continue; }
//		i->getConnection()->sendPacket(outPacket);
//	}
//
//	safe_delete(outPacket);
//}

//void GuildManager::_sendMOTDReply(Character* pCharacter) {
//	EXPECTED(pCharacter);
//	EXPECTED(pCharacter->hasGuild());
//
//	Guild* guild = pCharacter->getGuild();
//	pCharacter->getConnection()->sendGuildMOTDReply(guild->mMOTD, guild->mMOTDSetter);
//}

//void GuildManager::_sendMemberLevelUpdate(const Guild* pGuild, const GuildMember* pMember) {
//	EXPECTED(pGuild);
//	EXPECTED(pMember);
//	EXPECTED(pMember->mGuild == pGuild);
//
//	auto outPacket = new EQApplicationPacket(OP_GuildMemberLevelUpdate, sizeof(Payload::Guild::LevelUpdate));
//	auto payload = reinterpret_cast<Payload::Guild::LevelUpdate*>(outPacket->pBuffer);
//
//	payload->mGuildID = pGuild->mID;
//	strcpy(payload->mMemberName, pMember->mName.c_str());
//	payload->mLevel = pMember->mLevel;
//
//	for (auto i : pGuild->mOnlineMembers) {
//		if (i->isZoning()) { continue; }
//		i->getConnection()->sendPacket(outPacket);
//	}
//
//	safe_delete(outPacket);
//}

const bool GuildManager::onSetURL(Character* pCharacter, const String& pURL) {
	return true;
	//EXPECTED(pCharacter);
	//EXPECTED(pCharacter->hasGuild());
	//EXPECTED(isLeader(pCharacter)); // Only leader can change the url.
	//EXPECTED(Limits::Guild::urlLength(pURL));

	//pCharacter->getGuild()->mURL = pURL;
	//save();

	//// Update other members.
	//_sendURL(pCharacter->getGuild());
	//_sendMessage(pCharacter->getGuild(), SYS_NAME, pCharacter->getName() + " has updated the guild URL.");
}

const bool GuildManager::onSetChannel(Character* pCharacter, const String& pChannel) {
	return true;
	//EXPECTED(pCharacter);
	//EXPECTED(pCharacter->hasGuild());
	//EXPECTED(isLeader(pCharacter)); // Only leader can change the channel.
	//EXPECTED(Limits::Guild::channelLength(pChannel));

	//pCharacter->getGuild()->mChannel = pChannel;
	//save();

	//// Update other members.
	//_sendChannel(pCharacter->getGuild());
	//_sendMessage(pCharacter->getGuild(), SYS_NAME, pCharacter->getName() + " has updated the guild channel.");
}

const bool GuildManager::onSetPublicNote(Character* pCharacter, const String& pCharacterName, const String& pPublicNote) {
	if (!pCharacter) return false;

	return true;
	//EXPECTED(pCharacter);
	//EXPECTED(pCharacter->hasGuild());
	//EXPECTED(Limits::Character::nameLength(pCharacterName));
	//EXPECTED(Limits::Guild::publicNoteLength(pNote));

	//if (pCharacter->getName() != pCharacterName)
	//	EXPECTED(isLeader(pCharacter) || isOfficer(pCharacter)); // Only leader or officers can change another Character's public note.

	//Guild* guild = pCharacter->getGuild();
	//GuildMember* member = guild->getMember(pCharacterName);
	//EXPECTED(member);
	//member->mPublicNote = pNote;
	//save();

	//// Update other members.
	//// TODO: Use _sendMembers until a better way is found.
	//_sendMembers(guild);
}

const bool GuildManager::onStatusRequest(Character* pCharacter, const String& pCharacterName) {
	if (!pCharacter) return false;

	return true;
	//EXPECTED(pCharacter);
	//EXPECTED(Limits::Character::nameLength(pCharacterName));

	//ZoneConnection* connection = pCharacter->getConnection();
	//EXPECTED(connection);

	//// Try to find requested Character.
	//GuildMember* member = _findByCharacterName(pCharacterName);
	//if (!member) {
	//	// Character was not found, they either do not exist or do not have a guild.
	//	connection->sendSimpleMessage(MessageType::White, StringID::NOT_IN_A_GUILD, pCharacterName);
	//	return;
	//}

	//u32 stringID = StringID::SI_NONE;
	//String message = "";

	//// Same Guild
	//if (member->mGuild == pCharacter->getGuild()) {
	//	message = pCharacterName;
	//	if (member->mRank == GuildRanks::Leader) { stringID = StringID::LEADER_OF_YOUR_GUILD; }
	//	else if (member->mRank == GuildRanks::Officer) { stringID = StringID::OFFICER_OF_YOUR_GUILD; }
	//	else if (member->mRank == GuildRanks::Member){ stringID = StringID::MEMBER_OF_YOUR_GUILD; }
	//}
	//// Different Guild
	//else {
	//	message = member->mGuild->mName;
	//	if (member->mRank == GuildRanks::Leader) { stringID = StringID::LEADER_OF_X_GUILD; }
	//	else if (member->mRank == GuildRanks::Officer) { stringID = StringID::OFFICER_OF_X_GUILD; }
	//	else if (member->mRank == GuildRanks::Member){ stringID = StringID::MEMBER_OF_X_GUILD; }
	//}

	//if (stringID != StringID::SI_NONE)
	//	connection->sendSimpleMessage(MessageType::White, stringID, message);
}

const bool GuildManager::onPromote(Character* pPromoter, Character* pPromotee) {
	if (!pPromoter) return false;
	if (!pPromotee) return false;

	// Check: Attempt to promote self.
	if (pPromoter == pPromotee) {
		mLog->error(pPromoter->getName() + " attempted to self promote.");
		return false;
	}

	// Check: Promoter Guild is valid.
	auto promoterGuild = pPromoter->getGuild();
	if (!promoterGuild) {
		mLog->error(pPromoter->getName() + " (promoter) has no guild in " + __FUNCTION__);
		return false;
	}

	// Check: Character is allowed to promote other Guild members.
	if (!promoterGuild->canPromote(pPromoter)) {
		mLog->error(pPromoter->getName() + " attempted to promote " + pPromotee->getName());
		return false;
	}

	// Check: Promotee Guild is valid.
	auto promoteeGuild = pPromotee->getGuild();
	if (!promoteeGuild) {
		mLog->error(pPromotee->getName() + " (promotee) has no guild in " + __FUNCTION__);
		return false;
	}

	// Check: Both Characters are in the same Guild.
	if (promoterGuild != promoteeGuild) {
		mLog->error(pPromoter->getName() + " attempted to promote " + pPromotee->getName() + " but not in the same guild.");
		return false;
	}

	// Check: Character can be promoted.
	if (!promoterGuild->canBePromoted(pPromotee)) {
		mLog->error(pPromoter->getName() + " attempted to promote " + pPromotee->getName() + " but they can not be promoted.");
		return false;
	}

	// Notify Guild.
	promoterGuild->onPromote(pPromotee);

	// Save.
	if (!save()) mLog->error("Save failed in " + String(__FUNCTION__));

	mLog->info(pPromoter->getName() + " promoted " + pPromotee->getName() + " in " + promoterGuild->getName());
	return true;
}

const bool GuildManager::onDemote(Character* pDemoter, Character* pDemotee) {
	if (!pDemoter) return false;
	if (!pDemotee) return false;

	// Check: Attempt to demote self.
	if (pDemoter == pDemotee) return false; // TODO: Self demotion is allowed.

	// Check: Demoter Guild is valid.
	auto demoterGuild = pDemoter->getGuild();
	if (!demoterGuild) {
		mLog->error(pDemoter->getName() + " (demoter) has no guild in " + __FUNCTION__);
		return false;
	}

	// Check: Character is allowed to demote other Guild members.
	if (!demoterGuild->canDemote(pDemoter)) {
		mLog->error(pDemoter->getName() + " attempted to demote " + pDemotee->getName());
		return false;
	}

	// Check: Demotee Guild is valid.
	auto demoteeGuild = pDemotee->getGuild();
	if (!demoteeGuild) {
		mLog->error(pDemotee->getName() + " (demotee) has no guild in " + __FUNCTION__);
		return false;
	}

	// Check: Both Characters are in the same Guild.
	if (demoterGuild != demoteeGuild) {
		mLog->error(pDemoter->getName() + " attempted to demote " + pDemotee->getName() + " but not in the same guild.");
		return false;
	}

	// Check: Character can be demoted.
	if (!demoterGuild->canBeDemoted(pDemotee)) {
		mLog->error(pDemoter->getName() + " attempted to demote " + pDemotee->getName() + " but they can not be demoted.");
		return false;
	}

	// Notify Guild.
	demoterGuild->onDemote(pDemotee);

	// Save.
	if (!save()) mLog->error("Save failed in " + String(__FUNCTION__));

	mLog->info(pDemoter->getName() + " demoted " + pDemotee->getName() + " in " + demoterGuild->getName());
	return true;
}

const bool GuildManager::onMakeLeader(Character* pCharacter, const String& pLeaderName) {
	if (!pCharacter) return false;

	// Check: Character is not trying to make themself leader.
	if (pCharacter->getName() == pLeaderName) {
		mLog->error(pCharacter->getName() + " attempted to make themself leader.");
		return false;
	}

	// Check: Guild is valid.
	auto guild = pCharacter->getGuild();
	if (!guild) {
		mLog->error(pCharacter->getName() + " has no guild in " + __FUNCTION__);
		return false;
	}

	// Check: Character is the Guild leader.
	if (pCharacter->getGuildRank() != GuildRanks::Leader) {
		mLog->error(pCharacter->getName() + " attempted to change leader.");
		return false;
	}

	// Check: New leader is a member of the Guild.
	auto member = guild->getMember(pLeaderName);
	if (!member) {
		mLog->error(pCharacter->getName() + " attempted to make " + pLeaderName + " leader but guilds but not in the same guild.");
		return false;
	}

	// NOTE: The new leader does not need to be an officer.

	// Notify Guild.
	guild->onMakeLeader(pCharacter, member);

	// Save.
	if (!save()) mLog->error("Save failed in " + String(__FUNCTION__));

	mLog->info(pCharacter->getName() + " transferred leadership to " + pLeaderName);
	return true;
}

const bool GuildManager::onSetFlags(Character* pCharacter, const String& pCharacterName, const bool pBanker, const bool pAlt) {
	if (!pCharacter) return false;

	return true;
	//EXPECTED(pCharacter);
	//EXPECTED(pCharacter->hasGuild());
	//EXPECTED(isLeader(pCharacter));
	//EXPECTED(Limits::Character::nameLength(pOtherName));

	//GuildMember* member = pCharacter->getGuild()->getMember(pOtherName);
	//EXPECTED(member);

	//if (member->mBanker != pBanker) {
	//	member->mBanker = pBanker;
	//	save();
	//	// TODO: Use _sendMembers until a better way is found.
	//	_sendMembers(member->mGuild);
	//	return;
	//}
}

//void GuildManager::handleSetAlt(Character* pCharacter, const String& pAltName, const bool pAlt) {
//	EXPECTED(pCharacter);
//	EXPECTED(pCharacter->hasGuild());
//	EXPECTED(Limits::Character::nameLength(pAltName));
//
//	GuildMember* member = pCharacter->getGuild()->getMember(pAltName);
//	EXPECTED(member);
//
//	// Check: Return early if member is already an alt.
//	if (member->mAlt == pAlt)
//		return;
//
//	// Check: Permission
//	const bool changingOther = (pCharacter->getName() != pAltName);
//	if (changingOther)
//		EXPECTED(isLeader(pCharacter) || isOfficer(pCharacter)); // Only the leader or an officer can change the alt status of another member.
//
//	member->mAlt = pAlt;
//	save();
//	// TODO: Use _sendMembers until a better way is found.
//	_sendMembers(member->mGuild);
//}

GuildMember* GuildManager::searchMemberByName(const String& pCharacterName) {
	for (auto i : mGuilds) {
		for (auto j : i->getMembers()) {
			if (j->getName() == pCharacterName)
				return j;
		}
	}

	return nullptr;
}

Guild* GuildManager::findGuildByMemberName(const String& pCharacterName) {
	for (auto i : mGuilds) {
		for (auto j : i->getMembers()) {
			if (j->getName() == pCharacterName)
				return i;
		}
	}

	return nullptr;
}
