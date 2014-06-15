/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2006 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
	*/

#include "debug.h"
#include "guild_base.h"
#include "database.h"
#include "logsys.h"
//#include "MiscFunctions.h"
#include "StringUtil.h"
#include <cstdlib>
#include <cstring>

//until we move MAX_NUMBER_GUILDS
#include "eq_packet_structs.h"

const char *const BaseGuildManager::GuildActionNames[_MaxGuildAction] =
{ "HearGuildChat", "SpeakGuildChat", "Invite", "Remove", "Promote", "Demote", "Set_MOTD", "War/Peace" };

BaseGuildManager::BaseGuildManager()
: mDatabase(nullptr)
{
}

BaseGuildManager::~BaseGuildManager() {
	clearGuilds();
}

bool BaseGuildManager::loadGuilds() {

	clearGuilds();

	if (mDatabase == nullptr) {
		_log(GUILDS__DB, "Requested to load guilds when we have no database object.");
		return false;
	}

	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	std::map<uint32, GuildInfo *>::iterator res;

	// load up all the guilds
	if (!mDatabase->RunQuery(query, MakeAnyLenString(&query,
		"SELECT id, name, leader, minstatus, motd, motd_setter,channel,url FROM guilds"), errbuf, &result)) {
		_log(GUILDS__ERROR, "Error loading guilds '%s': %s", query, errbuf);
		safe_delete_array(query);
		return false;
	}
	safe_delete_array(query);
	while ((row = mysql_fetch_row(result))) {
		_createGuild(atoi(row[0]), row[1], atoi(row[2]), atoi(row[3]), row[4], row[5], row[6], row[7]);
	}
	mysql_free_result(result);

	//load up the rank info for each guild.
	if (!mDatabase->RunQuery(query, MakeAnyLenString(&query,
		"SELECT pGuildID,rank,title,can_hear,can_speak,can_invite,can_remove,can_promote,can_demote,can_motd,can_warpeace FROM guild_ranks"), errbuf, &result)) {
		_log(GUILDS__ERROR, "Error loading guild ranks '%s': %s", query, errbuf);
		safe_delete_array(query);
		return false;
	}
	safe_delete_array(query);
	while ((row = mysql_fetch_row(result))) {
		uint32 pGuildID = atoi(row[0]);
		uint8 rankn = atoi(row[1]);
		if (rankn > GUILD_MAX_RANK) {
			_log(GUILDS__ERROR, "Found invalid (too high) rank %d for guild %d, skipping.", rankn, pGuildID);
			continue;
		}

		res = mGuilds.find(pGuildID);
		if (res == mGuilds.end()) {
			_log(GUILDS__ERROR, "Found rank %d for non-existent guild %d, skipping.", rankn, pGuildID);
			continue;
		}

		RankInfo &rank = res->second->mRanks[rankn];

		rank.mName = row[2];
		rank.mPermissions[GUILD_HEAR] = (row[3][0] == '1') ? true : false;
		rank.mPermissions[GUILD_SPEAK] = (row[4][0] == '1') ? true : false;
		rank.mPermissions[GUILD_INVITE] = (row[5][0] == '1') ? true : false;
		rank.mPermissions[GUILD_REMOVE] = (row[6][0] == '1') ? true : false;
		rank.mPermissions[GUILD_PROMOTE] = (row[7][0] == '1') ? true : false;
		rank.mPermissions[GUILD_DEMOTE] = (row[8][0] == '1') ? true : false;
		rank.mPermissions[GUILD_MOTD] = (row[9][0] == '1') ? true : false;
		rank.mPermissions[GUILD_WARPEACE] = (row[10][0] == '1') ? true : false;
	}
	mysql_free_result(result);

	return true;
}

bool BaseGuildManager::refreshGuild(uint32 pGuildID) {
	if (mDatabase == nullptr) {
		_log(GUILDS__DB, "Requested to refresh guild %d when we have no database object.", pGuildID);
		return false;
	}

	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	std::map<uint32, GuildInfo *>::iterator res;
	GuildInfo *info;

	// load up all the guilds
	if (!mDatabase->RunQuery(query, MakeAnyLenString(&query,
		"SELECT name, leader, minstatus, motd, motd_setter, channel,url FROM guilds WHERE id=%lu", (unsigned long)pGuildID), errbuf, &result)) {
		_log(GUILDS__ERROR, "Error reloading guilds '%s': %s", query, errbuf);
		safe_delete_array(query);
		return false;
	}
	safe_delete_array(query);
	if ((row = mysql_fetch_row(result))) {
		//delete the old entry and create the new one.
		info = _createGuild(pGuildID, row[0], atoi(row[1]), atoi(row[2]), row[3], row[4], row[5], row[6]);
	}
	else {
		_log(GUILDS__ERROR, "Unable to find guild %d in the database.", pGuildID);
		return false;
	}
	mysql_free_result(result);

	//load up the rank info for each guild.
	if (!mDatabase->RunQuery(query, MakeAnyLenString(&query,
		"SELECT pGuildID,rank,title,can_hear,can_speak,can_invite,can_remove,can_promote,can_demote,can_motd,can_warpeace "
		"FROM guild_ranks WHERE pGuildID=%lu", (unsigned long)pGuildID), errbuf, &result)) {
		_log(GUILDS__ERROR, "Error reloading guild ranks '%s': %s", query, errbuf);
		safe_delete_array(query);
		return false;
	}
	safe_delete_array(query);

	while ((row = mysql_fetch_row(result))) {
		uint8 rankn = atoi(row[1]);
		if (rankn > GUILD_MAX_RANK) {
			_log(GUILDS__ERROR, "Found invalid (too high) rank %d for guild %d, skipping.", rankn, pGuildID);
			continue;
		}
		RankInfo &rank = info->mRanks[rankn];

		rank.mName = row[2];
		rank.mPermissions[GUILD_HEAR] = (row[3][0] == '1') ? true : false;
		rank.mPermissions[GUILD_SPEAK] = (row[4][0] == '1') ? true : false;
		rank.mPermissions[GUILD_INVITE] = (row[5][0] == '1') ? true : false;
		rank.mPermissions[GUILD_REMOVE] = (row[6][0] == '1') ? true : false;
		rank.mPermissions[GUILD_PROMOTE] = (row[7][0] == '1') ? true : false;
		rank.mPermissions[GUILD_DEMOTE] = (row[8][0] == '1') ? true : false;
		rank.mPermissions[GUILD_MOTD] = (row[9][0] == '1') ? true : false;
		rank.mPermissions[GUILD_WARPEACE] = (row[10][0] == '1') ? true : false;
	}
	mysql_free_result(result);

	_log(GUILDS__DB, "Successfully refreshed guild %d from the database.", pGuildID);

	return true;
}

BaseGuildManager::GuildInfo *BaseGuildManager::_createGuild(uint32 pGuildID, const char* pGuildName, uint32 pLeaderCharacterID, uint8 pMinStatus, const char* pGuildMOTD, const char* pMOTDSetter, const char* pChannel, const char* pURL)
{
	std::map<uint32, GuildInfo *>::iterator res;

	//remove any old entry.
	res = mGuilds.find(pGuildID);
	if (res != mGuilds.end()) {
		delete res->second;
		mGuilds.erase(res);
	}

	//make the new entry and store it into the map.
	GuildInfo *info = new GuildInfo;
	info->mName = pGuildName;
	info->mLeaderCharacterID = pLeaderCharacterID;
	info->mMinStatus = pMinStatus;
	info->mMOTD = pGuildMOTD;
	info->mMOTDSetter = pMOTDSetter;
	info->mURL = pURL;
	info->mChannel = pChannel;
	mGuilds[pGuildID] = info;

	//now setup default ranks (everything defaults to false)
	info->mRanks[0].mName = "Member";
	info->mRanks[0].mPermissions[GUILD_HEAR] = true;
	info->mRanks[0].mPermissions[GUILD_SPEAK] = true;
	info->mRanks[1].mName = "Officer";
	info->mRanks[1].mPermissions[GUILD_HEAR] = true;
	info->mRanks[1].mPermissions[GUILD_SPEAK] = true;
	info->mRanks[1].mPermissions[GUILD_INVITE] = true;
	info->mRanks[1].mPermissions[GUILD_REMOVE] = true;
	info->mRanks[1].mPermissions[GUILD_MOTD] = true;
	info->mRanks[2].mName = "Leader";
	info->mRanks[2].mPermissions[GUILD_HEAR] = true;
	info->mRanks[2].mPermissions[GUILD_SPEAK] = true;
	info->mRanks[2].mPermissions[GUILD_INVITE] = true;
	info->mRanks[2].mPermissions[GUILD_REMOVE] = true;
	info->mRanks[2].mPermissions[GUILD_PROMOTE] = true;
	info->mRanks[2].mPermissions[GUILD_DEMOTE] = true;
	info->mRanks[2].mPermissions[GUILD_MOTD] = true;
	info->mRanks[2].mPermissions[GUILD_WARPEACE] = true;

	return(info);
}

bool BaseGuildManager::_storeGuildDB(uint32 pGuildID) {
	if (mDatabase == nullptr) {
		_log(GUILDS__DB, "Requested to store guild %d when we have no database object.", pGuildID);
		return false;
	}

	std::map<uint32, GuildInfo *>::const_iterator res;
	res = mGuilds.find(pGuildID);
	if (res == mGuilds.end()) {
		_log(GUILDS__DB, "Requested to store non-existent guild %d", pGuildID);
		return false;
	}
	GuildInfo *info = res->second;

	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	//clear out old `guilds` entry
	if (!mDatabase->RunQuery(query, MakeAnyLenString(&query,
		"DELETE FROM guilds WHERE id=%lu", (unsigned long)pGuildID), errbuf))
	{
		_log(GUILDS__ERROR, "Error clearing old guild record when storing %d '%s': %s", pGuildID, query, errbuf);
	}
	safe_delete_array(query);

	//clear out old `guild_ranks` entries
	if (!mDatabase->RunQuery(query, MakeAnyLenString(&query,
		"DELETE FROM guild_ranks WHERE pGuildID=%lu", (unsigned long)pGuildID), errbuf))
	{
		_log(GUILDS__ERROR, "Error clearing old guild_ranks records when storing %d '%s': %s", pGuildID, query, errbuf);
	}
	safe_delete_array(query);

	//escape our strings.
	char *name_esc = new char[info->mName.length() * 2 + 1];
	char *motd_esc = new char[info->mMOTD.length() * 2 + 1];
	char *motd_set_esc = new char[info->mMOTDSetter.length() * 2 + 1];
	mDatabase->DoEscapeString(name_esc, info->mName.c_str(), info->mName.length());
	mDatabase->DoEscapeString(motd_esc, info->mMOTD.c_str(), info->mMOTD.length());
	mDatabase->DoEscapeString(motd_set_esc, info->mMOTDSetter.c_str(), info->mMOTDSetter.length());

	//insert the new `guilds` entry
	if (!mDatabase->RunQuery(query, MakeAnyLenString(&query,
		"INSERT INTO guilds (id,name,leader,minstatus,motd,motd_setter) VALUES(%lu,'%s',%lu,%d,'%s', '%s')",
		(unsigned long)pGuildID, name_esc, (unsigned long)info->mLeaderCharacterID, info->mMinStatus, motd_esc, motd_set_esc), errbuf))
	{
		_log(GUILDS__ERROR, "Error inserting new guild record when storing %d. Giving up. '%s': %s", pGuildID, query, errbuf);
		safe_delete_array(query);
		safe_delete_array(name_esc);
		safe_delete_array(motd_esc);
		safe_delete_array(motd_set_esc);
		return false;
	}
	safe_delete_array(query);
	safe_delete_array(name_esc);
	safe_delete_array(motd_esc);
	safe_delete_array(motd_set_esc);

	//now insert the new ranks
	uint8 rank;
	for (rank = 0; rank <= GUILD_MAX_RANK; rank++) {
		const RankInfo &r = info->mRanks[rank];

		char *title_esc = new char[r.mName.length() * 2 + 1];
		mDatabase->DoEscapeString(title_esc, r.mName.c_str(), r.mName.length());

		if (!mDatabase->RunQuery(query, MakeAnyLenString(&query,
			"INSERT INTO guild_ranks (pGuildID,rank,title,can_hear,can_speak,can_invite,can_remove,can_promote,can_demote,can_motd,can_warpeace)"
			" VALUES(%d,%d,'%s',%d,%d,%d,%d,%d,%d,%d,%d)",
			pGuildID, rank, title_esc,
			r.mPermissions[GUILD_HEAR],
			r.mPermissions[GUILD_SPEAK],
			r.mPermissions[GUILD_INVITE],
			r.mPermissions[GUILD_REMOVE],
			r.mPermissions[GUILD_PROMOTE],
			r.mPermissions[GUILD_DEMOTE],
			r.mPermissions[GUILD_MOTD],
			r.mPermissions[GUILD_WARPEACE]), errbuf))
		{
			_log(GUILDS__ERROR, "Error inserting new guild rank record when storing %d for %d. Giving up. '%s': %s", rank, pGuildID, query, errbuf);
			safe_delete_array(query);
			safe_delete_array(title_esc);
			return false;
		}
		safe_delete_array(query);
		safe_delete_array(title_esc);
	}

	_log(GUILDS__DB, "Stored guild %d in the database", pGuildID);

	return true;
}

uint32 BaseGuildManager::_getFreeGuildID() {
	if (mDatabase == nullptr) {
		_log(GUILDS__DB, "Requested find a free guild ID when we have no database object.");
		return(GUILD_NONE);
	}

	char errbuf[MYSQL_ERRMSG_SIZE];
	char query[100];
	MYSQL_RES *result;

	//this has got to be one of the more retarded things I have seen.
	//none the less, im too lazy to rewrite it right now.

	uint16 x;
	for (x = 1; x < MAX_NUMBER_GUILDS; x++) {
		snprintf(query, 100, "SELECT id FROM guilds where id=%i;", x);

		if (mDatabase->RunQuery(query, strlen(query), errbuf, &result)) {
			if (mysql_num_rows(result) == 0) {
				mysql_free_result(result);
				_log(GUILDS__DB, "Located free guild ID %d in the database", x);
				return x;
			}
			mysql_free_result(result);
		}
		else {
			LogFile->write(EQEMuLog::Error, "Error in _GetFreeGuildID query '%s': %s", query, errbuf);
		}
	}

	_log(GUILDS__ERROR, "Unable to find a free guild ID when requested.");
	return GUILD_NONE;
}



uint32 BaseGuildManager::createGuild(const char* pName, uint32 pLeaderCharacterID) {
	uint32 gid = DBCreateGuild(pName, pLeaderCharacterID);
	if (gid == GUILD_NONE)
		return(GUILD_NONE);

	sendGuildRefresh(gid, true, false, false, false);
	sendCharRefresh(GUILD_NONE, gid, pLeaderCharacterID);

	return(gid);
}

bool BaseGuildManager::deleteGuild(uint32 pGuildID) {
	if (!DBDeleteGuild(pGuildID))
		return false;

	sendGuildDelete(pGuildID);

	return true;
}

bool BaseGuildManager::renameGuild(uint32 pGuildID, const char* pName) {
	if (!DBRenameGuild(pGuildID, pName))
		return false;

	sendGuildRefresh(pGuildID, true, false, false, false);

	return true;
}

bool BaseGuildManager::setGuildLeader(uint32 pGuildID, uint32 pLeaderCharacterID) {
	//get old leader first.
	std::map<uint32, GuildInfo *>::const_iterator res;
	res = mGuilds.find(pGuildID);
	if (res == mGuilds.end())
		return false;
	GuildInfo *info = res->second;
	uint32 old_leader = info->mLeaderCharacterID;

	if (!DBSetGuildLeader(pGuildID, pLeaderCharacterID))
		return false;

	sendGuildRefresh(pGuildID, false, false, false, false);
	sendCharRefresh(GUILD_NONE, pGuildID, old_leader);
	sendCharRefresh(GUILD_NONE, pGuildID, pLeaderCharacterID);

	return true;
}

bool BaseGuildManager::setGuildMOTD(uint32 pGuildID, const char* pMOTD, const char* pSetter) {
	if (!DBSetGuildMOTD(pGuildID, pMOTD, pSetter))
		return false;

	sendGuildRefresh(pGuildID, false, true, false, false);

	return true;
}

bool BaseGuildManager::setGuildURL(uint32 pGuildID, const char* pURL)
{
	if (!DBSetGuildURL(pGuildID, pURL))
		return false;

	sendGuildRefresh(pGuildID, false, true, false, false);

	return true;
}

bool BaseGuildManager::setGuildChannel(uint32 pGuildID, const char* pChannel)
{
	if (!DBSetGuildChannel(pGuildID, pChannel))
		return false;

	sendGuildRefresh(pGuildID, false, true, false, false);

	return true;
}

bool BaseGuildManager::setGuild(uint32 pCharacterID, uint32 pGuildID, uint8 pRank) {
	if (pRank > GUILD_MAX_RANK && pGuildID != GUILD_NONE)
		return false;

	//lookup their old guild, if they had one.
	uint32 old_guild = GUILD_NONE;
	CharacterGuildInfo gci;
	if (getCharInfo(pCharacterID, gci)) {
		old_guild = gci.mGuildID;
	}

	if (!DBSetGuild(pCharacterID, pGuildID, pRank))
		return false;

	sendCharRefresh(old_guild, pGuildID, pCharacterID);

	return true;
}

//changes rank, but not guild.
bool BaseGuildManager::setGuildRank(uint32 pCharacterID, uint8 pRank) {
	if (pRank > GUILD_MAX_RANK)
		return false;

	if (!DBSetGuildRank(pCharacterID, pRank))
		return false;

	sendCharRefresh(GUILD_NONE, 0, pCharacterID);

	return true;
}


bool BaseGuildManager::setBankerFlag(uint32 pCharacterID, bool pIsBanker) {
	if (!DBSetBankerFlag(pCharacterID, pIsBanker))
		return false;

	sendRankUpdate(pCharacterID);

	return true;
}

bool BaseGuildManager::setAltFlag(uint32 pCharacterID, bool pIsAlt)
{
	if (!DBSetAltFlag(pCharacterID, pIsAlt))
		return false;

	sendRankUpdate(pCharacterID);

	return true;
}

bool BaseGuildManager::setTributeFlag(uint32 pCharacterID, bool pEnabled) {
	if (!DBSetTributeFlag(pCharacterID, pEnabled))
		return false;

	sendCharRefresh(GUILD_NONE, 0, pCharacterID);

	return true;
}

bool BaseGuildManager::setPublicNote(uint32 pCharacterID, const char* pNote) {
	if (!DBSetPublicNote(pCharacterID, pNote))
		return false;

	sendCharRefresh(GUILD_NONE, 0, pCharacterID);

	return true;
}

uint32 BaseGuildManager::DBCreateGuild(const char* pName, uint32 pLeaderCharacterID) {
	//first try to find a free ID.
	uint32 new_id = _getFreeGuildID();
	if (new_id == GUILD_NONE)
		return(GUILD_NONE);

	//now make the guild record in our local manager.
	//this also sets up the default ranks for us.
	_createGuild(new_id, pName, pLeaderCharacterID, 0, "", "", "", "");

	//now store the resulting guild setup into the DB.
	if (!_storeGuildDB(new_id)) {
		_log(GUILDS__ERROR, "Error storing new guild. It may have been partially created which may need manual removal.");
		return(GUILD_NONE);
	}

	_log(GUILDS__DB, "Created guild %d in the database.", new_id);

	return(new_id);
}

bool BaseGuildManager::DBDeleteGuild(uint32 pGuildID) {

	//remove the local entry
	std::map<uint32, GuildInfo *>::iterator res;
	res = mGuilds.find(pGuildID);
	if (res != mGuilds.end()) {
		delete res->second;
		mGuilds.erase(res);
	}

	if (mDatabase == nullptr) {
		_log(GUILDS__DB, "Requested to delete guild %d when we have no database object.", pGuildID);
		return false;
	}

	char *query = 0;

	//clear out old `guilds` entry
	_RunQuery(query, MakeAnyLenString(&query,
		"DELETE FROM guilds WHERE id=%lu", (unsigned long)pGuildID), "clearing old guild record");

	//clear out old `guild_ranks` entries
	_RunQuery(query, MakeAnyLenString(&query,
		"DELETE FROM guild_ranks WHERE guild_id=%lu", (unsigned long)pGuildID), "clearing old guild_ranks records");

	//clear out people belonging to this guild.
	_RunQuery(query, MakeAnyLenString(&query,
		"DELETE FROM guild_members WHERE guild_id=%lu", (unsigned long)pGuildID), "clearing chars in guild");

	// Delete the guild bank
	_RunQuery(query, MakeAnyLenString(&query,
		"DELETE FROM guild_bank WHERE guildid=%lu", (unsigned long)pGuildID), "deleting guild bank");

	_log(GUILDS__DB, "Deleted guild %d from the database.", pGuildID);

	return true;
}

bool BaseGuildManager::DBRenameGuild(uint32 pGuildID, const char* pName) {
	if (mDatabase == nullptr) {
		_log(GUILDS__DB, "Requested to rename guild %d when we have no database object.", pGuildID);
		return false;
	}

	std::map<uint32, GuildInfo *>::const_iterator res;
	res = mGuilds.find(pGuildID);
	if (res == mGuilds.end())
		return false;
	GuildInfo *info = res->second;

	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	//escape our strings.
	uint32 len = strlen(pName);
	char *esc = new char[len * 2 + 1];
	mDatabase->DoEscapeString(esc, pName, len);

	//insert the new `guilds` entry
	if (!mDatabase->RunQuery(query, MakeAnyLenString(&query,
		"UPDATE guilds SET name='%s' WHERE id=%d",
		esc, pGuildID), errbuf))
	{
		_log(GUILDS__ERROR, "Error renaming guild %d '%s': %s", pGuildID, query, errbuf);
		safe_delete_array(query);
		safe_delete_array(esc);
		return false;
	}
	safe_delete_array(query);
	safe_delete_array(esc);

	_log(GUILDS__DB, "Renamed guild %s (%d) to %s in database.", info->mName.c_str(), pGuildID, pName);

	info->mName = pName;	//update our local record.

	return true;
}

bool BaseGuildManager::DBSetGuildLeader(uint32 pGuildID, uint32 pLeaderCharacterID) {
	if (mDatabase == nullptr) {
		_log(GUILDS__DB, "Requested to set the leader for guild %d when we have no database object.", pGuildID);
		return false;
	}

	std::map<uint32, GuildInfo *>::const_iterator res;
	res = mGuilds.find(pGuildID);
	if (res == mGuilds.end())
		return false;
	GuildInfo *info = res->second;

	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	//insert the new `guilds` entry
	if (!mDatabase->RunQuery(query, MakeAnyLenString(&query,
		"UPDATE guilds SET leader='%d' WHERE id=%d",
		pLeaderCharacterID, pGuildID), errbuf))
	{
		_log(GUILDS__ERROR, "Error changing leader on guild %d '%s': %s", pGuildID, query, errbuf);
		safe_delete_array(query);
		return false;
	}
	safe_delete_array(query);

	//set the old leader to officer
	if (!DBSetGuildRank(info->mLeaderCharacterID, GUILD_OFFICER))
		return false;
	//set the new leader to leader
	if (!DBSetGuildRank(pLeaderCharacterID, GUILD_LEADER))
		return false;

	_log(GUILDS__DB, "Set guild leader for guild %d to %d in the database", pGuildID, pLeaderCharacterID);

	info->mLeaderCharacterID = pLeaderCharacterID;	//update our local record.

	return true;
}

bool BaseGuildManager::DBSetGuildMOTD(uint32 pGuildID, const char* pMOTD, const char* pSetter) {
	if (mDatabase == nullptr) {
		_log(GUILDS__DB, "Requested to set the MOTD for guild %d when we have no database object.", pGuildID);
		return false;
	}

	std::map<uint32, GuildInfo *>::const_iterator res;
	res = mGuilds.find(pGuildID);
	if (res == mGuilds.end())
		return false;
	GuildInfo *info = res->second;

	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	//escape our strings.
	uint32 len = strlen(pMOTD);
	uint32 len2 = strlen(pSetter);
	char *esc = new char[len * 2 + 1];
	char *esc_set = new char[len2 * 2 + 1];
	mDatabase->DoEscapeString(esc, pMOTD, len);
	mDatabase->DoEscapeString(esc_set, pSetter, len2);

	//insert the new `guilds` entry
	if (!mDatabase->RunQuery(query, MakeAnyLenString(&query,
		"UPDATE guilds SET motd='%s',motd_setter='%s' WHERE id=%d",
		esc, esc_set, pGuildID), errbuf))
	{
		_log(GUILDS__ERROR, "Error setting MOTD for guild %d '%s': %s", pGuildID, query, errbuf);
		safe_delete_array(query);
		safe_delete_array(esc);
		safe_delete_array(esc_set);
		return false;
	}
	safe_delete_array(query);
	safe_delete_array(esc);
	safe_delete_array(esc_set);

	_log(GUILDS__DB, "Set MOTD for guild %d in the database", pGuildID);

	info->mMOTD = pMOTD;	//update our local record.
	info->mMOTDSetter = pSetter;	//update our local record.

	return true;
}

bool BaseGuildManager::DBSetGuildURL(uint32 pGuildID, const char* pURL)
{
	if (mDatabase == nullptr)
		return false;

	std::map<uint32, GuildInfo *>::const_iterator res;

	res = mGuilds.find(pGuildID);

	if (res == mGuilds.end())
		return false;

	GuildInfo *info = res->second;

	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	//escape our strings.
	uint32 len = strlen(pURL);

	char *esc = new char[len * 2 + 1];

	mDatabase->DoEscapeString(esc, pURL, len);

	if (!mDatabase->RunQuery(query, MakeAnyLenString(&query, "UPDATE guilds SET url='%s' WHERE id=%d", esc, pGuildID), errbuf))
	{
		_log(GUILDS__ERROR, "Error setting URL for guild %d '%s': %s", pGuildID, query, errbuf);
		safe_delete_array(query);
		safe_delete_array(esc);
		return false;
	}
	safe_delete_array(query);
	safe_delete_array(esc);

	_log(GUILDS__DB, "Set URL for guild %d in the database", pGuildID);

	info->mURL = pURL;	//update our local record.

	return true;
}

bool BaseGuildManager::DBSetGuildChannel(uint32 pGuildID, const char* pChannel)
{
	if (mDatabase == nullptr)
		return false;

	std::map<uint32, GuildInfo *>::const_iterator res;

	res = mGuilds.find(pGuildID);

	if (res == mGuilds.end())
		return false;

	GuildInfo *info = res->second;

	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	//escape our strings.
	uint32 len = strlen(pChannel);

	char *esc = new char[len * 2 + 1];

	mDatabase->DoEscapeString(esc, pChannel, len);

	if (!mDatabase->RunQuery(query, MakeAnyLenString(&query, "UPDATE guilds SET channel='%s' WHERE id=%d", esc, pGuildID), errbuf))
	{
		_log(GUILDS__ERROR, "Error setting Channel for guild %d '%s': %s", pGuildID, query, errbuf);
		safe_delete_array(query);
		safe_delete_array(esc);
		return false;
	}
	safe_delete_array(query);
	safe_delete_array(esc);

	_log(GUILDS__DB, "Set Channel for guild %d in the database", pGuildID);

	info->mChannel = pChannel;	//update our local record.

	return true;
}

bool BaseGuildManager::DBSetGuild(uint32 pCharacterID, uint32 pGuildID, uint8 pRank) {
	if (mDatabase == nullptr) {
		_log(GUILDS__DB, "Requested to set char to guild %d when we have no database object.", pGuildID);
		return false;
	}

	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	if (pGuildID != GUILD_NONE) {
		if (!mDatabase->RunQuery(query, MakeAnyLenString(&query,
			"REPLACE INTO guild_members (char_id,guild_id,rank) VALUES(%d,%d,%d)",
			pCharacterID, pGuildID, pRank), errbuf))
		{
			_log(GUILDS__ERROR, "Error Changing char %d to guild %d '%s': %s", pCharacterID, pGuildID, query, errbuf);
			safe_delete_array(query);
			return false;
		}
	}
	else {
		if (!mDatabase->RunQuery(query, MakeAnyLenString(&query,
			"DELETE FROM guild_members WHERE char_id=%d",
			pCharacterID), errbuf))
		{
			_log(GUILDS__ERROR, "Error removing char %d from guild '%s': %s", pCharacterID, pGuildID, query, errbuf);
			safe_delete_array(query);
			return false;
		}
	}
	safe_delete_array(query);

	_log(GUILDS__DB, "Set char %d to guild %d and rank %d in the database.", pCharacterID, pGuildID, pRank);

	return true;
}

bool BaseGuildManager::DBSetGuildRank(uint32 pCharacterID, uint8 pRank) {
	char *query = 0;
	return(_RunQuery(query, MakeAnyLenString(&query,
		"UPDATE guild_members SET rank=%d WHERE char_id=%d",
		pRank, pCharacterID), "setting a guild member's rank"));
}

bool BaseGuildManager::DBSetBankerFlag(uint32 pCharacterID, bool pIsBanker) {
	char *query = 0;
	return(_RunQuery(query, MakeAnyLenString(&query,
		"UPDATE guild_members SET banker=%d WHERE char_id=%d",
		pIsBanker ? 1 : 0, pCharacterID), "setting a guild member's banker flag"));
}

bool BaseGuildManager::getBankerFlag(uint32 pCharacterID)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if (!mDatabase)
		return false;

	if (!mDatabase->RunQuery(query, MakeAnyLenString(&query, "select `banker` from `guild_members` where char_id=%i LIMIT 1", pCharacterID), errbuf, &result))
	{
		_log(GUILDS__ERROR, "Error retrieving banker flag '%s': %s", query, errbuf);

		safe_delete_array(query);

		return false;
	}

	safe_delete_array(query);

	if (mysql_num_rows(result) != 1)
		return false;

	row = mysql_fetch_row(result);

	bool IsBanker = atoi(row[0]);

	mysql_free_result(result);

	return IsBanker;
}

bool BaseGuildManager::DBSetAltFlag(uint32 pCharacterID, bool pIsAlt)
{
	char *query = 0;

	return(_RunQuery(query, MakeAnyLenString(&query,
		"UPDATE guild_members SET alt=%d WHERE char_id=%d",
		pIsAlt ? 1 : 0, pCharacterID), "setting a guild member's alt flag"));
}

bool BaseGuildManager::getAltFlag(uint32 pCharacterID)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if (!mDatabase)
		return false;

	if (!mDatabase->RunQuery(query, MakeAnyLenString(&query, "select `alt` from `guild_members` where char_id=%i LIMIT 1", pCharacterID), errbuf, &result))
	{
		_log(GUILDS__ERROR, "Error retrieving alt flag '%s': %s", query, errbuf);

		safe_delete_array(query);

		return false;
	}

	safe_delete_array(query);

	if (mysql_num_rows(result) != 1)
		return false;

	row = mysql_fetch_row(result);

	bool IsAlt = atoi(row[0]);

	mysql_free_result(result);

	return IsAlt;
}

bool BaseGuildManager::DBSetTributeFlag(uint32 pCharacterID, bool pEnabled) {
	char *query = 0;
	return(_RunQuery(query, MakeAnyLenString(&query,
		"UPDATE guild_members SET tribute_enable=%d WHERE char_id=%d",
		pEnabled ? 1 : 0, pCharacterID), "setting a guild member's tribute flag"));
}

bool BaseGuildManager::DBSetPublicNote(uint32 pCharacterID, const char* pNote) {
	if (mDatabase == nullptr)
		return false;

	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;

	//escape our strings.
	uint32 len = strlen(pNote);
	char *esc = new char[len * 2 + 1];
	mDatabase->DoEscapeString(esc, pNote, len);

	//insert the new `guilds` entry
	if (!mDatabase->RunQuery(query, MakeAnyLenString(&query,
		"UPDATE guild_members SET public_note='%s' WHERE char_id=%d",
		esc, pCharacterID), errbuf))
	{
		_log(GUILDS__ERROR, "Error setting public note for char %d '%s': %s", pCharacterID, query, errbuf);
		safe_delete_array(query);
		safe_delete_array(esc);
		return false;
	}
	safe_delete_array(query);
	safe_delete_array(esc);

	_log(GUILDS__DB, "Set public not for char %d", pCharacterID);

	return true;
}

bool BaseGuildManager::_RunQuery(char*& pQuery, int pLength, const char* pErrorMessage) {
	if (mDatabase == nullptr)
		return false;

	char errbuf[MYSQL_ERRMSG_SIZE];

	if (!mDatabase->RunQuery(pQuery, pLength, errbuf))
	{
		_log(GUILDS__ERROR, "Error %s: '%s': %s", pErrorMessage, pQuery, errbuf);
		safe_delete_array(pQuery);
		return false;
	}
	safe_delete_array(pQuery);

	return true;
}

#define GuildMemberBaseQuery \
	"SELECT c.id,c.name,c.class,c.level,c.timelaston,c.zoneid," \
	" g.guild_id,g.rank,g.tribute_enable,g.total_tribute,g.last_tribute," \
	" g.banker,g.public_note,g.alt " \
	" FROM character_ AS c LEFT JOIN guild_members AS g ON c.id=g.char_id "

static void ProcessGuildMember(MYSQL_ROW &row, CharacterGuildInfo &into) {
	//fields from `characer_`
	into.mCharacterID = atoi(row[0]);
	into.mCharacterName = row[1];
	into.mClass_ = atoi(row[2]);
	into.mLevel = atoi(row[3]);
	into.mTimeLastOn = atoul(row[4]);
	into.mZoneID = atoi(row[5]);

	//fields from `guild_members`, leave at defaults if missing
	into.mGuildID = row[6] ? atoi(row[6]) : GUILD_NONE;
	into.mRank = row[7] ? atoi(row[7]) : (GUILD_MAX_RANK + 1);
	into.mTributeEnable = row[8] ? (row[8][0] == '0' ? false : true) : false;
	into.mTotalTribute = row[9] ? atoi(row[9]) : 0;
	into.mLastTribute = row[10] ? atoul(row[10]) : 0;		//timestamp
	into.mBanker = row[11] ? (row[11][0] == '0' ? false : true) : false;
	into.mPublicNote = row[12] ? row[12] : "";
	into.mAlt = row[13] ? (row[13][0] == '0' ? false : true) : false;

	//a little sanity checking/cleanup
	if (into.mGuildID == 0)
		into.mGuildID = GUILD_NONE;
	if (into.mRank > GUILD_MAX_RANK)
		into.mRank = GUILD_RANK_NONE;
}


bool BaseGuildManager::getEntireGuild(uint32 pGuildID, std::vector<CharacterGuildInfo*>& pMembers) {
	pMembers.clear();

	if (mDatabase == nullptr)
		return false;

	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	//load up the rank info for each guild.
	if (!mDatabase->RunQuery(query, MakeAnyLenString(&query,
		GuildMemberBaseQuery " WHERE g.pGuildID=%d", pGuildID
		), errbuf, &result)) {
		_log(GUILDS__ERROR, "Error loading guild member list '%s': %s", query, errbuf);
		safe_delete_array(query);
		return false;
	}
	safe_delete_array(query);

	while ((row = mysql_fetch_row(result))) {
		CharacterGuildInfo *ci = new CharacterGuildInfo;
		ProcessGuildMember(row, *ci);
		pMembers.push_back(ci);
	}
	mysql_free_result(result);

	_log(GUILDS__DB, "Retreived entire guild member list for guild %d from the database", pGuildID);

	return true;
}

bool BaseGuildManager::getCharInfo(const char* pCharacterName, CharacterGuildInfo& pInto) {
	if (mDatabase == nullptr) {
		_log(GUILDS__DB, "Requested char info on %s when we have no database object.", pCharacterName);
		return false;
	}

	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	//escape our strings.
	uint32 nl = strlen(pCharacterName);
	char *esc = new char[nl * 2 + 1];
	mDatabase->DoEscapeString(esc, pCharacterName, nl);

	//load up the rank info for each guild.
	if (!mDatabase->RunQuery(query, MakeAnyLenString(&query,
		GuildMemberBaseQuery " WHERE c.name='%s'", esc
		), errbuf, &result)) {
		_log(GUILDS__ERROR, "Error loading guild member '%s': %s", query, errbuf);
		safe_delete_array(query);
		safe_delete_array(esc);
		return false;
	}
	safe_delete_array(query);
	safe_delete_array(esc);

	bool ret = true;
	if ((row = mysql_fetch_row(result))) {
		ProcessGuildMember(row, pInto);
		_log(GUILDS__DB, "Retreived guild member info for char %s from the database", pCharacterName);
	}
	else {
		ret = true;
	}
	mysql_free_result(result);

	return(ret);


}

bool BaseGuildManager::getCharInfo(uint32 pCharacterID, CharacterGuildInfo& pInto) {
	if (mDatabase == nullptr) {
		_log(GUILDS__DB, "Requested char info on %d when we have no database object.", pCharacterID);
		return false;
	}

	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	//load up the rank info for each guild.
	if (!mDatabase->RunQuery(query, MakeAnyLenString(&query, GuildMemberBaseQuery " WHERE c.id=%d", pCharacterID), errbuf, &result)) {
		_log(GUILDS__ERROR, "Error loading guild member '%s': %s", query, errbuf);
		safe_delete_array(query);
		return false;
	}
	safe_delete_array(query);

	bool ret = true;
	if ((row = mysql_fetch_row(result))) {
		ProcessGuildMember(row, pInto);
		_log(GUILDS__DB, "Retreived guild member info for char %d", pCharacterID);
	}
	else {
		ret = true;
	}
	mysql_free_result(result);

	return(ret);

}

//returns ownership of the buffer.
uint8 *BaseGuildManager::makeGuildList(const char* pHeadName, uint32& pLength) const {
	//dynamic structs will make this a lot less painful.

	pLength = sizeof(GuildsList_Struct);
	uint8 *buffer = new uint8[pLength];

	//a bit little better than memsetting the whole thing...
	uint32 r, pos;
	for (r = 0, pos = 0; r <= MAX_NUMBER_GUILDS; r++, pos += 64) {
		//strcpy((char *) buffer+pos, "BAD GUILD");
		// These 'BAD GUILD' entries were showing in the drop-downs for selecting guilds in the LFP window,
		// so just fill unused entries with an empty string instead.
		buffer[pos] = '\0';
	}

	strn0cpy((char *)buffer, pHeadName, 64);

	std::map<uint32, GuildInfo *>::const_iterator cur, end;
	cur = mGuilds.begin();
	end = mGuilds.end();
	for (; cur != end; ++cur) {
		pos = 64 + (64 * cur->first);
		strn0cpy((char *)buffer + pos, cur->second->mName.c_str(), 64);
	}
	return(buffer);
}

const char* BaseGuildManager::getRankName(uint32 pGuildID, uint8 pRank) const {
	if (pRank > GUILD_MAX_RANK)
		return("Invalid Rank");
	std::map<uint32, GuildInfo *>::const_iterator res;
	res = mGuilds.find(pGuildID);
	if (res == mGuilds.end())
		return("Invalid Guild Rank");
	return(res->second->mRanks[pRank].mName.c_str());
}

const char* BaseGuildManager::getGuildName(uint32 pGuildID) const {
	if (pGuildID == GUILD_NONE)
		return("");
	std::map<uint32, GuildInfo *>::const_iterator res;
	res = mGuilds.find(pGuildID);
	if (res == mGuilds.end())
		return("Invalid Guild");
	return(res->second->mName.c_str());
}

bool BaseGuildManager::getGuildNameByID(uint32 pGuildID, std::string& pInto) const {
	std::map<uint32, GuildInfo *>::const_iterator res;
	res = mGuilds.find(pGuildID);
	if (res == mGuilds.end())
		return false;
	pInto = res->second->mName;
	return true;
}

uint32 BaseGuildManager::getGuildIDByName(const char* pGuildName)
{
	std::map<uint32, GuildInfo *>::iterator Iterator;

	for (Iterator = mGuilds.begin(); Iterator != mGuilds.end(); ++Iterator)
	{
		if (!strcasecmp((*Iterator).second->mName.c_str(), pGuildName))
			return (*Iterator).first;
	}

	return GUILD_NONE;
}

bool BaseGuildManager::getGuildMOTD(uint32 pGuildID, char* pMOTDBuffer, char* pSetterBuffer) const {
	std::map<uint32, GuildInfo *>::const_iterator res;
	res = mGuilds.find(pGuildID);
	if (res == mGuilds.end())
		return false;
	strn0cpy(pMOTDBuffer, res->second->mMOTD.c_str(), 512);
	strn0cpy(pSetterBuffer, res->second->mMOTDSetter.c_str(), 64);
	return true;
}

bool BaseGuildManager::getGuildURL(uint32 pGuildID, char* pURLBuffer) const
{
	std::map<uint32, GuildInfo *>::const_iterator res;
	res = mGuilds.find(pGuildID);
	if (res == mGuilds.end())
		return false;
	strn0cpy(pURLBuffer, res->second->mURL.c_str(), 512);

	return true;
}

bool BaseGuildManager::getGuildChannel(uint32 pGuildID, char* pChannelBuffer) const
{
	std::map<uint32, GuildInfo *>::const_iterator res;
	res = mGuilds.find(pGuildID);
	if (res == mGuilds.end())
		return false;
	strn0cpy(pChannelBuffer, res->second->mChannel.c_str(), 128);
	return true;
}

bool BaseGuildManager::guildExists(uint32 pGuildID) const {
	if (pGuildID == GUILD_NONE)
		return false;
	return(mGuilds.find(pGuildID) != mGuilds.end());
}

bool BaseGuildManager::isGuildLeader(uint32 pGuildID, uint32 pCharacterID) const {
	if (pGuildID == GUILD_NONE) {
		_log(GUILDS__PERMISSIONS, "Check leader for char %d: not a guild.", pCharacterID);
		return false;
	}
	std::map<uint32, GuildInfo *>::const_iterator res;
	res = mGuilds.find(pGuildID);
	if (res == mGuilds.end()) {
		_log(GUILDS__PERMISSIONS, "Check leader for char %d: invalid guild.", pCharacterID);
		return false;	//invalid guild
	}
	_log(GUILDS__PERMISSIONS, "Check leader for guild %d, char %d: leader id=%d", pGuildID, pCharacterID, res->second->mLeaderCharacterID);
	return(pCharacterID == res->second->mLeaderCharacterID);
}

uint32 BaseGuildManager::findGuildByLeader(uint32 pLeader) const {
	std::map<uint32, GuildInfo *>::const_iterator cur, end;
	cur = mGuilds.begin();
	end = mGuilds.end();
	for (; cur != end; ++cur) {
		if (cur->second->mLeaderCharacterID == pLeader)
			return(cur->first);
	}
	return(GUILD_NONE);
}

//returns the rank to be sent to the client for display purposes, given their eqemu rank.
uint8 BaseGuildManager::getDisplayedRank(uint32 pGuildID, uint8 pRank, uint32 pCharacterID) const {
	std::map<uint32, GuildInfo *>::const_iterator res;
	res = mGuilds.find(pGuildID);
	if (res == mGuilds.end())
		return(3);	//invalid guild rank
	if (res->second->mRanks[pRank].mPermissions[GUILD_WARPEACE] || res->second->mLeaderCharacterID == pCharacterID)
		return(2);	//leader rank
	else if (res->second->mRanks[pRank].mPermissions[GUILD_INVITE] || res->second->mRanks[pRank].mPermissions[GUILD_REMOVE] || res->second->mRanks[pRank].mPermissions[GUILD_MOTD])
		return(1);	//officer rank
	return(0);	//member rank
}

bool BaseGuildManager::checkGMStatus(uint32 pGuildID, uint8 pStatus) const {
	if (pStatus >= 250) {
		_log(GUILDS__PERMISSIONS, "Check permission on guild %d with user status %d > 250, granted.", pGuildID, pStatus);
		return true;	//250+ as allowed anything
	}

	std::map<uint32, GuildInfo *>::const_iterator res;
	res = mGuilds.find(pGuildID);
	if (res == mGuilds.end()) {
		_log(GUILDS__PERMISSIONS, "Check permission on guild %d with user status %d, no such guild, denied.", pGuildID, pStatus);
		return false;	//invalid guild
	}

	bool granted = (res->second->mMinStatus <= pStatus);

	_log(GUILDS__PERMISSIONS, "Check permission on guild %s (%d) with user status %d. Min status %d: %s",
		res->second->mName.c_str(), pGuildID, pStatus, res->second->mMinStatus, granted ? "granted" : "denied");

	return(granted);
}

bool BaseGuildManager::checkPermission(uint32 pGuildID, uint8 pRank, GuildAction pAction) const {
	if (pRank > GUILD_MAX_RANK) {
		_log(GUILDS__PERMISSIONS, "Check permission on guild %d and rank %d for action %s (%d): Invalid rank, denied.",
			pGuildID, pRank, GuildActionNames[pAction], pAction);
		return false;	//invalid rank
	}
	std::map<uint32, GuildInfo *>::const_iterator res;
	res = mGuilds.find(pGuildID);
	if (res == mGuilds.end()) {
		_log(GUILDS__PERMISSIONS, "Check permission on guild %d and rank %d for action %s (%d): Invalid guild, denied.",
			pGuildID, pRank, GuildActionNames[pAction], pAction);
		return false;	//invalid guild
	}

	bool granted = res->second->mRanks[pRank].mPermissions[pAction];

	_log(GUILDS__PERMISSIONS, "Check permission on guild %s (%d) and rank %s (%d) for action %s (%d): %s",
		res->second->mName.c_str(), pGuildID,
		res->second->mRanks[pRank].mName.c_str(), pRank,
		GuildActionNames[pAction], pAction,
		granted ? "granted" : "denied");

	return(granted);
}

bool BaseGuildManager::localDeleteGuild(uint32 pGuildID) {
	std::map<uint32, GuildInfo *>::iterator res;
	res = mGuilds.find(pGuildID);
	if (res == mGuilds.end())
		return false;	//invalid guild
	mGuilds.erase(res);
	return true;
}

void BaseGuildManager::clearGuilds() {
	std::map<uint32, GuildInfo *>::iterator cur, end;
	cur = mGuilds.begin();
	end = mGuilds.end();
	for (; cur != end; ++cur) {
		delete cur->second;
	}
	mGuilds.clear();
}

BaseGuildManager::RankInfo::RankInfo() {
	uint8 r;
	for (r = 0; r < _MaxGuildAction; r++)
		mPermissions[r] = false;
}

BaseGuildManager::GuildInfo::GuildInfo() {
	mLeaderCharacterID = 0;
	mMinStatus = 0;
}

uint32 BaseGuildManager::doesAccountContainAGuildLeader(uint32 pAccountID)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;

	if (!mDatabase->RunQuery(query,
		MakeAnyLenString(&query,
		"select guild_id from guild_members where char_id in (select id from character_ where account_id = %i) and rank = 2",
		pAccountID), errbuf, &result))
	{
		_log(GUILDS__ERROR, "Error executing query '%s': %s", query, errbuf);
		safe_delete_array(query);
		return 0;
	}
	safe_delete_array(query);

	uint32 Rows = mysql_num_rows(result);
	mysql_free_result(result);

	return Rows;
}
