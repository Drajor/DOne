#include "GuildManager.h"

#include "Utility.h"
#include "Limits.h"
#include "Character.h"
#include "ZoneClientConnection.h"
#include "ZoneManager.h"
#include "Zone.h"
#include "Profile.h"

#include "../common/tinyxml/tinyxml.h"

bool GuildManager::initialise() {
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
			EXPECTED_BOOL(Utility::stou32Safe(member->mID, String(memberElement->Attribute("id"))));
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

			// Read GuildMember::PersonalNotes data.
			TiXmlElement* noteElement = memberElement->FirstChildElement("notes")->FirstChildElement("note");
			while (noteElement) {
				String characterName = noteElement->Attribute("name");
				EXPECTED_BOOL(Limits::Character::nameLength(characterName));
				String noteValue = noteElement->Attribute("value");
				EXPECTED_BOOL(Limits::Guild::personalNoteLength(noteValue));
				member->mPersonalNotes.push_back({ characterName, noteValue });

				noteElement = noteElement->NextSiblingElement();
			}

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

	return true;
}

void GuildManager::handleCreate(Character* pCharacter, const String pGuildName) {
	ARG_PTR_CHECK(pCharacter);
	EXPECTED(pCharacter->hasGuild() == false);

	// Check: Guild name already exists.
	if (_findGuildByName(pGuildName)) {
		pCharacter->getConnection()->sendSimpleMessage(MessageType::Red, StringID::GUILD_NAME_IN_USE);
		return;
	}

	Guild* guild = new Guild();
	guild->mID = getNextGuildID();
	guild->mName = pGuildName;
	mGuilds.push_back(guild);

	_storeGuildName(guild->mID, guild->mName);

	// Tell Zone that something has changed.
	pCharacter->getZone()->notifyGuildsChanged();

	GuildMember* member = new GuildMember();
	member->mID = pCharacter->getID();
	member->mRank = GuildRanks::Leader;
	guild->mMembers.push_back(member);
	guild->mOnlineMembers.push_back(pCharacter);

	pCharacter->setGuild(guild, guild->mID, GuildRanks::Leader);

	// Notify the Zone of the Character joining.
	pCharacter->getZone()->notifyCharacterGuildChange(pCharacter);

	pCharacter->getConnection()->sendGuildRank();

	_save();
	return;
}

void GuildManager::handleDelete(Character* pCharacter) {
	ARG_PTR_CHECK(pCharacter);
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
	ARG_PTR_CHECK(pCharacter);
	EXPECTED(pCharacter->hasGuild());

	// NOTE: UF Prevents a Guild leader from removing them self but we still need to check it.
	// "You must transfer leadership or delete the guild before removing yourself."

	// Self remove.
	if (pCharacter->getName() == pRemoveCharacterName) {
		// Prevent leader removing them self (requires packet forging).
		if (pCharacter->getGuildRank() == GuildRanks::Leader) {
			// TODO: Log error.
			return;
		}

		Guild* guild = pCharacter->getGuild();

		// Remove Character from Guild members.
		for (auto i = guild->mMembers.begin(); i != guild->mMembers.end(); i++) {
			if ((*i)->mID == pCharacter->getID()) {
				guild->mMembers.erase(i);
				break;
			}
		}
		// Remove Character from online members.
		guild->mOnlineMembers.remove(pCharacter);

		pCharacter->clearGuild();

		// Notify the Zone of the Character leaving.
		pCharacter->getZone()->notifyCharacterGuildChange(pCharacter);
	}

	// OP_GuildManageRemove
	_save();
}

void GuildManager::handleInviteSent(Character* pCharacter, const String& pInviteCharacterName) {
	ARG_PTR_CHECK(pCharacter);
	EXPECTED(pCharacter->hasGuild());

	auto guild = pCharacter->getGuild();

	// TODO: Check pCharacter is officer/leader.

	auto character = ZoneManager::getInstance().findCharacter(pInviteCharacterName);

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
	ARG_PTR_CHECK(pCharacter);
	Guild* guild = _findByID(pCharacter->getPendingGuildInviteID());
	EXPECTED(guild != nullptr);

	GuildMember* member = new GuildMember();
	member->mID = pCharacter->getID();
	member->mRank = GuildRanks::Member;
	guild->mMembers.push_back(member);
	guild->mOnlineMembers.push_back(pCharacter);

	pCharacter->setGuild(guild, guild->mID, GuildRanks::Member);
	pCharacter->clearPendingGuildInvite();

	// TODO: Notify Inviter.

	// Notify the Zone of the Character joining.
	pCharacter->getZone()->notifyCharacterGuildChange(pCharacter);
	pCharacter->getConnection()->sendGuildRank();
	_save();
}

void GuildManager::handleInviteDecline(Character* pCharacter, const String& InviterName) {
	ARG_PTR_CHECK(pCharacter);
	pCharacter->clearPendingGuildInvite();
}

Guild* GuildManager::_findGuildByName(const String pGuildName) {
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
			memberElement->SetAttribute("id", j->mID);
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
			
			TiXmlElement* notesElement = new TiXmlElement("notes");
			for (auto k : j->mPersonalNotes) {
				TiXmlElement* noteElement = new TiXmlElement("note");
				noteElement->SetAttribute("name", k.mName.c_str());
				noteElement->SetAttribute("value", k.mNote.c_str());

				notesElement->LinkEndChild(noteElement);
			}

			memberElement->LinkEndChild(notesElement);
			membersElement->LinkEndChild(memberElement);
		}
		guildElement->LinkEndChild(membersElement);
		guildsElement->LinkEndChild(guildElement);
	}

	document.LinkEndChild(guildsElement);
	document.SaveFile("./data/guilds.xml");
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

GuildManager::~GuildManager()
{

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
		if (i->mID == pCharacter->getID()) {
			
			pCharacter->setGuild(guild, guild->mID, i->mRank);
			_sendMessage(guild, SYS_NAME, pCharacter->getName() + " has come online!");
			guild->mOnlineMembers.push_back(pCharacter);

			// Update member details.
			// NOTE: Membership information should always be up to date unless there was external changes to the Character.
			i->mName = pCharacter->getName();
			i->mClass = pCharacter->getClass();
			i->mLevel = pCharacter->getLevel();

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
	ARG_PTR_CHECK(pCharacter);
	EXPECTED(pCharacter->hasGuild());

	GuildMember* member = pCharacter->getGuild()->getMember(pCharacter->getName());
	EXPECTED(member);
	Zone* zone = pCharacter->getZone();
	EXPECTED(zone);
	Guild* guild = pCharacter->getGuild();
	EXPECTED(guild);
	ZoneClientConnection* connection = pCharacter->getConnection();
	EXPECTED(connection);

	// Update member details.
	member->mZoneID = zone->getID();
	member->mInstanceID = zone->getInstanceID();

	connection->sendGuildMembers(guild->mMembers);
	connection->sendGuildURL(guild->mURL);
	connection->sendGuildChannel(guild->mChannel);
	connection->sendGuildMOTD(guild->mMOTD, guild->mMOTDSetter);
}

void GuildManager::onLeaveZone(Character* pCharacter) {
	ARG_PTR_CHECK(pCharacter);
	EXPECTED(pCharacter->hasGuild());

	// TODO: Do I need to zero member zone id / instance id?
}

void GuildManager::onCamp(Character* pCharacter){
	ARG_PTR_CHECK(pCharacter);
	EXPECTED(pCharacter->hasGuild());

	pCharacter->getGuild()->mOnlineMembers.remove(pCharacter);
	_sendMessage(pCharacter->getGuild(), SYS_NAME, pCharacter->getName() + " has gone offline (Camped).");
}

void GuildManager::onLinkdead(Character* pCharacter) {
	ARG_PTR_CHECK(pCharacter);
	EXPECTED(pCharacter->hasGuild());

	pCharacter->getGuild()->mOnlineMembers.remove(pCharacter);
	_sendMessage(pCharacter->getGuild(), SYS_NAME, pCharacter->getName() + " has gone offline (Linkdead).", pCharacter);
}

void GuildManager::onLevelChange(Character* pCharacter) {
	ARG_PTR_CHECK(pCharacter);
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
}


void GuildManager::handleMessage(Character* pCharacter, const String& pMessage) {
	ARG_PTR_CHECK(pCharacter);
	EXPECTED(pCharacter->hasGuild());

	_sendMessage(pCharacter->getGuild(), pCharacter->getName(), pMessage);
}

void GuildManager::_sendMessage(Guild* pGuild, const String& pSenderName, const String& pMessage, Character* pExclude) {
	ARG_PTR_CHECK(pGuild);

	for (auto i : pGuild->mOnlineMembers) {
		if (i == pExclude) continue;;

		// Check: Where a guild member is zoning, queue the message.
		if (i->isZoning()) {
			i->addQueuedMessage(ChannelID::CH_GUILD, pSenderName, pMessage);
			continue;
		}
		i->getConnection()->sendGuildMessage(pSenderName, pMessage);
	}
}

void GuildManager::handleSetMOTD(Character* pCharacter, const String& pMOTD) {
	ARG_PTR_CHECK(pCharacter);
	EXPECTED(pCharacter->hasGuild());
	EXPECTED(pMOTD.length() < Limits::Guild::MAX_MOTD_LENGTH); // NOTE: 'Less-Than' is used instead of 'Less-Than-Or-Equal-To' because std::string::length does not include the null terminator.
	// EXPECTED: pCharacter has permission.

	Guild* guild = pCharacter->getGuild();
	guild->mMOTD = pMOTD;
	guild->mMOTDSetter = pCharacter->getName();
	_sendMOTD(guild);
	_save();
}

void GuildManager::handleGetMOTD(Character* pCharacter) {
	ARG_PTR_CHECK(pCharacter);
	EXPECTED(pCharacter->hasGuild());

	_sendMOTD(pCharacter);
}

void GuildManager::_sendMOTD(Guild* pGuild) {
	ARG_PTR_CHECK(pGuild);

	const String motd = pGuild->mMOTD;
	for (auto i : pGuild->mOnlineMembers) {
		if (i->isZoning()) { continue; }
		i->getConnection()->sendGuildMOTD(pGuild->mMOTD, pGuild->mMOTDSetter);
	}
}

void GuildManager::_sendMOTD(Character* pCharacter) {
	ARG_PTR_CHECK(pCharacter);
	EXPECTED(pCharacter->hasGuild());

	Guild* guild = pCharacter->getGuild();
	pCharacter->getConnection()->sendGuildMOTDReply(guild->mMOTD, guild->mMOTDSetter);
}

bool GuildManager::isLeader(Character* pCharacter){
	ARG_PTR_CHECK_BOOL(pCharacter);
	EXPECTED_BOOL(pCharacter->hasGuild());

	GuildMember* member = pCharacter->getGuild()->getMember(pCharacter->getName());
	EXPECTED_BOOL(member);

	return member->mRank == GuildRanks::Leader;
}

bool GuildManager::isOfficer(Character* pCharacter){
	ARG_PTR_CHECK_BOOL(pCharacter);
	EXPECTED_BOOL(pCharacter->hasGuild());

	GuildMember* member = pCharacter->getGuild()->getMember(pCharacter->getName());
	EXPECTED_BOOL(member);

	return member->mRank == GuildRanks::Officer;
}

void GuildManager::handleSetURL(Character* pCharacter, const String& pURL) {
	ARG_PTR_CHECK(pCharacter);
	EXPECTED(pCharacter->hasGuild());
	EXPECTED(isLeader(pCharacter));
	EXPECTED(Limits::Guild::urlLength(pURL));

	pCharacter->getGuild()->mURL = pURL;
	_save();
}

void GuildManager::handleSetChannel(Character* pCharacter, const String& pChannel) {
	ARG_PTR_CHECK(pCharacter);
	EXPECTED(pCharacter->hasGuild());
	EXPECTED(isLeader(pCharacter));
	EXPECTED(Limits::Guild::channelLength(pChannel));

	pCharacter->getGuild()->mChannel = pChannel;
	_save();
}
