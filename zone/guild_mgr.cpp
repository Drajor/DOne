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
#include "../common/debug.h"
#include "guild_mgr.h"
#include "zonedb.h"
#include "worldserver.h"
#include "../common/servertalk.h"
#include "../common/StringUtil.h"
#include "client.h"
#include "entity.h"

/*

CREATE TABLE guilds (
id MEDIUMINT UNSIGNED NOT NULL,
name VARCHAR(32) NOT NULL,
leader int NOT NULL,
minstatus SMALLINT NOT NULL,
tribute INT UNSIGNED NOT NULL,
motd TEXT NOT NULL DEFAULT '',
PRIMARY KEY(id),
UNIQUE KEY(name),
UNIQUE KEY(leader)
);

CREATE TABLE guild_ranks (
guild_id MEDIUMINT UNSIGNED NOT NULL,
rank TINYINT UNSIGNED NOT NULL,
title VARCHAR(128) NOT NULL,
can_hear TINYINT UNSIGNED NOT NULL,
can_speak TINYINT UNSIGNED NOT NULL,
can_invite TINYINT UNSIGNED NOT NULL,
can_remove TINYINT UNSIGNED NOT NULL,
can_promote TINYINT UNSIGNED NOT NULL,
can_demote TINYINT UNSIGNED NOT NULL,
can_motd TINYINT UNSIGNED NOT NULL,
can_warpeace TINYINT UNSIGNED NOT NULL,
PRIMARY KEY(guild_id,rank)
);

# guild1 < guild2 by definition.
CREATE TABLE guild_relations (
guild1 MEDIUMINT UNSIGNED NOT NULL,
guild2 MEDIUMINT UNSIGNED NOT NULL,
relation TINYINT NOT NULL,
PRIMARY KEY(guild1, guild1)
);

CREATE TABLE guild_members (
char_id INT NOT NULL,
guild_id MEDIUMINT UNSIGNED NOT NULL,
rank TINYINT UNSIGNED NOT NULL,
tribute_enable TINYINT UNSIGNED NOT NULL DEFAULT 0,
total_tribute INT UNSIGNED NOT NULL DEFAULT 0,
last_tribute INT UNSIGNED NOT NULL DEFAULT 0,
banker TINYINT UNSIGNED NOT NULL DEFAULT 0,
public_note TEXT NOT NULL DEFAULT '',
PRIMARY KEY(char_id)
);


*/


ZoneGuildManager guild_mgr;
GuildBankManager *GuildBanks;

extern WorldServer worldserver;
extern volatile bool ZoneLoaded;

void ZoneGuildManager::sendGuildRefresh(uint32 pGuildID, bool pName, bool pMOTD, bool pRank, bool pRelation) {
	_log(GUILDS__REFRESH, "Sending guild refresh for %d to world, changes: name=%d, motd=%d, rank=d, relation=%d", pGuildID, pName, pMOTD, pRank, pRelation);
	ServerPacket* pack = new ServerPacket(ServerOP_RefreshGuild, sizeof(ServerGuildRefresh_Struct));
	ServerGuildRefresh_Struct *s = (ServerGuildRefresh_Struct *)pack->pBuffer;
	s->guild_id = pGuildID;
	s->name_change = pName;
	s->motd_change = pMOTD;
	s->rank_change = pRank;
	s->relation_change = pRelation;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void ZoneGuildManager::sendCharRefresh(uint32 pOldGuildID, uint32 pGuildID, uint32 pCharacterID) {
	if (pGuildID == 0) {
		_log(GUILDS__REFRESH, "Guild lookup for char %d when sending char refresh.", pCharacterID);

		CharacterGuildInfo gci;
		if (!getCharInfo(pCharacterID, gci)) {
			pGuildID = GUILD_NONE;
		}
		else {
			pGuildID = gci.mGuildID;
		}
	}

	_log(GUILDS__REFRESH, "Sending char refresh for %d from guild %d to world", pCharacterID, pGuildID);

	ServerPacket* pack = new ServerPacket(ServerOP_GuildCharRefresh, sizeof(ServerGuildCharRefresh_Struct));
	ServerGuildCharRefresh_Struct *s = (ServerGuildCharRefresh_Struct *)pack->pBuffer;
	s->guild_id = pGuildID;
	s->old_guild_id = pOldGuildID;
	s->char_id = pCharacterID;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void ZoneGuildManager::sendRankUpdate(uint32 pCharacterID)
{
	CharacterGuildInfo gci;

	if (!getCharInfo(pCharacterID, gci))
		return;

	ServerPacket* pack = new ServerPacket(ServerOP_GuildRankUpdate, sizeof(ServerGuildRankUpdate_Struct));

	ServerGuildRankUpdate_Struct *sgrus = (ServerGuildRankUpdate_Struct*)pack->pBuffer;

	sgrus->GuildID = gci.mGuildID;
	strn0cpy(sgrus->MemberName, gci.mCharacterName.c_str(), sizeof(sgrus->MemberName));
	sgrus->Rank = gci.mRank;
	sgrus->Banker = gci.mBanker + (gci.mAlt * 2);

	worldserver.SendPacket(pack);

	safe_delete(pack);
}

void ZoneGuildManager::sendGuildDelete(uint32 pGuildID) {
	_log(GUILDS__REFRESH, "Sending guild delete for guild %d to world", pGuildID);
	ServerPacket* pack = new ServerPacket(ServerOP_DeleteGuild, sizeof(ServerGuildID_Struct));
	ServerGuildID_Struct *s = (ServerGuildID_Struct *)pack->pBuffer;
	s->guild_id = pGuildID;
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

//makes a guild member list packet (internal format), returns ownership of the buffer.
uint8 *ZoneGuildManager::makeGuildMembers(uint32 pGuildID, const char* pPrefixName, uint32& pLength) {
	uint8 *retbuffer;

	//hack because we dont have the "remove from guild" packet right now.
	if (pGuildID == GUILD_NONE) {
		pLength = sizeof(Internal_GuildMembers_Struct);
		retbuffer = new uint8[pLength];
		Internal_GuildMembers_Struct *gms = (Internal_GuildMembers_Struct *)retbuffer;
		strcpy(gms->player_name, pPrefixName);
		gms->count = 0;
		gms->name_length = 0;
		gms->note_length = 0;
		return(retbuffer);
	}

	std::vector<CharacterGuildInfo *> members;
	if (!getEntireGuild(pGuildID, members))
		return(nullptr);

	//figure out the actual packet length.
	uint32 fixed_length = sizeof(Internal_GuildMembers_Struct)+members.size()*sizeof(Internal_GuildMemberEntry_Struct);
	std::vector<CharacterGuildInfo *>::iterator cur, end;
	CharacterGuildInfo *ci;
	cur = members.begin();
	end = members.end();
	uint32 name_len = 0;
	uint32 note_len = 0;
	for (; cur != end; ++cur) {
		ci = *cur;
		name_len += ci->mCharacterName.length();
		note_len += ci->mPublicNote.length();
	}

	//calc total length.
	pLength = fixed_length + name_len + note_len + members.size() * 2;	//string data + null terminators

	//make our nice buffer
	retbuffer = new uint8[pLength];

	Internal_GuildMembers_Struct *gms = (Internal_GuildMembers_Struct *)retbuffer;

	//fill in the global header
	strcpy(gms->player_name, pPrefixName);
	gms->count = members.size();
	gms->name_length = name_len;
	gms->note_length = note_len;

	char *name_buf = (char *)(retbuffer + fixed_length);
	char *note_buf = (char *)(name_buf + name_len + members.size());

	//fill in each member's entry.
	Internal_GuildMemberEntry_Struct *e = gms->member;

	cur = members.begin();
	end = members.end();
	for (; cur != end; ++cur) {
		ci = *cur;

		//the order we set things here must match the struct

		//nice helper macro
#define SlideStructString(field, str) \
	strcpy(field, str.c_str()); \
	field += str.length() + 1

		SlideStructString(name_buf, ci->mCharacterName);
		e->level = ci->mLevel;
		e->banker = ci->mBanker + (ci->mAlt * 2);	// low bit is banker flag, next bit is 'alt' flag.
		e->class_ = ci->mClass_;
		e->rank = ci->mRank;
		e->time_last_on = ci->mTimeLastOn;
		e->tribute_enable = ci->mTributeEnable;
		e->total_tribute = ci->mTotalTribute;
		e->last_tribute = ci->mLastTribute;
		SlideStructString(note_buf, ci->mPublicNote);
		e->zoneinstance = 0;
		e->zone_id = 0;	// Flag them as offline (zoneid 0) as world will update us with their online status afterwards.
#undef SlideStructString
#undef PutFieldN

		delete *cur;

		e++;
	}

	return(retbuffer);
}

void ZoneGuildManager::listGuilds(Client* pClient) const {
	pClient->Message(0, "Listing guilds on the server:");
	char leadername[64];
	std::map<uint32, GuildInfo *>::const_iterator cur, end;
	cur = mGuilds.begin();
	end = mGuilds.end();
	int r = 0;
	for (; cur != end; ++cur) {
		leadername[0] = '\0';
		database.GetCharName(cur->second->mLeaderCharacterID, leadername);
		if (leadername[0] == '\0')
			pClient->Message(0, "  Guild #%i <%s>", cur->first, cur->second->mName.c_str());
		else
			pClient->Message(0, "  Guild #%i <%s> Leader: %s", cur->first, cur->second->mName.c_str(), leadername);
		r++;
	}
	pClient->Message(0, "%i guilds listed.", r);
}


void ZoneGuildManager::describeGuild(Client* pClient, uint32 pGuildID) const {
	std::map<uint32, GuildInfo *>::const_iterator res;
	res = mGuilds.find(pGuildID);
	if (res == mGuilds.end()) {
		pClient->Message(0, "Guild %d not found.", pGuildID);
		return;
	}

	const GuildInfo *info = res->second;

	pClient->Message(0, "Guild info DB# %i <%s>", pGuildID, info->mName.c_str());

	char leadername[64];
	database.GetCharName(info->mLeaderCharacterID, leadername);
	pClient->Message(0, "Guild Leader: %s", leadername);

	char permbuffer[256];
	uint8 i;
	for (i = 0; i <= GUILD_MAX_RANK; i++) {
		char *permptr = permbuffer;
		uint8 r;
		for (r = 0; r < _MaxGuildAction; r++)
			permptr += sprintf(permptr, "  %s: %c", GuildActionNames[r], info->mRanks[i].mPermissions[r] ? 'Y' : 'N');

		pClient->Message(0, "Rank %i: %s", i, info->mRanks[i].mName.c_str());
		pClient->Message(0, "Permissions: %s", permbuffer);
	}

}

//in theory, we could get a pile of unused entries in this array, but only if
//we had a malicious client sending controlled packets, plus its like 10 bytes per entry.
void ZoneGuildManager::recordInvite(uint32 pCharacterID, uint32 pGuildID, uint8 pRank) {
	mInviteQueue[pCharacterID] = std::pair<uint32, uint8>(pGuildID, pRank);
}

bool ZoneGuildManager::verifyAndClearInvite(uint32 pCharacterID, uint32 pGuildID, uint8 pRank) {
	std::map<uint32, std::pair<uint32, uint8> >::iterator res;
	res = mInviteQueue.find(pCharacterID);
	if (res == mInviteQueue.end())
		return(false);	//no entry...
	bool valid = false;
	if (res->second.first == pGuildID && res->second.second == pRank) {
		valid = true;
	}
	mInviteQueue.erase(res);
	return(valid);
}

void ZoneGuildManager::processWorldPacket(ServerPacket *pPacket) {
	switch (pPacket->opcode) {
	case ServerOP_RefreshGuild: {
									if (pPacket->size != sizeof(ServerGuildRefresh_Struct)) {
										_log(GUILDS__ERROR, "Received ServerOP_RefreshGuild of incorrect size %d, expected %d", pPacket->size, sizeof(ServerGuildRefresh_Struct));
										return;
									}
									ServerGuildRefresh_Struct *s = (ServerGuildRefresh_Struct *)pPacket->pBuffer;

									_log(GUILDS__REFRESH, "Received guild refresh from world for %d, changes: name=%d, motd=%d, rank=%d, relation=%d", s->guild_id, s->name_change, s->motd_change, s->rank_change, s->relation_change);

									//reload all the guild details from the database.
									refreshGuild(s->guild_id);

									if (s->motd_change) {
										//resend guild MOTD to all guild members in this zone.
										entity_list.SendGuildMOTD(s->guild_id);
									}

									if (s->name_change) {
										//until we figure out the guild update packet, we resend the whole guild list.
										entity_list.SendGuildList();
									}

									if (s->rank_change) {
										//we need to send spawn appearance packets for all members of this guild in the zone, to everybody.
										entity_list.SendGuildSpawnAppearance(s->guild_id);
									}

									if (s->relation_change) {
										//unknown until we implement guild relations.
									}

									break;
	}

	case ServerOP_GuildCharRefresh: {
										if (pPacket->size != sizeof(ServerGuildCharRefresh_Struct)) {
											_log(GUILDS__ERROR, "Received ServerOP_RefreshGuild of incorrect size %d, expected %d", pPacket->size, sizeof(ServerGuildCharRefresh_Struct));
											return;
										}
										ServerGuildCharRefresh_Struct *s = (ServerGuildCharRefresh_Struct *)pPacket->pBuffer;

										_log(GUILDS__REFRESH, "Received guild member refresh from world for char %d from guild %d", s->char_id, s->guild_id);

										Client *c = entity_list.GetClientByCharID(s->char_id);

										if (c != nullptr) {
											//this reloads the char's guild info from the database and sends appearance updates
											c->RefreshGuildInfo();
										}

										//it would be nice if we had the packet to send just a one-person update
										if (s->guild_id == GUILD_NONE) {
											if (c != nullptr)
												c->SendGuildMembers();	//only need to update this player's list (trying to clear it)
										}
										else {
											entity_list.SendGuildMembers(s->guild_id);		//even send GUILD_NONE (empty)
										}

										if (s->old_guild_id != 0 && s->old_guild_id != GUILD_NONE && s->old_guild_id != s->guild_id)
											entity_list.SendGuildMembers(s->old_guild_id);
										else if (c != nullptr && s->guild_id != GUILD_NONE) {
											//char is in zone, and has changed into a new guild, send MOTD.
											c->SendGuildMOTD();
										}


										break;
	}

	case ServerOP_GuildRankUpdate:
	{
									 if (ZoneLoaded)
									 {
										 if (pPacket->size != sizeof(ServerGuildRankUpdate_Struct))
										 {
											 _log(GUILDS__ERROR, "Received ServerOP_RankUpdate of incorrect size %d, expected %d",
												 pPacket->size, sizeof(ServerGuildRankUpdate_Struct));

											 return;
										 }

										 ServerGuildRankUpdate_Struct *sgrus = (ServerGuildRankUpdate_Struct*)pPacket->pBuffer;

										 EQApplicationPacket *outapp = new EQApplicationPacket(OP_SetGuildRank, sizeof(GuildSetRank_Struct));

										 GuildSetRank_Struct *gsrs = (GuildSetRank_Struct*)outapp->pBuffer;

										 gsrs->Rank = sgrus->Rank;
										 strn0cpy(gsrs->MemberName, sgrus->MemberName, sizeof(gsrs->MemberName));
										 gsrs->Banker = sgrus->Banker;

										 entity_list.QueueClientsGuild(nullptr, outapp, false, sgrus->GuildID);

										 safe_delete(outapp);
									 }

									 break;
	}

	case ServerOP_DeleteGuild: {
								   if (pPacket->size != sizeof(ServerGuildID_Struct)) {
									   _log(GUILDS__ERROR, "Received ServerOP_DeleteGuild of incorrect size %d, expected %d", pPacket->size, sizeof(ServerGuildID_Struct));
									   return;
								   }
								   ServerGuildID_Struct *s = (ServerGuildID_Struct *)pPacket->pBuffer;

								   _log(GUILDS__REFRESH, "Received guild delete from world for guild %d", s->guild_id);

								   //clear all the guild tags.
								   entity_list.RefreshAllGuildInfo(s->guild_id);

								   //remove the guild data from the local guild manager
								   guild_mgr.localDeleteGuild(s->guild_id);

								   //if we stop forcing guild list to send on guild create, we need to do this:
								   //in the case that we delete a guild and add a new one.
								   //entity_list.SendGuildList();

								   break;
	}

	case ServerOP_GuildMemberUpdate:
	{
									   ServerGuildMemberUpdate_Struct *sgmus = (ServerGuildMemberUpdate_Struct*)pPacket->pBuffer;

									   if (ZoneLoaded)
									   {
										   EQApplicationPacket *outapp = new EQApplicationPacket(OP_GuildMemberUpdate, sizeof(GuildMemberUpdate_Struct));

										   GuildMemberUpdate_Struct *gmus = (GuildMemberUpdate_Struct*)outapp->pBuffer;

										   gmus->GuildID = sgmus->GuildID;
										   strn0cpy(gmus->MemberName, sgmus->MemberName, sizeof(gmus->MemberName));
										   gmus->ZoneID = sgmus->ZoneID;
										   gmus->InstanceID = 0;	// I don't think we care what Instance they are in, for the Guild Management Window.
										   gmus->LastSeen = sgmus->LastSeen;

										   entity_list.QueueClientsGuild(nullptr, outapp, false, sgmus->GuildID);

										   safe_delete(outapp);
									   }
									   break;
	}
	case ServerOP_OnlineGuildMembersResponse:
		if (ZoneLoaded)
		{
			char *Buffer = (char *)pPacket->pBuffer;

			uint32 FromID = VARSTRUCT_DECODE_TYPE(uint32, Buffer);
			uint32 Count = VARSTRUCT_DECODE_TYPE(uint32, Buffer);
			Client *c = entity_list.GetClientByCharID(FromID);

			if (!c || !c->IsInAGuild())
			{
				_log(GUILDS__ERROR, "Invalid Client or not in guild. ID=%i", FromID);
				break;
			}
			_log(GUILDS__IN_PACKETS, "Processing ServerOP_OnlineGuildMembersResponse");
			EQApplicationPacket *outapp = new EQApplicationPacket(OP_GuildMemberUpdate, sizeof(GuildMemberUpdate_Struct));
			GuildMemberUpdate_Struct *gmus = (GuildMemberUpdate_Struct*)outapp->pBuffer;
			char Name[64];
			gmus->LastSeen = time(nullptr);
			gmus->InstanceID = 0;
			gmus->GuildID = c->GuildID();
			for (int i = 0; i < Count; i++)
			{
				// Just make the packet once and swap out name/zone and send
				VARSTRUCT_DECODE_STRING(Name, Buffer);
				strn0cpy(gmus->MemberName, Name, sizeof(gmus->MemberName));
				gmus->ZoneID = VARSTRUCT_DECODE_TYPE(uint32, Buffer);
				_log(GUILDS__OUT_PACKETS, "Sending OP_GuildMemberUpdate to %i. Name=%s ZoneID=%i", FromID, Name, gmus->ZoneID);
				c->QueuePacket(outapp);
			}
			safe_delete(outapp);

		}
		break;

	case ServerOP_LFGuildUpdate:
	{
								   if (ZoneLoaded)
								   {
									   char GuildName[33];
									   char Comments[257];
									   uint32 FromLevel, ToLevel, Classes, AACount, TimeZone, TimePosted, Toggle;

									   pPacket->ReadString(GuildName);
									   pPacket->ReadString(Comments);
									   FromLevel = pPacket->ReadUInt32();
									   ToLevel = pPacket->ReadUInt32();
									   Classes = pPacket->ReadUInt32();
									   AACount = pPacket->ReadUInt32();
									   TimeZone = pPacket->ReadUInt32();
									   TimePosted = pPacket->ReadUInt32();
									   Toggle = pPacket->ReadUInt32();

									   uint32 GuildID = getGuildIDByName(GuildName);

									   if (GuildID == GUILD_NONE)
										   break;

									   EQApplicationPacket *outapp = new EQApplicationPacket(OP_LFGuild, sizeof(LFGuild_GuildToggle_Struct));

									   LFGuild_GuildToggle_Struct *gts = (LFGuild_GuildToggle_Struct *)outapp->pBuffer;
									   gts->Command = 1;
									   strcpy(gts->Comment, Comments);
									   gts->FromLevel = FromLevel;
									   gts->ToLevel = ToLevel;
									   gts->Classes = Classes;
									   gts->AACount = AACount;
									   gts->TimeZone = TimeZone;
									   gts->Toggle = Toggle;
									   gts->TimePosted = TimePosted;
									   gts->Name[0] = 0;
									   entity_list.QueueClientsGuild(nullptr, outapp, false, GuildID);
									   safe_delete(outapp);
									   break;
								   }
	}
	}
}

void ZoneGuildManager::sendGuildMemberUpdateToWorld(const char* pMemberName, uint32 pGuildID, uint16 pZoneID, uint32 pLastSeen)
{
	ServerPacket* pack = new ServerPacket(ServerOP_GuildMemberUpdate, sizeof(ServerGuildMemberUpdate_Struct));

	ServerGuildMemberUpdate_Struct *sgmus = (ServerGuildMemberUpdate_Struct*)pack->pBuffer;
	sgmus->GuildID = pGuildID;
	strn0cpy(sgmus->MemberName, pMemberName, sizeof(sgmus->MemberName));
	sgmus->ZoneID = pZoneID;
	sgmus->LastSeen = pLastSeen;
	worldserver.SendPacket(pack);

	safe_delete(pack);
}

void ZoneGuildManager::requestOnlineGuildMembers(uint32 pFromID, uint32 pGuildID)
{
	ServerPacket* pack = new ServerPacket(ServerOP_RequestOnlineGuildMembers, sizeof(ServerRequestOnlineGuildMembers_Struct));
	ServerRequestOnlineGuildMembers_Struct *srogm = (ServerRequestOnlineGuildMembers_Struct*)pack->pBuffer;

	srogm->FromID = pFromID;
	srogm->GuildID = pGuildID;
	worldserver.SendPacket(pack);

	safe_delete(pack);
}

void ZoneGuildManager::addGuildApproval(const char* pGuildName, Client* pOwner)
{
	GuildApproval* tmp = new GuildApproval(pGuildName, pOwner, getFreeID());
	mGuildApprovals.Insert(tmp);
}

void ZoneGuildManager::addMemberApproval(uint32 pRefID, Client* pClient)
{
	GuildApproval* tmp = getGuildApproval(pRefID);
	if (tmp != 0)
	{
		if (!tmp->addMemberApproval(pClient))
			pClient->Message(0, "Unable to add to list.");
		else
		{
			pClient->Message(0, "Added to list.");
		}
	}
	else
		pClient->Message(0, "Unable to find guild reference id.");
}

ZoneGuildManager::~ZoneGuildManager()
{
	clearGuilds();
}

GuildApproval* ZoneGuildManager::getGuildApproval(uint32 refid)
{
	LinkedListIterator<GuildApproval*> iterator(mGuildApprovals);

	iterator.Reset();
	while (iterator.MoreElements())
	{
		if (iterator.GetData()->getID() == refid)
			return iterator.GetData();
		iterator.Advance();
	}
	return 0;
}

GuildBankManager::~GuildBankManager()
{
	std::list<GuildBank*>::iterator Iterator = mBanks.begin();

	while (Iterator != mBanks.end())
	{
		safe_delete(*Iterator);

		++Iterator;
	}
}

bool GuildBankManager::load(uint32 pGuildID)
{
	const char *LoadQuery = "SELECT `area`, `slot`, `itemid`, `qty`, `donator`, `permissions`, `whofor` from `guild_bank` "
		"WHERE `guildid` = %i";

	char errbuf[MYSQL_ERRMSG_SIZE];

	char* query = 0;

	MYSQL_RES *result;

	MYSQL_ROW row;

	if (database.RunQuery(query, MakeAnyLenString(&query, LoadQuery, pGuildID), errbuf, &result))
	{
		GuildBank *Bank = new GuildBank;

		Bank->mGuildID = pGuildID;

		for (int i = 0; i < GUILD_BANK_MAIN_AREA_SIZE; ++i)
			Bank->mItems.mMainArea[i].mItemID = 0;

		for (int i = 0; i < GUILD_BANK_DEPOSIT_AREA_SIZE; ++i)
			Bank->mItems.mDepositArea[i].mItemID = 0;

		char Donator[64], WhoFor[64];

		while ((row = mysql_fetch_row(result)))
		{
			int Area = atoi(row[0]);

			int Slot = atoi(row[1]);

			int ItemID = atoi(row[2]);

			int Qty = atoi(row[3]);

			if (row[4])
				strn0cpy(Donator, row[4], sizeof(Donator));
			else
				Donator[0] = '\0';

			int Permissions = atoi(row[5]);

			if (row[6])
				strn0cpy(WhoFor, row[6], sizeof(WhoFor));
			else
				WhoFor[0] = '\0';

			if (Area == GuildBankMainArea)
			{
				if ((Slot >= 0) && (Slot < GUILD_BANK_MAIN_AREA_SIZE))
				{
					Bank->mItems.mMainArea[Slot].mItemID = ItemID;

					Bank->mItems.mMainArea[Slot].mQuantity = Qty;

					strn0cpy(Bank->mItems.mMainArea[Slot].mDonator, Donator, sizeof(Donator));

					Bank->mItems.mMainArea[Slot].mPermissions = Permissions;

					strn0cpy(Bank->mItems.mMainArea[Slot].mWhoFor, WhoFor, sizeof(WhoFor));
				}
			}
			else
			{
				if ((Slot >= 0) && (Slot < GUILD_BANK_DEPOSIT_AREA_SIZE))
				{
					Bank->mItems.mDepositArea[Slot].mItemID = ItemID;

					Bank->mItems.mDepositArea[Slot].mQuantity = Qty;

					strn0cpy(Bank->mItems.mDepositArea[Slot].mDonator, Donator, sizeof(Donator));

					Bank->mItems.mDepositArea[Slot].mPermissions = Permissions;

					strn0cpy(Bank->mItems.mDepositArea[Slot].mWhoFor, WhoFor, sizeof(WhoFor));
				}
			}

		}
		mysql_free_result(result);

		safe_delete_array(query);

		mBanks.push_back(Bank);
	}
	else
	{
		_log(GUILDS__BANK_ERROR, "Error Loading guild bank: %s, %s", query, errbuf);

		safe_delete_array(query);

		return false;
	}

	return true;

}

bool GuildBankManager::isLoaded(uint32 pGuildID)
{
	return getGuildBank(pGuildID) != nullptr;
}

void GuildBankManager::sendGuildBank(Client* pClient)
{
	if (!pClient || !pClient->IsInAGuild())
		return;

	if (!isLoaded(pClient->GuildID()))
		load(pClient->GuildID());

	GuildBank* bank = getGuildBank(pClient->GuildID());
	if (!bank)
	{
		_log(GUILDS__BANK_ERROR, "Unable to find guild bank for guild ID %i", pClient->GuildID());
		return;
	}

	for (int i = 0; i < GUILD_BANK_DEPOSIT_AREA_SIZE; ++i)
	{
		if (bank->mItems.mDepositArea[i].mItemID > 0)
		{
			const Item_Struct *Item = database.GetItem(bank->mItems.mDepositArea[i].mItemID);

			if (!Item)
				continue;

			EQApplicationPacket *outapp = new EQApplicationPacket(OP_GuildBank, sizeof(GuildBankItemUpdate_Struct));

			GuildBankItemUpdate_Struct *gbius = (GuildBankItemUpdate_Struct*)outapp->pBuffer;

			if (!Item->Stackable)
				gbius->Init(GuildBankItemUpdate, 1, i, GuildBankDepositArea, 1, Item->ID, Item->Icon, 1,
				bank->mItems.mDepositArea[i].mPermissions, 0, 0);
			else
			{
				if (bank->mItems.mDepositArea[i].mQuantity == Item->StackSize)
					gbius->Init(GuildBankItemUpdate, 1, i, GuildBankDepositArea, 1, Item->ID, Item->Icon,
					bank->mItems.mDepositArea[i].mQuantity, bank->mItems.mDepositArea[i].mPermissions, 0, 0);
				else
					gbius->Init(GuildBankItemUpdate, 1, i, GuildBankDepositArea, 1, Item->ID, Item->Icon,
					bank->mItems.mDepositArea[i].mQuantity, bank->mItems.mDepositArea[i].mPermissions, 1, 0);
			}

			strn0cpy(gbius->ItemName, Item->Name, sizeof(gbius->ItemName));

			strn0cpy(gbius->Donator, bank->mItems.mDepositArea[i].mDonator, sizeof(gbius->Donator));

			strn0cpy(gbius->WhoFor, bank->mItems.mDepositArea[i].mWhoFor, sizeof(gbius->WhoFor));

			pClient->FastQueuePacket(&outapp);
		}
	}

	for (int i = 0; i < GUILD_BANK_MAIN_AREA_SIZE; ++i)
	{
		if (bank->mItems.mMainArea[i].mItemID > 0)
		{
			const Item_Struct *Item = database.GetItem(bank->mItems.mMainArea[i].mItemID);

			if (!Item)
				continue;

			bool Useable = Item->IsEquipable(pClient->GetBaseRace(), pClient->GetBaseClass());

			EQApplicationPacket *outapp = new EQApplicationPacket(OP_GuildBank, sizeof(GuildBankItemUpdate_Struct));

			GuildBankItemUpdate_Struct *gbius = (GuildBankItemUpdate_Struct*)outapp->pBuffer;

			if (!Item->Stackable)
				gbius->Init(GuildBankItemUpdate, 1, i, GuildBankMainArea, 1, Item->ID, Item->Icon, 1,
				bank->mItems.mMainArea[i].mPermissions, 0, Useable);
			else
			{
				if (bank->mItems.mMainArea[i].mQuantity == Item->StackSize)
					gbius->Init(GuildBankItemUpdate, 1, i, GuildBankMainArea, 1, Item->ID, Item->Icon,
					bank->mItems.mMainArea[i].mQuantity, bank->mItems.mMainArea[i].mPermissions, 0, Useable);
				else
					gbius->Init(GuildBankItemUpdate, 1, i, GuildBankMainArea, 1, Item->ID, Item->Icon,
					bank->mItems.mMainArea[i].mQuantity, bank->mItems.mMainArea[i].mPermissions, 1, Useable);
			}

			strn0cpy(gbius->ItemName, Item->Name, sizeof(gbius->ItemName));

			strn0cpy(gbius->Donator, bank->mItems.mMainArea[i].mDonator, sizeof(gbius->Donator));

			strn0cpy(gbius->WhoFor, bank->mItems.mMainArea[i].mWhoFor, sizeof(gbius->WhoFor));

			pClient->FastQueuePacket(&outapp);
		}
	}
}
bool GuildBankManager::isAreaFull(uint32 pGuildID, uint16 pArea)
{
	GuildBank* bank = getGuildBank(pGuildID);
	if (!bank) return false;

	GuildBankItem* BankArea = nullptr;

	int AreaSize = 0;

	if (pArea == GuildBankMainArea) {
		BankArea = &bank->mItems.mMainArea[0];
		AreaSize = GUILD_BANK_MAIN_AREA_SIZE;
	}
	else {
		BankArea = &bank->mItems.mDepositArea[0];
		AreaSize = GUILD_BANK_DEPOSIT_AREA_SIZE;
	}

	for (int i = 0; i < AreaSize; ++i)
		if (BankArea[i].mItemID == 0)
			return false;

	return true;
}

bool GuildBankManager::addItem(uint32 pGuildID, uint8 pArea, uint32 pItemID, int32 pQtyOrCharges, const char* pDonator, uint8 pPermissions, const char* pWhoFor)
{
	GuildBank* bank = getGuildBank(pGuildID);

	if (!bank) {
		_log(GUILDS__BANK_ERROR, "Unable to find guild bank for guild ID %i", pGuildID);
		return false;
	}

	GuildBankItem* BankArea = nullptr;

	int AreaSize = 0;

	if (pArea == GuildBankMainArea) {
		BankArea = &bank->mItems.mMainArea[0];
		AreaSize = GUILD_BANK_MAIN_AREA_SIZE;
	}
	else {
		BankArea = &bank->mItems.mDepositArea[0];
		AreaSize = GUILD_BANK_DEPOSIT_AREA_SIZE;
	}

	int Slot = -1;

	for (int i = 0; i < AreaSize; ++i) {
		if (BankArea[i].mItemID == 0) {
			BankArea[i].mItemID = pItemID;
			BankArea[i].mQuantity = pQtyOrCharges;
			strn0cpy(BankArea[i].mDonator, pDonator, sizeof(BankArea[i].mDonator));
			BankArea[i].mPermissions = pPermissions;
			strn0cpy(BankArea[i].mWhoFor, pWhoFor, sizeof(BankArea[i].mWhoFor));
			Slot = i;

			break;
		}
	}

	if (Slot < 0) {
		_log(GUILDS__BANK_ERROR, "No space to add item to the guild bank.");
		return false;
	}

	const char *Query = "INSERT INTO `guild_bank` (`guildid`, `area`, `slot`, `itemid`, `qty`, `donator`, `permissions`, `WhoFor`) "
		"VALUES (%i, %i, %i, %i, %i, '%s', %i, '%s')";

	char errbuf[MYSQL_ERRMSG_SIZE];

	char* query = 0;

	if (!database.RunQuery(query, MakeAnyLenString(&query, Query, pGuildID, pArea, Slot, pItemID, pQtyOrCharges, pDonator, pPermissions, pWhoFor), errbuf))
	{
		_log(GUILDS__BANK_ERROR, "Insert Error: %s : %s", query, errbuf);
		safe_delete_array(query);
		return false;
	}

	safe_delete_array(query);

	const Item_Struct *Item = database.GetItem(pItemID);

	GuildBankItemUpdate_Struct gbius;

	if (!Item->Stackable)
		gbius.Init(GuildBankItemUpdate, 1, Slot, pArea, 1, pItemID, Item->Icon, Item->Stackable ? pQtyOrCharges : 1, pPermissions, 0, 0);
	else
	{
		if (pQtyOrCharges == Item->StackSize)
			gbius.Init(GuildBankItemUpdate, 1, Slot, pArea, 1, pItemID, Item->Icon, Item->Stackable ? pQtyOrCharges : 1, pPermissions, 0, 0);
		else
			gbius.Init(GuildBankItemUpdate, 1, Slot, pArea, 1, pItemID, Item->Icon, Item->Stackable ? pQtyOrCharges : 1, pPermissions, 1, 0);
	}

	strn0cpy(gbius.ItemName, Item->Name, sizeof(gbius.ItemName));

	strn0cpy(gbius.Donator, pDonator, sizeof(gbius.Donator));

	strn0cpy(gbius.WhoFor, pWhoFor, sizeof(gbius.WhoFor));

	entity_list.QueueClientsGuildBankItemUpdate(&gbius, pGuildID);

	return true;
}

int GuildBankManager::promote(uint32 pGuildID, int pSlotID)
{
	if ((pSlotID < 0) || (pSlotID >(GUILD_BANK_DEPOSIT_AREA_SIZE - 1)))
		return -1;

	GuildBank* bank = getGuildBank(pGuildID);

	if (!bank) return -1;

	if (bank->mItems.mDepositArea[pSlotID].mItemID == 0) return -1;

	int MainSlot = -1;

	for (int i = 0; i < GUILD_BANK_MAIN_AREA_SIZE; ++i)
	if (bank->mItems.mMainArea[i].mItemID == 0) {
		MainSlot = i;
		break;
	}

	if (MainSlot == -1)
		return -1;


	bank->mItems.mMainArea[MainSlot].mItemID = bank->mItems.mDepositArea[pSlotID].mItemID;

	bank->mItems.mMainArea[MainSlot].mQuantity = bank->mItems.mDepositArea[pSlotID].mQuantity;

	strn0cpy(bank->mItems.mMainArea[MainSlot].mDonator, bank->mItems.mDepositArea[pSlotID].mDonator, sizeof(bank->mItems.mMainArea[MainSlot].mDonator));
	bank->mItems.mMainArea[MainSlot].mPermissions = bank->mItems.mDepositArea[pSlotID].mPermissions;

	strn0cpy(bank->mItems.mMainArea[MainSlot].mWhoFor, bank->mItems.mDepositArea[pSlotID].mWhoFor, sizeof(bank->mItems.mMainArea[MainSlot].mWhoFor));

	const char *Query = "UPDATE `guild_bank` SET `area` = 1, `slot` = %i WHERE `guildid` = %i AND `area` = 0 AND `slot` = %i LIMIT 1";

	char errbuf[MYSQL_ERRMSG_SIZE];

	char* query = 0;

	if (!database.RunQuery(query, MakeAnyLenString(&query, Query, MainSlot, pGuildID, pSlotID), errbuf))
	{
		_log(GUILDS__BANK_ERROR, "error promoting item: %s : %s", query, errbuf);

		safe_delete_array(query);

		return -1;
	}

	safe_delete_array(query);

	bank->mItems.mDepositArea[pSlotID].mItemID = 0;

	const Item_Struct *Item = database.GetItem(bank->mItems.mMainArea[MainSlot].mItemID);

	GuildBankItemUpdate_Struct gbius;

	if (!Item->Stackable)
		gbius.Init(GuildBankItemUpdate, 1, MainSlot, GuildBankMainArea, 1, Item->ID, Item->Icon, 1, 0, 0, 0);
	else
	{
		if (bank->mItems.mMainArea[MainSlot].mQuantity == Item->StackSize)
			gbius.Init(GuildBankItemUpdate, 1, MainSlot, GuildBankMainArea, 1, Item->ID, Item->Icon, bank->mItems.mMainArea[MainSlot].mQuantity, 0, 0, 0);
		else
			gbius.Init(GuildBankItemUpdate, 1, MainSlot, GuildBankMainArea, 1, Item->ID, Item->Icon, bank->mItems.mMainArea[MainSlot].mQuantity, 0, 1, 0);
	}

	strn0cpy(gbius.ItemName, Item->Name, sizeof(gbius.ItemName));

	entity_list.QueueClientsGuildBankItemUpdate(&gbius, pGuildID);

	gbius.Init(GuildBankItemUpdate, 1, pSlotID, GuildBankDepositArea, 0, 0, 0, 0, 0, 0, 0);

	entity_list.QueueClientsGuildBankItemUpdate(&gbius, pGuildID);

	return MainSlot;
}

void GuildBankManager::setPermissions(uint32 pGuildID, uint16 pSlotID, uint32 pPermissions, const char* pMemberName)
{
	if ((pSlotID > (GUILD_BANK_MAIN_AREA_SIZE - 1)))
		return;

	GuildBank* bank = getGuildBank(pGuildID);

	if (!bank) return;

	if (bank->mItems.mMainArea[pSlotID].mItemID == 0) return;

	const char *Query = "UPDATE `guild_bank` SET `permissions` = %i, `whofor` = '%s' WHERE `guildid` = %i AND `area` = 1 AND `slot` = %i LIMIT 1";

	char errbuf[MYSQL_ERRMSG_SIZE];

	char* query = 0;

	if (!database.RunQuery(query, MakeAnyLenString(&query, Query, pPermissions, pMemberName, pGuildID, pSlotID), errbuf))
	{
		_log(GUILDS__BANK_ERROR, "error changing permissions: %s : %s", query, errbuf);

		safe_delete_array(query);

		return;
	}

	safe_delete_array(query);

	bank->mItems.mMainArea[pSlotID].mPermissions = pPermissions;

	if (pPermissions == GuildBankSingleMember)
		strn0cpy(bank->mItems.mMainArea[pSlotID].mWhoFor, pMemberName, sizeof(bank->mItems.mMainArea[pSlotID].mWhoFor));
	else
		bank->mItems.mMainArea[pSlotID].mWhoFor[0] = '\0';


	const Item_Struct *Item = database.GetItem(bank->mItems.mMainArea[pSlotID].mItemID);

	GuildBankItemUpdate_Struct gbius;

	if (!Item->Stackable)
		gbius.Init(GuildBankItemUpdate, 1, pSlotID, GuildBankMainArea, 1, Item->ID, Item->Icon, 1, bank->mItems.mMainArea[pSlotID].mPermissions, 0, 0);
	else
	{
		if (bank->mItems.mMainArea[pSlotID].mQuantity == Item->StackSize)
			gbius.Init(GuildBankItemUpdate, 1, pSlotID, GuildBankMainArea, 1, Item->ID, Item->Icon, bank->mItems.mMainArea[pSlotID].mQuantity, bank->mItems.mMainArea[pSlotID].mPermissions, 0, 0);
		else
			gbius.Init(GuildBankItemUpdate, 1, pSlotID, GuildBankMainArea, 1, Item->ID, Item->Icon, bank->mItems.mMainArea[pSlotID].mQuantity, bank->mItems.mMainArea[pSlotID].mPermissions, 1, 0);
	}


	strn0cpy(gbius.ItemName, Item->Name, sizeof(gbius.ItemName));

	strn0cpy(gbius.WhoFor, bank->mItems.mMainArea[pSlotID].mWhoFor, sizeof(gbius.WhoFor));

	entity_list.QueueClientsGuildBankItemUpdate(&gbius, pGuildID);
}

ItemInst* GuildBankManager::getItem(uint32 pGuildID, uint16 pArea, uint16 pSlotID, uint32 pQuantity)
{
	GuildBank* bank = getGuildBank(pGuildID);

	if (!bank) return nullptr;

	GuildBankItem* BankArea = nullptr;

	ItemInst* inst = nullptr;

	if (pArea == GuildBankDepositArea) {
		if ((pSlotID > (GUILD_BANK_DEPOSIT_AREA_SIZE - 1)))
			return nullptr;

		inst = database.CreateItem(bank->mItems.mDepositArea[pSlotID].mItemID);

		if (!inst)
			return nullptr;

		BankArea = &bank->mItems.mDepositArea[0];
	}
	else {
		if ((pSlotID > (GUILD_BANK_MAIN_AREA_SIZE - 1)))
			return nullptr;

		inst = database.CreateItem(bank->mItems.mMainArea[pSlotID].mItemID);

		if (!inst)
			return nullptr;

		BankArea = &bank->mItems.mMainArea[0];
	}

	if (!inst->IsStackable())
		inst->SetCharges(BankArea[pSlotID].mQuantity);
	else {
		if (pQuantity <= BankArea[pSlotID].mQuantity)
			inst->SetCharges(pQuantity);
		else
			inst->SetCharges(BankArea[pSlotID].mQuantity);
	}

	return inst;
}

bool GuildBankManager::hasItem(uint32 pGuildID, uint32 pItemID)
{
	GuildBank* bank = getGuildBank(pGuildID);

	if (!bank) return false;

	for (int i = 0; i < GUILD_BANK_MAIN_AREA_SIZE; ++i)
		if (bank->mItems.mMainArea[i].mItemID == pItemID)
			return true;

	for (int i = 0; i < GUILD_BANK_DEPOSIT_AREA_SIZE; ++i)
		if (bank->mItems.mDepositArea[i].mItemID == pItemID)
			return true;

	return false;
}

GuildBank* GuildBankManager::getGuildBank(uint32 pGuildID)
{
	for (auto i = mBanks.begin(); i != mBanks.end(); i++)
		if ((*i)->mGuildID == pGuildID)
			return *i;
	return nullptr;
}

bool GuildBankManager::deleteItem(uint32 pGuildID, uint16 pArea, uint16 pSlotID, uint32 pQuantity)
{
	GuildBank* bank = getGuildBank(pGuildID);

	if (!bank) return false;

	char errbuf[MYSQL_ERRMSG_SIZE];

	char* query = 0;

	GuildBankItem* BankArea = nullptr;

	if (pArea == GuildBankMainArea) {
		if (pSlotID > (GUILD_BANK_MAIN_AREA_SIZE - 1))
			return false;

		BankArea = &bank->mItems.mMainArea[0];
	}
	else
	{
		if (pSlotID > (GUILD_BANK_DEPOSIT_AREA_SIZE - 1))
			return false;

		BankArea = &bank->mItems.mDepositArea[0];
	}


	bool Deleted = true;

	const Item_Struct *Item = database.GetItem(BankArea[pSlotID].mItemID);

	if (!Item->Stackable || (pQuantity >= BankArea[pSlotID].mQuantity))
	{
		const char *Query = "DELETE from `guild_bank` where `guildid` = %i AND `area` = %i AND `slot` = %i LIMIT 1";

		if (!database.RunQuery(query, MakeAnyLenString(&query, Query, pGuildID, pArea, pSlotID), errbuf))
		{
			_log(GUILDS__BANK_ERROR, "Delete item failed. %s : %s", query, errbuf);

			safe_delete_array(query);

			return false;
		}

		safe_delete_array(query);

		BankArea[pSlotID].mItemID = 0;
	}
	else
	{
		const char *Query = "UPDATE `guild_bank` SET `qty` = %i where `guildid` = %i AND `area` = %i AND `slot` = %i LIMIT 1";

		if (!database.RunQuery(query, MakeAnyLenString(&query, Query, BankArea[pSlotID].mQuantity - pQuantity,
			pGuildID, pArea, pSlotID), errbuf))
		{
			_log(GUILDS__BANK_ERROR, "Update item failed. %s : %s", query, errbuf);

			safe_delete_array(query);

			return false;
		}

		safe_delete_array(query);

		BankArea[pSlotID].mQuantity -= pQuantity;

		Deleted = false;
	}
	GuildBankItemUpdate_Struct gbius;

	if (!Deleted)
	{
		gbius.Init(GuildBankItemUpdate, 1, pSlotID, pArea, 1, Item->ID, Item->Icon, BankArea[pSlotID].mQuantity, BankArea[pSlotID].mPermissions, 1, 0);

		strn0cpy(gbius.ItemName, Item->Name, sizeof(gbius.ItemName));

		strn0cpy(gbius.WhoFor, BankArea[pSlotID].mWhoFor, sizeof(gbius.WhoFor));
	}
	else
		gbius.Init(GuildBankItemUpdate, 1, pSlotID, pArea, 0, 0, 0, 0, 0, 0, 0);

	entity_list.QueueClientsGuildBankItemUpdate(&gbius, pGuildID);

	return true;

}

bool GuildBankManager::mergeStacks(uint32 pGuildID, uint16 pSlotID)
{
	if (pSlotID > (GUILD_BANK_MAIN_AREA_SIZE - 1))
		return false;

	GuildBank* bank = getGuildBank(pGuildID);

	if (!bank) return false;

	GuildBankItem* BankArea = &bank->mItems.mMainArea[0];

	if (BankArea[pSlotID].mItemID == 0)
		return false;

	const Item_Struct *Item = database.GetItem(BankArea[pSlotID].mItemID);

	if (!Item->Stackable)
		return false;

	uint32 ItemID = BankArea[pSlotID].mItemID;

	for (int i = 0; i < GUILD_BANK_MAIN_AREA_SIZE - 1; ++i)
	{
		if (BankArea[i].mItemID != ItemID)
			continue;

		if (BankArea[i].mQuantity == Item->StackSize)
			continue;

		bool Merged = false;

		for (int j = i + 1; j < GUILD_BANK_MAIN_AREA_SIZE; ++j)
		{
			if (BankArea[j].mItemID != ItemID)
				continue;

			if (BankArea[j].mPermissions != BankArea[i].mPermissions)
				continue;

			if (BankArea[i].mPermissions == 1)
			if (strncmp(BankArea[i].mWhoFor, BankArea[j].mWhoFor, sizeof(BankArea[i].mWhoFor)))
				continue;

			if ((BankArea[i].mQuantity + BankArea[j].mQuantity) <= Item->StackSize)
			{
				BankArea[i].mQuantity += BankArea[j].mQuantity;

				deleteItem(pGuildID, GuildBankMainArea, j, BankArea[j].mQuantity);

				Merged = true;

				if (BankArea[i].mQuantity == Item->StackSize)
					break;
			}
			else
			{
				uint32 QuantityToMove = Item->StackSize - BankArea[i].mQuantity;

				deleteItem(pGuildID, GuildBankMainArea, j, QuantityToMove);

				BankArea[i].mQuantity = Item->StackSize;

				Merged = true;

				break;
			}
		}

		if (Merged)
		{
			updateItemQuantity(pGuildID, GuildBankMainArea, i, BankArea[i].mQuantity);

			GuildBankItemUpdate_Struct gbius;

			if (BankArea[i].mQuantity == Item->StackSize)
				gbius.Init(GuildBankItemUpdate, 1, i, GuildBankMainArea, 1, ItemID, Item->Icon, BankArea[i].mQuantity, BankArea[i].mPermissions, 0, 0);
			else
				gbius.Init(GuildBankItemUpdate, 1, i, GuildBankMainArea, 1, ItemID, Item->Icon, BankArea[i].mQuantity, BankArea[i].mPermissions, 1, 0);

			strn0cpy(gbius.ItemName, Item->Name, sizeof(gbius.ItemName));

			strn0cpy(gbius.WhoFor, BankArea[i].mWhoFor, sizeof(gbius.WhoFor));

			entity_list.QueueClientsGuildBankItemUpdate(&gbius, pGuildID);
		}

	}

	return true;
}

bool GuildBankManager::splitStack(uint32 pGuildID, uint16 pSlotID, uint32 pQuantity)
{
	if (pSlotID > (GUILD_BANK_MAIN_AREA_SIZE - 1))
		return false;

	GuildBank* bank = getGuildBank(pGuildID);

	if (!bank) return false;

	if (isAreaFull(pGuildID, GuildBankMainArea))
		return false;

	GuildBankItem* BankArea = &bank->mItems.mMainArea[0];

	if (BankArea[pSlotID].mItemID == 0)
		return false;

	if (BankArea[pSlotID].mQuantity <= pQuantity || pQuantity == 0)
		return false;

	const Item_Struct *Item = database.GetItem(BankArea[pSlotID].mItemID);

	if (!Item->Stackable)
		return false;

	addItem(pGuildID, GuildBankMainArea, BankArea[pSlotID].mItemID, pQuantity, "", BankArea[pSlotID].mPermissions, BankArea[pSlotID].mWhoFor);

	deleteItem(pGuildID, GuildBankMainArea, pSlotID, pQuantity);

	return true;
}

void GuildBankManager::updateItemQuantity(uint32 pGuildID, uint16 pArea, uint16 pSlotID, uint32 pQuantity)
{
	// Helper method for MergeStacks. Assuming all passed parameters are valid.
	//
	char errbuf[MYSQL_ERRMSG_SIZE];

	char* query = 0;

	const char *Query = "UPDATE `guild_bank` SET `qty` = %i where `guildid` = %i AND `area` = %i AND `slot` = %i LIMIT 1";

	if (!database.RunQuery(query, MakeAnyLenString(&query, Query, pQuantity, pGuildID, pArea, pSlotID), errbuf))
	{
		_log(GUILDS__BANK_ERROR, "Update item quantity failed. %s : %s", query, errbuf);

		safe_delete_array(query);

		return;
	}

	safe_delete_array(query);
}

bool GuildBankManager::allowedToWithdraw(uint32 pGuildID, uint16 pArea, uint16 pSlotID, const char* pName)
{
	// Is a none-Guild Banker allowed to withdraw the item at this slot ?
	// This is really here for anti-hacking measures, as the client should not request an item it does not have permission to withdraw.
	//
	if (pSlotID > (GUILD_BANK_MAIN_AREA_SIZE - 1))
		return false;

	GuildBank* bank = getGuildBank(pGuildID);

	if (!bank) return false;

	if (pArea != GuildBankMainArea)
		return false;

	uint8 Permissions = bank->mItems.mMainArea[pSlotID].mPermissions;

	if (Permissions == GuildBankBankerOnly)
		return false;

	if (Permissions != GuildBankSingleMember)	// Public or Public-If-Useable (should really check if item is useable)
		return true;

	if (!strncmp(bank->mItems.mMainArea[pSlotID].mWhoFor, pName, sizeof(bank->mItems.mMainArea[pSlotID].mWhoFor)))
		return true;

	return false;
}

/*================== GUILD APPROVAL ========================*/

bool GuildApproval::processApproval()
{
	if (mOwner && mOwner->GuildID() != 0)
	{
		mOwner->Message(10, "You are already in a guild! Guild request deleted.");
		return false;
	}
	if (mDeletionTimer->Check() || !mOwner)
	{
		if (mOwner)
			mOwner->Message(0, "You took too long! Your guild request has been deleted.");
		return false;
	}

	return true;
}

GuildApproval::GuildApproval(const char* pGuildName, Client* pOwner, uint32 pID)
{
	database.GetVariable("GuildCreation", mFounders, 3);
	uint8 tmp = atoi(mFounders);
	mDeletionTimer = new Timer(1800000);
	strcpy(mGuild, pGuildName);
	this->mOwner = pOwner;
	this->mRefID = pID;
	if (pOwner)
		pOwner->Message(0, "You can now start getting your guild approved, tell your %i members to #guildapprove %i, you have 30 minutes to create your guild.", tmp, getID());
	for (int i = 0; i < tmp; i++)
		mMembers[i] = 0;
}

GuildApproval::~GuildApproval()
{
	safe_delete(mDeletionTimer);
}

bool GuildApproval::addMemberApproval(Client* pAddition)
{
	database.GetVariable("GuildCreation", mFounders, 3);
	uint8 tmp = atoi(mFounders);
	for (int i = 0; i < tmp; i++)
	{
		if (mMembers[i] && mMembers[i] == pAddition)
			return false;
	}

	for (int i = 0; i < tmp; i++)
	{
		if (!mMembers[i])
		{
			mMembers[i] = pAddition;
			int z = 0;
			for (int i = 0; i < tmp; i++)
			{
				if (mMembers[i])
					z++;
			}
			if (z == tmp)
				guildApproved();

			return true;
		}
	}
	return false;
}

void GuildApproval::approvedMembers(Client* pRequestee)
{
	database.GetVariable("GuildCreation", mFounders, 3);
	uint8 tmp = atoi(mFounders);
	for (int i = 0; i < tmp; i++)
	{
		if (mMembers[i])
			pRequestee->Message(0, "%i: %s", i, mMembers[i]->GetName());
	}
}

void GuildApproval::guildApproved()
{
	char petitext[PBUFFER] = "A new guild was founded! Guildname: ";
	char gmembers[MBUFFER] = " ";

	if (!mOwner)
		return;
	database.GetVariable("GuildCreation", mFounders, 3);
	uint8 tmp = atoi(mFounders);
	uint32 tmpeq = guild_mgr.createGuild(mGuild, mOwner->CharacterID());
	guild_mgr.setGuild(mOwner->CharacterID(), tmpeq, 2);
	mOwner->SendAppearancePacket(AT_GuildID, true, false);
	for (int i = 0; i < tmp; i++)
	{
		if (mMembers[i])
		{
			mOwner->Message(0, "%s", mMembers[i]->GetName());
			mOwner->Message(0, "%i", mMembers[i]->CharacterID());
			guild_mgr.setGuild(mMembers[i]->CharacterID(), tmpeq, 0);
			size_t len = MBUFFER - strlen(gmembers) + 1;
			strncat(gmembers, " ", len);
			strncat(gmembers, mMembers[i]->GetName(), len);
		}
	}
	size_t len = PBUFFER - strlen(petitext) + 1;
	strncat(petitext, mGuild, len);
	strncat(petitext, " Leader: ", len);
	strncat(petitext, mOwner->CastToClient()->GetName(), len);
	strncat(petitext, " Members:", len);
	strncat(petitext, gmembers, len);
	Petition* pet = new Petition(mOwner->CastToClient()->CharacterID());
	pet->SetAName(mOwner->CastToClient()->AccountName());
	pet->SetClass(mOwner->CastToClient()->GetClass());
	pet->SetLevel(mOwner->CastToClient()->GetLevel());
	pet->SetCName(mOwner->CastToClient()->GetName());
	pet->SetRace(mOwner->CastToClient()->GetRace());
	pet->SetLastGM("");
	pet->SetCName(mOwner->CastToClient()->GetName()); //aza77 is this really 2 times needed ??
	pet->SetPetitionText(petitext);
	pet->SetZone(zone->GetZoneID());
	pet->SetUrgency(0);
	petition_list.AddPetition(pet);
	database.InsertPetitionToDB(pet);
	petition_list.UpdateGMQueue();
	petition_list.UpdateZoneListQueue();
	worldserver.SendEmoteMessage(0, 0, 80, 15, "%s has made a petition. #%i", mOwner->CastToClient()->GetName(), pet->GetID());
	ServerPacket* pack = new ServerPacket;
	pack->opcode = ServerOP_RefreshGuild;
	pack->size = tmp;
	pack->pBuffer = new uchar[pack->size];
	memcpy(pack->pBuffer, &tmpeq, 4);
	worldserver.SendPacket(pack);
	safe_delete(pack);
	mOwner->Message(0, "Your guild was created.");
	mOwner = 0;
}

