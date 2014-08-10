#include "GuildManager.h"
#include "Utility.h"
#include "Character.h"
#include "ZoneClientConnection.h"
#include "Zone.h"
#include "Profile.h"

#include "../common/tinyxml/tinyxml.h"

bool GuildManager::initialise() {
	Profile p("GuildManager::initialise");
	Log::status("[Guild Manager] Initialising.");

	for (auto i = 0; i < MAX_GUILDS; i++) {
		mGuildNames[i][0] = '\0';
	}

	TiXmlDocument document;
	bool loaded = document.LoadFile("./data/guilds.xml");
	if (!loaded) {
		Log::error("[Guild Manager] Failed to load data.");
		return false;
	}

	TiXmlElement* guildElement = document.FirstChildElement("guilds")->FirstChildElement("guild");
	while (guildElement) {
		Guild* guild = new Guild(); 
		mGuilds.push_back(guild);

		Utility::stoulSafe(guild->mID, String(guildElement->Attribute("id")));
		guild->mName = guildElement->Attribute("name");
		guild->mMOTD = guildElement->Attribute("motd");

		// Read members data.
		TiXmlElement* memberElement = guildElement->FirstChildElement("members")->FirstChildElement("member");
		while (memberElement) {
			uint32 memberID = 0;
			uint32 memberRank = 0;
			Utility::stoulSafe(memberID, String(memberElement->Attribute("id")));
			Utility::stoulSafe(memberRank, String(memberElement->Attribute("rank")));
			guild->mMembers.push_back({ memberID, memberRank });
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

bool GuildManager::makeGuild(Character* pCharacter, const String pGuildName) {
	ARG_PTR_CHECK_BOOL(pCharacter);

	// Check: Guild name already exists.
	if (_findGuildByName(pGuildName)) {
		pCharacter->getConnection()->sendSimpleMessage(MessageType::Red, StringID::GUILD_NAME_IN_USE);
		return false;
	}

	Guild* guild = new Guild();
	guild->mID = getNextGuildID();
	guild->mName = pGuildName;
	mGuilds.push_back(guild);

	_storeGuildName(guild->mID, guild->mName);

	// Tell Zone that something has changed.
	pCharacter->getZone()->notifyGuildsChanged();

	guild->mMembers.push_back({pCharacter->getID(), GuildRanks::Leader});
	guild->mOnlineMembers.push_back(pCharacter);

	pCharacter->setGuild(guild);
	pCharacter->setGuildID(guild->mID);
	pCharacter->setGuildRank(GuildRanks::Leader);

	pCharacter->getConnection()->sendAppearance(SA_GuildID, guild->mID);
	pCharacter->getConnection()->sendAppearance(SA_GuildRank, GuildRanks::Leader);
	pCharacter->getConnection()->sendGuildRank();

	_save();
	return true;
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

		TiXmlElement* membersElement = new TiXmlElement("members");
		for (auto j : i->mMembers) {
			TiXmlElement* memberElement = new TiXmlElement("member");
			memberElement->SetAttribute("id", j.mID);
			memberElement->SetAttribute("rank", j.mRank);
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
	// NOTE: The + 1 is due to how UF client stores guild names.
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

void GuildManager::handleCharacterLogIn(Character* pCharacter, uint32 pGuildID) {
	Guild* guild = _findByID(pGuildID);

	// Check: Guild does not exist. It was probably deleted.
	if (!guild) {
		pCharacter->setGuild(nullptr);
		pCharacter->setGuildID(NO_GUILD);
		pCharacter->setGuildRank(GuildRanks::Member);
		return;
	}

	// Check: Character still belongs to this guild.
	bool found = false;
	for (auto i : guild->mMembers) {
		if (i.mID == pCharacter->getID()) {
			pCharacter->setGuild(guild);
			pCharacter->setGuildID(guild->mID); // This should not really be required.
			pCharacter->setGuildRank(i.mRank); // Character rank may have changed since log in (TODO: Determine if this is even possible).

			// TODO: Notify guild that player is online.

			found = true;
			break;
		}
	}

	// Character was not found in Guild members (save de-sync or removed while offline/zoning?)
	if (!found) {

	}
}

Guild* GuildManager::_findByID(const GuildID pID) {
	for (auto i : mGuilds) {
		if (i->mID == pID)
			return i;
	}

	return nullptr;
}

void GuildManager::handleMemberRemove(Character* pCharacter, String pRemoveCharacterName) {
	ARG_PTR_CHECK(pCharacter);
	EXPECTED(pCharacter->hasGuild());

	// NOTE: UF Prevents a Guild leader from removing them self but we still need to check it.
	// "You must transfer leadership or delete the guild before removing yourself."

	// Self remove.
	if (pCharacter->getName() == pRemoveCharacterName) {
		Guild* guild = pCharacter->getGuild();

		// TODO: Remove from members.

		pCharacter->setGuild(nullptr);
		pCharacter->setGuildID(NO_GUILD);
		pCharacter->setGuildRank(GuildRanks::Member);

		pCharacter->getConnection()->sendAppearance(SA_GuildID, NO_GUILD);
		pCharacter->getConnection()->sendAppearance(SA_GuildRank, GuildRanks::Member);
	}

	// OP_GuildManageRemove
}
