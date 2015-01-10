#include "GuildManager.h"
#include "ServiceLocator.h"
#include "Utility.h"
#include "Limits.h"
#include "Character.h"
#include "ZoneClientConnection.h"
#include "../common/EQPacket.h"
#include "Payload.h"
#include "ZoneManager.h"
#include "Zone.h"
#include "Profile.h"

#include "../common/tinyxml/tinyxml.h"

const bool GuildManager::initialise(DataStore* pDataStore, ZoneManager* pZoneManager) {
	EXPECTED_BOOL(mInitialised == false);
	EXPECTED_BOOL(pDataStore);
	EXPECTED_BOOL(pZoneManager);

	mDataStore = pDataStore;
	mZoneManager = pZoneManager;

	Profile p("GuildManager::initialise");
	Log::status("[Guild Manager] Initialising.");

	for (auto i = 0; i < Limits::Guild::MAX_GUILDS; i++) {
		mGuildNames[i][0] = '\0';
	}

	TiXmlDocument document;
	bool loaded = document.LoadFile("./data/guilds.xml");
	if (!loaded) {
		Log::error("[Guild Manager] Failed to load data.");
		return false;
	}

	// Read Guild data.
	TiXmlElement* guildElement = document.FirstChildElement("guilds")->FirstChildElement("guild");
	while (guildElement) {
		Guild* guild = new Guild(); 
		mGuilds.push_back(guild);

		// ID
		Utility::stou32Safe(guild->mID, String(guildElement->Attribute("id")));
		EXPECTED_BOOL(Limits::Guild::ID(guild->mID));
		// Name
		guild->mName = guildElement->Attribute("name");
		EXPECTED_BOOL(Limits::Guild::nameLength(guild->mName));
		// MOTD
		guild->mMOTD = guildElement->Attribute("motd");
		EXPECTED_BOOL(Limits::Guild::MOTDLength(guild->mMOTD));
		// MOTD Setter
		guild->mMOTDSetter = guildElement->Attribute("motd_setter");
		EXPECTED_BOOL(Limits::Character::nameLength(guild->mName));
		// URL
		guild->mURL = guildElement->Attribute("url");
		EXPECTED_BOOL(Limits::Guild::urlLength(guild->mURL));
		// Channel
		guild->mChannel = guildElement->Attribute("channel");
		EXPECTED_BOOL(Limits::Guild::channelLength(guild->mChannel));

		// Read GuildMember data.
		TiXmlElement* memberElement = guildElement->FirstChildElement("members")->FirstChildElement("member");
		while (memberElement) {
			GuildMember* member = new GuildMember();
			member->mGuild = guild;
			EXPECTED_BOOL(Utility::stou8Safe(member->mRank, String(memberElement->Attribute("rank"))));
			EXPECTED_BOOL(Utility::stou32Safe(member->mLevel, String(memberElement->Attribute("level"))));
			EXPECTED_BOOL(Utility::stou32Safe(member->mClass, String(memberElement->Attribute("class"))));
			EXPECTED_BOOL(Limits::Character::classID(member->mClass));
			member->mBanker = Utility::stobool(String(memberElement->Attribute("banker")));
			EXPECTED_BOOL(Utility::stou32Safe(member->mTimeLastOn, String(memberElement->Attribute("time_last_on"))));
			member->mTributeEnabled = Utility::stobool(String(memberElement->Attribute("tribute_enabled")));
			EXPECTED_BOOL(Utility::stou32Safe(member->mTotalTribute, String(memberElement->Attribute("total_tribute"))));
			EXPECTED_BOOL(Utility::stou32Safe(member->mLastTribute, String(memberElement->Attribute("last_tribute"))));

			member->mName = String(memberElement->Attribute("name"));
			EXPECTED_BOOL(Limits::Character::nameLength(member->mName));
			member->mPublicNote = String(memberElement->Attribute("public_note"));
			EXPECTED_BOOL(Limits::Guild::publicNoteLength(member->mPublicNote));

			guild->mMembers.push_back(member);
			memberElement = memberElement->NextSiblingElement();
		}

		guildElement = guildElement->NextSiblingElement();
	}

	// Store Guild names for packets.
	for (auto i : mGuilds) {
		_storeGuildName(i->mID, i->mName);
	}

	StringStream ss; ss << "[Guild Manager] Loaded data for " << mGuilds.size() << " Guilds.";
	Log::info(ss.str());

	mInitialised = true;
	return true;
}

void GuildManager::handleCreate(Character* pCharacter, const String pGuildName) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->hasGuild() == false);

	// Check: Guild name already exists.
	if (_findByGuildName(pGuildName)) {
		pCharacter->getConnection()->sendSimpleMessage(MessageType::Red, StringID::GUILD_NAME_IN_USE);
		return;
	}

	Guild* guild = new Guild();
	guild->mID = getNextGuildID();
	guild->mName = pGuildName;
	mGuilds.push_back(guild);

	_storeGuildName(guild->mID, guild->mName);

	// Tell Zone that something has changed.
	// TODO: This needs to be sent globally
	pCharacter->getZone()->notifyGuildsChanged();

	GuildMember* member = new GuildMember();
	member->mRank = GuildRanks::Leader;
	member->mName = pCharacter->getName();
	member->mClass = pCharacter->getClass();
	member->mLevel = pCharacter->getLevel();
	member->mZoneID = pCharacter->getZone()->getID();
	member->mInstanceID = pCharacter->getZone()->getInstanceID();
	member->mGuild = guild;

	guild->mMembers.push_back(member);
	guild->mOnlineMembers.push_back(pCharacter);

	pCharacter->setGuild(guild, guild->mID, GuildRanks::Leader, guild->mName);

	// Notify the Zone of the Character joining.
	pCharacter->getZone()->notifyCharacterGuildChange(pCharacter);
	_sendGuildInformation(pCharacter);
	_save();
}

void GuildManager::handleDelete(Character* pCharacter) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->hasGuild());
	EXPECTED(pCharacter->getGuildRank() == GuildRanks::Leader);

	Guild* guild = pCharacter->getGuild();

	for (auto i : guild->mOnlineMembers) {
		pCharacter->clearGuild();

		// Notify all zones of all online Characters.
		i->getZone()->notifyCharacterGuildChange(pCharacter);
	}

	mGuilds.remove(guild);
	delete guild;

	_save();
}

void GuildManager::handleRemove(Character* pCharacter, const String& pRemoveCharacterName) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->hasGuild());
	EXPECTED(Limits::Character::nameLength(pRemoveCharacterName));

	// NOTE: UF Prevents a Guild leader from removing them self but we still need to check it.
	// "You must transfer leadership or delete the guild before removing yourself."

	const bool removeOther = (pCharacter->getName() != pRemoveCharacterName);
	const bool removeSelf = !removeOther;

	// Check: Prevent leader from removing self.
	if (removeSelf)
		EXPECTED(isLeader(pCharacter) == false);

	// Check: Prevent non-leader or non-officer from removing other members.
	if (removeOther)
		EXPECTED(isLeader(pCharacter) || isOfficer(pCharacter));

	Guild* guild = pCharacter->getGuild();
	EXPECTED(guild);
	GuildMember* member = guild->getMember(pRemoveCharacterName);
	EXPECTED(member);

	// Check: Prevent lower or same ranked removal
	// Allows Leader to remove: Officer + Member
	// Allows Officer to remove: Member
	if (removeOther)
		EXPECTED(member->mRank < pCharacter->getGuildRank());

	guild->mMembers.remove(member);
	safe_delete(member);

	// If the Character being removed is online we need to update.
	// NOTE: If the character is not online 
	Character* removeCharacter = removeSelf ? pCharacter : mZoneManager->findCharacter(pRemoveCharacterName, true);
	if (removeCharacter) {
		guild->mOnlineMembers.remove(removeCharacter);
		removeCharacter->clearGuild();
		if (removeCharacter->isZoning() == false) {
			removeCharacter->getZone()->notifyCharacterGuildChange(removeCharacter);
			removeCharacter->getConnection()->sendGuildMembers(std::list<GuildMember*>()); // NOTE: This clears the guild window member list on the removed client.
			removeCharacter->getConnection()->sendGuildMOTD("", "");
			removeCharacter->getConnection()->sendGuildChannel("");
			removeCharacter->getConnection()->sendGuildURL("");
		}
	}

	if (removeOther)
		pCharacter->getConnection()->sendMessage(MessageType::Yellow, "Successfully removed " + pRemoveCharacterName + " from the guild.");

	// Update other guild members.
	_sendMemberRemoved(guild, pRemoveCharacterName);
	_sendMembers(guild);
	_save();
}

void GuildManager::handleInviteSent(Character* pCharacter, const String& pInviteCharacterName) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->hasGuild());
	EXPECTED(isLeader(pCharacter) || isOfficer(pCharacter));

	auto guild = pCharacter->getGuild();

	auto character = mZoneManager->findCharacter(pInviteCharacterName);

	// Character to be invited was not found.
	if (!character) {
		pCharacter->message(MessageType::Yellow, pInviteCharacterName + " could not be found.");
		return;
	}
	// Character to be invited already has a Guild.
	if (character->hasGuild()) {
		pCharacter->message(MessageType::Yellow, pInviteCharacterName + " already has a guild.");
		return;
	}
	// Character to be invited already has a pending Guild invite.
	if (character->hasPendingGuildInvite()) {
		pCharacter->message(MessageType::Yellow, pInviteCharacterName + " is considering joining another guild.");
		return;
	}

	character->setPendingGuildInviteID(guild->mID);
	character->setPendingGuildInviteName(pCharacter->getName());
	character->getConnection()->sendGuildInvite(pCharacter->getName(), guild->mID);
}

void GuildManager::handleInviteAccept(Character* pCharacter, const String& pInviterName) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->hasGuild() == false); // Check: Sanity.
	EXPECTED(pInviterName == pCharacter->getPendingGuildInviteName());
	Guild* guild = _findByID(pCharacter->getPendingGuildInviteID());
	EXPECTED(guild);

	GuildMember* member = new GuildMember();
	member->mName = pCharacter->getName();
	member->mRank = GuildRanks::Member;
	member->mClass = pCharacter->getClass();
	member->mLevel = pCharacter->getLevel();
	member->mGuild = guild;
	member->mZoneID = pCharacter->getZone()->getID();
	member->mInstanceID = pCharacter->getZone()->getInstanceID();

	guild->mMembers.push_back(member);
	guild->mOnlineMembers.push_back(pCharacter);

	pCharacter->setGuild(guild, guild->mID, GuildRanks::Member, guild->mName);
	pCharacter->clearPendingGuildInvite();

	// Notify the inviter (Zoning characters are ignored for now.)
	auto inviter = mZoneManager->findCharacter(pInviterName);
	if (inviter)
		inviter->message(MessageType::Yellow, pCharacter->getName() + " has accepted your invitation to join the guild.");

	// Update the zone of the joining member.
	pCharacter->getZone()->notifyCharacterGuildChange(pCharacter);
	// Update the joining member.
	_sendGuildInformation(pCharacter);
	// Update other guild members.
	_sendMembers(guild);
	_save();
}

void GuildManager::handleInviteDecline(Character* pCharacter, const String& pInviterName) {
	EXPECTED(pCharacter);
	EXPECTED(pInviterName == pCharacter->getPendingGuildInviteName());
	pCharacter->clearPendingGuildInvite();

	// Notify the inviter (Zoning characters are ignored for now.)
	Character* inviter = mZoneManager->findCharacter(pInviterName);
	if (inviter)
		inviter->message(MessageType::Yellow, pCharacter->getName() + " has declined your invitation to join the guild.");
}

Guild* GuildManager::_findByGuildName(const String& pGuildName) {
	for (auto i : mGuilds) {
		if (i->mName == pGuildName)
			return i;
	}

	return nullptr;
}

void GuildManager::_save() {
	Profile p("GuildManager::_save");

	TiXmlDocument document;
	TiXmlElement* guildsElement = new TiXmlElement("guilds");

	for (auto i : mGuilds) {
		TiXmlElement* guildElement = new TiXmlElement("guild");
		guildElement->SetAttribute("id", i->mID);
		guildElement->SetAttribute("name", i->mName.c_str());
		guildElement->SetAttribute("motd", i->mMOTD.c_str());
		guildElement->SetAttribute("motd_setter", i->mMOTDSetter.c_str());
		guildElement->SetAttribute("url", i->mURL.c_str());
		guildElement->SetAttribute("channel", i->mChannel.c_str());

		TiXmlElement* membersElement = new TiXmlElement("members");
		for (auto j : i->mMembers) {
			TiXmlElement* memberElement = new TiXmlElement("member");
			memberElement->SetAttribute("name", j->mName.c_str());
			memberElement->SetAttribute("rank", j->mRank);
			memberElement->SetAttribute("level", j->mLevel);
			memberElement->SetAttribute("banker", j->mBanker);
			memberElement->SetAttribute("class", j->mClass);
			memberElement->SetAttribute("time_last_on", j->mTimeLastOn);
			memberElement->SetAttribute("tribute_enabled", j->mTributeEnabled);
			memberElement->SetAttribute("total_tribute", j->mTotalTribute);
			memberElement->SetAttribute("banker", j->mBanker);
			memberElement->SetAttribute("last_tribute", j->mLastTribute);
			memberElement->SetAttribute("public_note", j->mPublicNote.c_str());

			membersElement->LinkEndChild(memberElement);
		}
		guildElement->LinkEndChild(membersElement);
		guildsElement->LinkEndChild(guildElement);
	}

	document.LinkEndChild(guildsElement);
	EXPECTED(document.SaveFile("./data/guilds.xml"));
}

uint32 GuildManager::getNextGuildID() {
	return getHighestGuildID() + 1;
}

std::list<String> GuildManager::getGuildNames() {
	std::list<String> guildNames;
	for (auto i : mGuilds) {
		guildNames.push_back(i->mName);
	}

	return guildNames;
}

void GuildManager::_storeGuildName(GuildID pGuildID, String pGuildName) {
	// TODO: Error checking.
	strcpy(&mGuildNames[pGuildID + 1][0], pGuildName.c_str());
}

GuildID GuildManager::getHighestGuildID() {
	GuildID highest = 0;
	for (auto i : mGuilds) {
		if (i->mID > highest)
			highest = i->mID;
	}
	return highest;
}

GuildSearchResults GuildManager::getAllGuilds() {
	GuildSearchResults results;
	for (auto i : mGuilds) {
		results.push_back({i->mID, i->mName});
	}

	return results;
}

void GuildManager::onConnect(Character* pCharacter, uint32 pGuildID) {
	Guild* guild = _findByID(pGuildID);

	// Check: Guild does not exist. It was probably deleted.
	if (!guild) {
		pCharacter->clearGuild();
		return;
	}

	// Check: Character still belongs to this guild.
	bool found = false;
	for (auto i : guild->mMembers) {
		if (i->mName == pCharacter->getName()) {
			
			pCharacter->setGuild(guild, guild->mID, i->mRank, guild->mName);
			_sendMessage(guild, SYS_NAME, pCharacter->getName() + " has come online!");
			guild->mOnlineMembers.push_back(pCharacter);

			// Update member details.
			// NOTE: Membership information should always be up to date unless there was external changes to the Character.
			i->mName = pCharacter->getName();
			i->mClass = pCharacter->getClass();
			i->mLevel = pCharacter->getLevel();
			i->mCharacter = pCharacter;

			found = true;
			break;
		}
	}

	// Character was not found in Guild members (save de-sync or removed while offline/zoning?)
	if (!found) {
		pCharacter->clearGuild();
	}
}

Guild* GuildManager::_findByID(const GuildID pID) {
	for (auto i : mGuilds) {
		if (i->mID == pID)
			return i;
	}

	return nullptr;
}

void GuildManager::onEnterZone(Character* pCharacter) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->hasGuild());

	GuildMember* member = pCharacter->getGuild()->getMember(pCharacter->getName());
	EXPECTED(member);
	Zone* zone = pCharacter->getZone();
	EXPECTED(zone);
	Guild* guild = pCharacter->getGuild();
	EXPECTED(guild);
	ZoneConnection* connection = pCharacter->getConnection();
	EXPECTED(connection);

	// Update member details.
	member->mZoneID = zone->getID();
	member->mInstanceID = zone->getInstanceID();

	// Update other members.
	_sendMemberZoneUpdate(guild, member);
	// Update Character.
	_sendGuildInformation(pCharacter);
}

void GuildManager::onLeaveZone(Character* pCharacter) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->hasGuild());

	// TODO: Do I need to zero member zone id / instance id?
}

void GuildManager::onCamp(Character* pCharacter){
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->hasGuild());

	// Update member.
	GuildMember* member = pCharacter->getGuild()->getMember(pCharacter->getName());
	EXPECTED(member);
	member->mZoneID = 0;
	member->mInstanceID = 0;
	member->mTimeLastOn = 0; // TODO: Time
	member->mCharacter = nullptr;

	pCharacter->getGuild()->mOnlineMembers.remove(pCharacter);
	_sendMessage(pCharacter->getGuild(), SYS_NAME, pCharacter->getName() + " has gone offline (Camped).");

	// Update other members.
	_sendMemberZoneUpdate(pCharacter->getGuild(), member);
}

void GuildManager::onLinkdead(Character* pCharacter) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->hasGuild());

	// Update member.
	GuildMember* member = pCharacter->getGuild()->getMember(pCharacter->getName());
	EXPECTED(member);
	member->mZoneID = 0;
	member->mInstanceID = 0;
	member->mTimeLastOn = 0; // TODO: Time
	member->mCharacter = nullptr;

	pCharacter->getGuild()->mOnlineMembers.remove(pCharacter);
	_sendMessage(pCharacter->getGuild(), SYS_NAME, pCharacter->getName() + " has gone offline (Linkdead).", pCharacter);

	// Update other members.
	_sendMemberZoneUpdate(pCharacter->getGuild(), member);
}

void GuildManager::onLevelChange(Character* pCharacter) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->hasGuild());

	// Update member details.
	GuildMember* member = pCharacter->getGuild()->getMember(pCharacter->getName());
	EXPECTED(member);

	uint32 previousLevel = member->mLevel;
	member->mLevel = pCharacter->getLevel();

	// Notify guild members.
	if (member->mLevel > previousLevel) {
		StringStream ss; ss << pCharacter->getName() << " is now level " << member->mLevel << "!";
		_sendMessage(pCharacter->getGuild(), SYS_NAME, ss.str());
	}
	// Update other members.
	// NOTE: This does not work.....
	//_sendMemberLevelUpdate(member->mGuild, member);

	// TODO: Use _sendMembers until a better way is found.
	_sendMembers(member->mGuild);
}


void GuildManager::handleMessage(Character* pCharacter, const String& pMessage) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->hasGuild());

	_sendMessage(pCharacter->getGuild(), pCharacter->getName(), pMessage);
}

void GuildManager::_sendMessage(Guild* pGuild, const String& pSenderName, const String& pMessage, Character* pExclude) {
	EXPECTED(pGuild);

	for (auto i : pGuild->mOnlineMembers) {
		if (i == pExclude) continue;;

		// Check: Where a guild member is zoning, queue the message.
		if (i->isZoning()) {
			i->addQueuedMessage(ChannelID::Guild, pSenderName, pMessage);
			continue;
		}
		i->getConnection()->sendGuildMessage(pSenderName, pMessage);
	}
}

void GuildManager::handleSetMOTD(Character* pCharacter, const String& pMOTD) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->hasGuild());
	EXPECTED(Limits::Guild::MOTDLength(pMOTD));
	EXPECTED(isLeader(pCharacter) || isOfficer(pCharacter)); // Only leader or officers can set the MOTD.

	Guild* guild = pCharacter->getGuild();
	guild->mMOTD = pMOTD;
	guild->mMOTDSetter = pCharacter->getName();

	// Update other members.
	_sendMOTD(guild);
	_save();
}

void GuildManager::handleGetMOTD(Character* pCharacter) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->hasGuild());

	_sendMOTDReply(pCharacter);
}

void GuildManager::_sendMOTD(const Guild* pGuild) {
	EXPECTED(pGuild);

	auto outPacket = new EQApplicationPacket(OP_GuildMOTD, sizeof(Payload::Guild::MOTD));
	auto payload = reinterpret_cast<Payload::Guild::MOTD*>(outPacket->pBuffer);
	strcpy(payload->mSetByName, pGuild->mMOTDSetter.c_str());
	strcpy(payload->mMOTD, pGuild->mMOTD.c_str());

	const String motd = pGuild->mMOTD;
	for (auto i : pGuild->mOnlineMembers) {
		if (i->isZoning()) { continue; }
		strcpy(payload->mCharacterName, i->getName().c_str());
		i->getConnection()->sendPacket(outPacket);
	}

	safe_delete(outPacket);
}

void GuildManager::_sendURL(Guild* pGuild) {
	EXPECTED(pGuild);

	auto outPacket = new EQApplicationPacket(OP_GuildUpdateURLAndChannel, sizeof(Payload::Guild::GuildUpdate));
	auto payload = reinterpret_cast<Payload::Guild::GuildUpdate*>(outPacket->pBuffer);
	payload->mAction = Payload::Guild::GuildUpdate::Action::GUILD_URL;
	strcpy(&payload->mText[0], pGuild->mURL.c_str());

	for (auto i : pGuild->mOnlineMembers) {
		if (i->isZoning()) { continue; }
		i->getConnection()->sendPacket(outPacket);
	}

	safe_delete(outPacket);
}

void GuildManager::_sendChannel(Guild* pGuild) {
	EXPECTED(pGuild);

	auto outPacket = new EQApplicationPacket(OP_GuildUpdateURLAndChannel, sizeof(Payload::Guild::GuildUpdate));
	auto payload = reinterpret_cast<Payload::Guild::GuildUpdate*>(outPacket->pBuffer);
	payload->mAction = Payload::Guild::GuildUpdate::Action::GUILD_CHANNEL;
	strcpy(&payload->mText[0], pGuild->mChannel.c_str());

	for (auto i : pGuild->mOnlineMembers) {
		if (i->isZoning()) { continue; }
		i->getConnection()->sendPacket(outPacket);
	}

	safe_delete(outPacket);
}

void GuildManager::_sendMOTDReply(Character* pCharacter) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->hasGuild());

	Guild* guild = pCharacter->getGuild();
	pCharacter->getConnection()->sendGuildMOTDReply(guild->mMOTD, guild->mMOTDSetter);
}

void GuildManager::_sendGuildInformation(Character* pCharacter) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->hasGuild());

	ZoneConnection* connection = pCharacter->getConnection();
	EXPECTED(connection);
	Guild* guild = pCharacter->getGuild();
	EXPECTED(guild);

	connection->sendGuildRank();
	connection->sendGuildMembers(guild->mMembers);
	connection->sendGuildURL(guild->mURL);
	connection->sendGuildChannel(guild->mChannel);
	connection->sendGuildMOTD(guild->mMOTD, guild->mMOTDSetter);
}

void GuildManager::_sendMembers(Guild* pGuild) {
	EXPECTED(pGuild);

	for (auto i : pGuild->mOnlineMembers) {
		if (i->isZoning()) { continue; }
		i->getConnection()->sendGuildMembers(pGuild->mMembers);
	}
}

void GuildManager::_sendMemberRemoved(const Guild* pGuild, const String& pRemoveCharacterName) {
	using namespace Payload::Guild;
	EXPECTED(pGuild);
	EXPECTED(Limits::Character::nameLength(pRemoveCharacterName));

	auto outPacket = new EQApplicationPacket(OP_GuildManageRemove, Remove::size());
	auto payload = Remove::convert(outPacket->pBuffer);

	payload->mGuildID = pGuild->mID;
	strcpy(payload->mCharacterName, pRemoveCharacterName.c_str());

	for (auto i : pGuild->mOnlineMembers) {
		if (i->isZoning()) { continue; }
		i->getConnection()->sendPacket(outPacket);
	}

	safe_delete(outPacket);
}


void GuildManager::_sendMemberZoneUpdate(const Guild* pGuild, const GuildMember* pMember) {
	EXPECTED(pGuild);
	EXPECTED(pMember);
	EXPECTED(pMember->mGuild == pGuild);

	auto outPacket = new EQApplicationPacket(OP_GuildMemberUpdate, sizeof(Payload::Guild::MemberUpdate));
	auto payload = reinterpret_cast<Payload::Guild::MemberUpdate*>(outPacket->pBuffer);

	payload->mGuildID = pGuild->mID;
	strcpy(payload->mMemberName, pMember->mName.c_str());
	payload->mZoneID = pMember->mZoneID;
	payload->mInstanceID = pMember->mInstanceID;
	payload->mLastSeen = 0; // TODO: Time

	for (auto i : pGuild->mOnlineMembers) {
		if (i->isZoning()) { continue; }
		i->getConnection()->sendPacket(outPacket);
	}

	safe_delete(outPacket);
}

void GuildManager::_sendMemberLevelUpdate(const Guild* pGuild, const GuildMember* pMember) {
	EXPECTED(pGuild);
	EXPECTED(pMember);
	EXPECTED(pMember->mGuild == pGuild);

	auto outPacket = new EQApplicationPacket(OP_GuildMemberLevelUpdate, sizeof(Payload::Guild::LevelUpdate));
	auto payload = reinterpret_cast<Payload::Guild::LevelUpdate*>(outPacket->pBuffer);

	payload->mGuildID = pGuild->mID;
	strcpy(payload->mMemberName, pMember->mName.c_str());
	payload->mLevel = pMember->mLevel;

	for (auto i : pGuild->mOnlineMembers) {
		if (i->isZoning()) { continue; }
		i->getConnection()->sendPacket(outPacket);
	}

	safe_delete(outPacket);
}

bool GuildManager::isLeader(Character* pCharacter){
	EXPECTED_BOOL(pCharacter);
	EXPECTED_BOOL(pCharacter->hasGuild());

	GuildMember* member = pCharacter->getGuild()->getMember(pCharacter->getName());
	EXPECTED_BOOL(member);

	return member->mRank == GuildRanks::Leader;
}

bool GuildManager::isOfficer(Character* pCharacter){
	EXPECTED_BOOL(pCharacter);
	EXPECTED_BOOL(pCharacter->hasGuild());

	GuildMember* member = pCharacter->getGuild()->getMember(pCharacter->getName());
	EXPECTED_BOOL(member);

	return member->mRank == GuildRanks::Officer;
}

void GuildManager::handleSetURL(Character* pCharacter, const String& pURL) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->hasGuild());
	EXPECTED(isLeader(pCharacter)); // Only leader can change the url.
	EXPECTED(Limits::Guild::urlLength(pURL));

	pCharacter->getGuild()->mURL = pURL;
	_save();

	// Update other members.
	_sendURL(pCharacter->getGuild());
	_sendMessage(pCharacter->getGuild(), SYS_NAME, pCharacter->getName() + " has updated the guild URL.");
}

void GuildManager::handleSetChannel(Character* pCharacter, const String& pChannel) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->hasGuild());
	EXPECTED(isLeader(pCharacter)); // Only leader can change the channel.
	EXPECTED(Limits::Guild::channelLength(pChannel));

	pCharacter->getGuild()->mChannel = pChannel;
	_save();

	// Update other members.
	_sendChannel(pCharacter->getGuild());
	_sendMessage(pCharacter->getGuild(), SYS_NAME, pCharacter->getName() + " has updated the guild channel.");
}

void GuildManager::handleSetPublicNote(Character* pCharacter, const String& pCharacterName, const String& pNote) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->hasGuild());
	EXPECTED(Limits::Character::nameLength(pCharacterName));
	EXPECTED(Limits::Guild::publicNoteLength(pNote));

	if (pCharacter->getName() != pCharacterName)
		EXPECTED(isLeader(pCharacter) || isOfficer(pCharacter)); // Only leader or officers can change another Character's public note.

	Guild* guild = pCharacter->getGuild();
	GuildMember* member = guild->getMember(pCharacterName);
	EXPECTED(member);
	member->mPublicNote = pNote;
	_save();

	// Update other members.
	// TODO: Use _sendMembers until a better way is found.
	_sendMembers(guild);
}

void GuildManager::handleStatusRequest(Character* pCharacter, const String& pCharacterName) {
	EXPECTED(pCharacter);
	EXPECTED(Limits::Character::nameLength(pCharacterName));

	ZoneConnection* connection = pCharacter->getConnection();
	EXPECTED(connection);

	// Try to find requested Character.
	GuildMember* member = _findByCharacterName(pCharacterName);
	if (!member) {
		// Character was not found, they either do not exist or do not have a guild.
		connection->sendSimpleMessage(MessageType::White, StringID::NOT_IN_A_GUILD, pCharacterName);
		return;
	}

	u32 stringID = StringID::SI_NONE;
	String message = "";

	// Same Guild
	if (member->mGuild == pCharacter->getGuild()) {
		message = pCharacterName;
		if (member->mRank == GuildRanks::Leader) { stringID = StringID::LEADER_OF_YOUR_GUILD; }
		else if (member->mRank == GuildRanks::Officer) { stringID = StringID::OFFICER_OF_YOUR_GUILD; }
		else if (member->mRank == GuildRanks::Member){ stringID = StringID::MEMBER_OF_YOUR_GUILD; }
	}
	// Different Guild
	else {
		message = member->mGuild->mName;
		if (member->mRank == GuildRanks::Leader) { stringID = StringID::LEADER_OF_X_GUILD; }
		else if (member->mRank == GuildRanks::Officer) { stringID = StringID::OFFICER_OF_X_GUILD; }
		else if (member->mRank == GuildRanks::Member){ stringID = StringID::MEMBER_OF_X_GUILD; }
	}

	if (stringID != StringID::SI_NONE)
		connection->sendSimpleMessage(MessageType::White, stringID, message);
}

void GuildManager::handlePromote(Character* pCharacter, const String& pPromoteName) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->hasGuild());
	EXPECTED(isLeader(pCharacter)); // Only the guild leader can promote others.
	EXPECTED(Limits::Character::nameLength(pPromoteName));
	EXPECTED(pCharacter->getName() != pPromoteName); // Check: Sanity.

	GuildMember* member = pCharacter->getGuild()->getMember(pPromoteName);
	EXPECTED(member);

	if (member->mRank != GuildRanks::Member) {
		pCharacter->getConnection()->sendMessage(MessageType::Yellow, pPromoteName + " can not be promoted beyond officer.");
		return;
	}

	_changeRank(member, GuildRanks::Officer);
	_sendMembers(pCharacter->getGuild());
	_save();
}

void GuildManager::handleDemote(Character* pCharacter, const String& pDemoteName) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->hasGuild());
	EXPECTED(isLeader(pCharacter) || isOfficer(pCharacter));
	EXPECTED(Limits::Character::nameLength(pDemoteName));
	Guild* guild = pCharacter->getGuild();
	EXPECTED(guild);

	// Officer demotes self.
	if (pCharacter->getName() == pDemoteName) {
		EXPECTED(isOfficer(pCharacter));
		GuildMember* member = guild->getMember(pCharacter->getName());
		EXPECTED(member);
		_changeRank(member, GuildRanks::Member);
	}
	// Leader demotes officer
	else {
		EXPECTED(isLeader(pCharacter));
		GuildMember* member = guild->getMember(pDemoteName);
		EXPECTED(member->mRank == GuildRanks::Officer);
		_changeRank(member, GuildRanks::Member);
	}

	_sendMembers(guild);
	_save();
}

void GuildManager::handleSetBanker(Character* pCharacter, const String& pOtherName, const bool pBanker) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->hasGuild());
	EXPECTED(isLeader(pCharacter));
	EXPECTED(Limits::Character::nameLength(pOtherName));

	GuildMember* member = pCharacter->getGuild()->getMember(pOtherName);
	EXPECTED(member);

	if (member->mBanker != pBanker) {
		member->mBanker = pBanker;
		_save();
		// TODO: Use _sendMembers until a better way is found.
		_sendMembers(member->mGuild);
		return;
	}
}

void GuildManager::handleSetAlt(Character* pCharacter, const String& pAltName, const bool pAlt) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->hasGuild());
	EXPECTED(Limits::Character::nameLength(pAltName));

	GuildMember* member = pCharacter->getGuild()->getMember(pAltName);
	EXPECTED(member);

	// Check: Return early if member is already an alt.
	if (member->mAlt == pAlt)
		return;

	// Check: Permission
	const bool changingOther = (pCharacter->getName() != pAltName);
	if (changingOther)
		EXPECTED(isLeader(pCharacter) || isOfficer(pCharacter)); // Only the leader or an officer can change the alt status of another member.

	member->mAlt = pAlt;
	_save();
	// TODO: Use _sendMembers until a better way is found.
	_sendMembers(member->mGuild);
}

GuildMember* GuildManager::_findByCharacterName(const String& pCharacterName) {
	EXPECTED_PTR(Limits::Character::nameLength(pCharacterName));

	for (auto i : mGuilds) {
		for (auto j : i->mMembers) {
			if (j->mName == pCharacterName)
				return j;
		}
	}

	// Not found.
	return nullptr;
}

void GuildManager::handleMakeLeader(Character* pCharacter, const String& pLeaderName) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacter->hasGuild());
	EXPECTED(isLeader(pCharacter));
	EXPECTED(Limits::Character::nameLength(pLeaderName));

	Guild* guild = pCharacter->getGuild();
	EXPECTED(guild);
	GuildMember* oldLeader = guild->getMember(pCharacter->getName());
	EXPECTED(oldLeader);
	GuildMember* newLeader = guild->getMember(pLeaderName);
	EXPECTED(newLeader);

	_changeRank(oldLeader, GuildRanks::Officer);
	_changeRank(newLeader, GuildRanks::Leader);
	_sendMembers(guild);
	_save();
}

void GuildManager::_changeRank(GuildMember* pMember, const GuildRank pNewRank) {
	EXPECTED(pMember);
	EXPECTED(Limits::Guild::rankValid(pNewRank));

	Guild* guild = pMember->mGuild;
	EXPECTED(guild);

	const GuildRank	previousRank = pMember->mRank;
	pMember->mRank = pNewRank;

	// Check: Character is online, updates required!
	if (pMember->isOnline()) {
		Character* character = pMember->mCharacter;
		EXPECTED(character);

		character->setGuildRank(pNewRank);

		// Check: Character is not zoning, updates required!
		if (character->isZoning() == false) {
			// Update Character (Rank).
			character->getConnection()->sendGuildRank();
			// Update Character Zone (Rank).
			character->getZone()->notifyCharacterGuildChange(character);
		}
	}

	// Notify online guild members.
	const String change = previousRank < pNewRank ? "promoted" : "demoted";
	const String rank = Utility::guildRankToString(pNewRank);
	_sendMessage(guild, SYS_NAME, pMember->mName + " has been " + change + " to " + rank);
}