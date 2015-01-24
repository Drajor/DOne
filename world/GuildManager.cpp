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
}

void GuildManager::onLinkdead(Character* pCharacter) {
	if (!pCharacter) return;

	auto guild = pCharacter->getGuild();
	if (!guild) return;

	// Notify Guild.
	guild->onLinkdead(pCharacter);
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

const bool GuildManager::onSetMOTD(Character* pCharacter, const String& pMOTD) {
	if (!pCharacter) return false;

	// Check: Character has a guild.
	auto guild = pCharacter->getGuild();
	if (!guild) {
		mLog->error(pCharacter->getName() + " attempted to set MOTD but they are not in a guild.");
		return false;
	}

	// Check: Character is allowed to set the MOTD.
	if (!guild->canSetMOTD(pCharacter)) {
		mLog->error(pCharacter->getName() + " attempted to set MOTD but they are not allowed.");
		return false;
	}

	// Notify Guild.
	guild->onSetMOTD(pCharacter, pMOTD);

	// Save.
	if (!save()) mLog->error("Save failed in " + String(__FUNCTION__));

	return true;
}

const bool GuildManager::onMOTDRequest(Character* pCharacter) {
	if (!pCharacter) return false;

	// Check: Character has a guild.
	auto guild = pCharacter->getGuild();
	if (!guild) {
		mLog->error(pCharacter->getName() + " attempted to request MOTD but they are not in a guild.");
		return false;
	}

	// Notify Guild.
	guild->onMOTDRequest(pCharacter);

	return true;
}

const bool GuildManager::onSetURL(Character* pCharacter, const String& pURL) {
	if (!pCharacter) return false;

	// Check: Character has a guild.
	auto guild = pCharacter->getGuild();
	if (!guild) {
		mLog->error(pCharacter->getName() + " attempted to set URL but they are not in a guild.");
		return false;
	}

	// Check: Character is allowed to set the URL.
	if (!guild->canSetURL(pCharacter)) {
		mLog->error(pCharacter->getName() + " attempted to set URL but they are not allowed.");
		return false;
	}

	// Notify Guild.
	guild->onSetURL(pURL);

	// Save.
	if (!save()) mLog->error("Save failed in " + String(__FUNCTION__));

	return true;
}

const bool GuildManager::onSetChannel(Character* pCharacter, const String& pChannel) {
	if (!pCharacter) return false;

	// Check: Character has a guild.
	auto guild = pCharacter->getGuild();
	if (!guild) {
		mLog->error(pCharacter->getName() + " attempted to set channel but they are not in a guild.");
		return false;
	}

	// Check: Character is allowed to set the channel.
	if (!guild->canSetChannel(pCharacter)) {
		mLog->error(pCharacter->getName() + " attempted to set channel but they are not allowed.");
		return false;
	}

	// Notify Guild.
	guild->onSetChannel(pChannel);

	// Save.
	if (!save()) mLog->error("Save failed in " + String(__FUNCTION__));

	return true;
}

const bool GuildManager::onSetPublicNote(Character* pCharacter, const String& pCharacterName, const String& pPublicNote) {
	if (!pCharacter) return false;

	// Check: Character has a guild.
	auto guild = pCharacter->getGuild();
	if (!guild) {
		mLog->error(pCharacter->getName() + " attempted to set a public note but they are not in a guild.");
		return false;
	}

	// Check: Character is allowed to set public notes.
	if (!guild->canSetPublicNotes(pCharacter)) {
		mLog->error(pCharacter->getName() + " attempted to set a public note but they are not allowed.");
		return false;
	}

	// Check: Target Character is a member of the Guild.
	auto member = guild->getMember(pCharacterName);
	if (!member) {
		mLog->error(pCharacter->getName() + " attempted to set the public note of " + pCharacterName + " but they are not a guild member." );
		return false;
	}

	// Notify Guild.
	guild->onSetPublicNote(member, pPublicNote);

	// Save.
	if (!save()) mLog->error("Save failed in " + String(__FUNCTION__));

	return true;
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

const bool GuildManager::onSetFlags(Character* pCharacter, const String& pCharacterName, const u32 pFlags) {
	if (!pCharacter) return false;

	// Check: Character has a guild.
	auto guild = pCharacter->getGuild();
	if (!guild) {
		mLog->error(pCharacter->getName() + " attempted to set flags but they are not in a guild.");
		return false;
	}

	// Check: Target Character is a member of the Guild.
	auto member = guild->getMember(pCharacterName);
	if (!member) {
		mLog->error(pCharacter->getName() + " attempted to set the flags of " + pCharacterName + " but they are not a guild member.");
		return false;
	}

	// Check: Flags value range.
	if (pFlags > 3) {
		mLog->error(pCharacter->getName() + " attempted to set the flags of " + pCharacterName + " with an invalid value of" + toString(pFlags));
		return false;
	}

	const bool setSelf = pCharacter->getName() == pCharacterName;
	const bool setBanker = pFlags & 0x01;
	const bool changeBanker = member->isBanker() != setBanker;
	const bool setAlt = pFlags & 0x02;
	const bool changeAlt = member->isAlt() != setAlt;

	// Check: Character is allowed to set banker flag.
	if (changeBanker && !guild->canSetBankerFlag(pCharacter)) {
		mLog->error(pCharacter->getName() + " attempted to set banker flag but they are not allowed.");
		return false;
	}

	// Check: Character is allowed to set alt flag.
	if (changeAlt && !setSelf && !guild->canSetAltFlag(pCharacter)) {
		mLog->error(pCharacter->getName() + " attempted to set alt flag but they are not allowed.");
		return false;
	}

	// Notify Guild.
	guild->onSetFlags(member, pFlags);

	// Save.
	if (!save()) mLog->error("Save failed in " + String(__FUNCTION__));

	return true;
}

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
