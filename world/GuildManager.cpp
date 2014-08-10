#include "GuildManager.h"
#include "Utility.h"
#include "Character.h"
#include "ZoneClientConnection.h"
#include "Zone.h"

#include "../common/tinyxml/tinyxml.h"

bool GuildManager::initialise() {
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

		// TODO: Members

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

	return true;
}

Guild* GuildManager::_findGuildByName(const String pGuildName) {
	for (auto i : mGuilds) {
		if (i->mName == pGuildName)
			return i;
	}

	return nullptr;
}

void GuildManager::_save(){ }

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
