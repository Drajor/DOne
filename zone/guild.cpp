/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2003 EQEMu Development Team (http://eqemulator.net)

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
#include "masterentity.h"
#include "worldserver.h"
#include "net.h"
#include "../common/database.h"
#include "../common/spdat.h"
#include "../common/packet_dump.h"
#include "../common/packet_functions.h"
#include "petitions.h"
#include "../common/serverinfo.h"
#include "../common/ZoneNumbers.h"
#include "../common/guilds.h"
#include "../common/StringUtil.h"
#include "guild_mgr.h"
#include "StringIDs.h"
#include "NpcAI.h"

extern WorldServer worldserver;

void Client::SendGuildMOTD(bool GetGuildMOTDReply) {
	EQApplicationPacket *outapp = new EQApplicationPacket(OP_GuildMOTD, sizeof(GuildMOTD_Struct));

	// When the Client gets an OP_GuildMOTD, it compares the text to the version it has previously stored.
	// If the text in the OP_GuildMOTD packet is the same, it does nothing. If not the same, it displays
	// the new MOTD and then stores the new text.
	//
	// When the Client receives an OP_GetGuildMOTDReply, it displays the text in the packet.
	//
	// So OP_GuildMOTD should be sent on zone entry and when an Officer changes the MOTD, and OP_GetGuildMOTDReply
	// should be sent when the client issues the /getguildmotd command.
	//
	if (GetGuildMOTDReply)
		outapp->SetOpcode(OP_GetGuildMOTDReply);

	GuildMOTD_Struct *motd = (GuildMOTD_Struct *)outapp->pBuffer;
	motd->unknown0 = 0;
	strn0cpy(motd->name, m_pp.name, 64);

	if (IsInAGuild()) {
		if (!guild_mgr.getGuildMOTD(GuildID(), motd->motd, motd->setby_name)) {
			motd->setby_name[0] = '\0';
			strcpy(motd->motd, "ERROR GETTING MOTD!");
		}
	}
	else {
		//we have to send them an empty MOTD anywyas.
		motd->motd[0] = '\0';	//just to be sure
		motd->setby_name[0] = '\0';	//just to be sure

	}

	mlog(GUILDS__OUT_PACKETS, "Sending OP_GuildMOTD of length %d", outapp->size);
	mpkt(GUILDS__OUT_PACKET_TRACE, outapp);

	FastQueuePacket(&outapp);
}

void Client::SendGuildURL()
{
	if (GetClientVersion() < EQClientSoF)
		return;

	if (IsInAGuild())
	{
		EQApplicationPacket *outapp = new EQApplicationPacket(OP_GuildUpdateURLAndChannel, sizeof(GuildUpdateURLAndChannel_Struct));

		GuildUpdateURLAndChannel_Struct *guuacs = (GuildUpdateURLAndChannel_Struct*)outapp->pBuffer;

		if (guild_mgr.getGuildURL(GuildID(), guuacs->Text))
		{
			guuacs->Action = 0;
			FastQueuePacket(&outapp);
		}
		else
			safe_delete(outapp);
	}
}

void Client::SendGuildChannel()
{
	if (GetClientVersion() < EQClientSoF)
		return;

	if (IsInAGuild())
	{
		EQApplicationPacket *outapp = new EQApplicationPacket(OP_GuildUpdateURLAndChannel, sizeof(GuildUpdateURLAndChannel_Struct));

		GuildUpdateURLAndChannel_Struct *guuacs = (GuildUpdateURLAndChannel_Struct*)outapp->pBuffer;

		if (guild_mgr.getGuildChannel(GuildID(), guuacs->Text))
		{
			guuacs->Action = 1;

			FastQueuePacket(&outapp);
		}
		else
			safe_delete(outapp);
	}
}

void Client::SendGuildRanks()
{
	if (GetClientVersion() < EQClientRoF)
		return;

	int permissions = 30 + 1; //Static number of permissions in all EQ clients as of May 2014
	int ranks = 8 + 1; // Static number of RoF+ ranks as of May 2014
	int j = 1;
	int i = 1;
	if (IsInAGuild())
	{
		while (j < ranks)
		{
			while (i < permissions)
			{
				EQApplicationPacket *outapp = new EQApplicationPacket(OP_GuildUpdateURLAndChannel, sizeof(GuildUpdateRanks_Struct));
				GuildUpdateRanks_Struct *guuacs = (GuildUpdateRanks_Struct*)outapp->pBuffer;
				//guuacs->Unknown0008 = this->GuildID();
				strncpy(guuacs->Unknown0012, this->GetCleanName(), 64);
				guuacs->Action = 5;
				guuacs->RankID = j;
				guuacs->GuildID = this->GuildID();
				guuacs->PermissionID = i;
				guuacs->PermissionVal = 1;
				guuacs->Unknown0089[0] = 0x2c;
				guuacs->Unknown0089[1] = 0x01;
				guuacs->Unknown0089[2] = 0x00;
				FastQueuePacket(&outapp);
				i++;
			}
			j++;
			i = 1;
		}
	}
}

void Client::SendGuildSpawnAppearance() {
	if (!IsInAGuild()) {
		// clear guildtag
		SendAppearancePacket(AT_GuildID, GUILD_NONE);
		mlog(GUILDS__OUT_PACKETS, "Sending spawn appearance for no guild tag.");
	}
	else {
		uint8 rank = guild_mgr.getDisplayedRank(GuildID(), GuildRank(), CharacterID());
		mlog(GUILDS__OUT_PACKETS, "Sending spawn appearance for guild %d at rank %d", GuildID(), rank);
		SendAppearancePacket(AT_GuildID, GuildID());
		SendAppearancePacket(AT_GuildRank, rank);
	}

	UpdateWho();
}

void Client::SendGuildList() {
	EQApplicationPacket *outapp;
	//	outapp = new EQApplicationPacket(OP_ZoneGuildList);
	outapp = new EQApplicationPacket(OP_GuildsList);

	//ask the guild manager to build us a nice guild list packet
	outapp->pBuffer = guild_mgr.makeGuildList(/*GetName()*/"", outapp->size);
	if (outapp->pBuffer == nullptr) {
		mlog(GUILDS__ERROR, "Unable to make guild list!");
		return;
	}

	mlog(GUILDS__OUT_PACKETS, "Sending OP_ZoneGuildList of length %d", outapp->size);
	//	mpkt(GUILDS__OUT_PACKET_TRACE, outapp);

	FastQueuePacket(&outapp);
}


void Client::SendGuildMembers() {
	uint32 len;
	uint8 *data = guild_mgr.makeGuildMembers(GuildID(), getName(), len);
	if (data == nullptr)
		return;	//invalid guild, shouldent happen.

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_GuildMemberList);
	outapp->size = len;
	outapp->pBuffer = data;
	data = nullptr;

	mlog(GUILDS__OUT_PACKETS, "Sending OP_GuildMemberList of length %d", outapp->size);
	mpkt(GUILDS__OUT_PACKET_TRACE, outapp);

	FastQueuePacket(&outapp);

	ServerPacket* pack = new ServerPacket(ServerOP_RequestOnlineGuildMembers, sizeof(ServerRequestOnlineGuildMembers_Struct));

	ServerRequestOnlineGuildMembers_Struct *srogms = (ServerRequestOnlineGuildMembers_Struct*)pack->pBuffer;

	srogms->FromID = CharacterID();
	srogms->GuildID = GuildID();

	worldserver.SendPacket(pack);

	safe_delete(pack);

	// We need to send the Guild URL and Channel name again, as sending OP_GuildMemberList appears to clear this information out.
	SendGuildURL();
	SendGuildChannel();
}

void Client::RefreshGuildInfo()
{
	uint32 OldGuildID = guild_id;

	guildrank = GUILD_RANK_NONE;
	guild_id = GUILD_NONE;

	bool WasBanker = GuildBanker;

	CharacterGuildInfo info;
	if (!guild_mgr.getCharInfo(CharacterID(), info)) {
		mlog(GUILDS__ERROR, "Unable to obtain guild char info for %s (%d)", getName(), CharacterID());
		return;
	}

	guildrank = info.mRank;
	guild_id = info.mGuildID;
	GuildBanker = info.mBanker || guild_mgr.isGuildLeader(GuildID(), CharacterID());

	if (((int)zone->GetZoneID() == RuleI(World, GuildBankZoneID)))
	{
		if (WasBanker != GuildBanker)
		{
			EQApplicationPacket *outapp = new EQApplicationPacket(OP_SetGuildRank, sizeof(GuildSetRank_Struct));

			GuildSetRank_Struct *gsrs = (GuildSetRank_Struct*)outapp->pBuffer;

			gsrs->Rank = guildrank;
			strn0cpy(gsrs->MemberName, getName(), sizeof(gsrs->MemberName));
			gsrs->Banker = GuildBanker;

			FastQueuePacket(&outapp);
		}

		if ((guild_id != OldGuildID) && GuildBanks)
		{
			ClearGuildBank();

			if (guild_id != GUILD_NONE)
				GuildBanks->sendGuildBank(this);
		}
	}

	SendGuildSpawnAppearance();
}

void EntityList::sendGuildMOTD(uint32 pGuildID) {
	if (pGuildID == GUILD_NONE)
		return;

	for (auto i = mClients.begin(); i != mClients.end(); i++) {
		Client* client = i->second;
		if (client->GuildID() == pGuildID) {
			client->SendGuildMOTD();
			client->SendGuildURL();
			client->SendGuildChannel();
		}
	}
}

void EntityList::sendGuildSpawnAppearance(uint32 pGuildID) {
	if (pGuildID == GUILD_NONE)
		return;

	for (auto i = mClients.begin(); i != mClients.end(); i++) {
		Client* client = i->second;
		if (client->GuildID() == pGuildID) {
			client->SendGuildSpawnAppearance();
		}
	}
}

void EntityList::refreshAllGuildInfo(uint32 pGuildID) {
	if (pGuildID == GUILD_NONE)
		return;

	for (auto i = mClients.begin(); i != mClients.end(); i++) {
		Client* client = i->second;
		if (client->GuildID() == pGuildID) {
			client->RefreshGuildInfo();
		}
	}
}

void EntityList::sendGuildMembers(uint32 pGuildID) {
	if (pGuildID == GUILD_NONE)
		return;

	//this could be optimized a bit to only build the member's packet once
	//and then keep swapping out the name in the packet on each send.

	for (auto i = mClients.begin(); i != mClients.end(); i++) {
		Client* client = i->second;
		if (client->GuildID() == pGuildID) {
			client->SendGuildMembers();
		}
	}
}

void EntityList::sendGuildList() {
	for (auto i = mClients.begin(); i != mClients.end(); i++) {
		i->second->SendGuildList();
	}
}

void Client::SendGuildJoin(GuildJoin_Struct* gj){
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_GuildManageAdd, sizeof(GuildJoin_Struct));
	GuildJoin_Struct* outgj = (GuildJoin_Struct*)outapp->pBuffer;
	outgj->class_ = gj->class_;
	outgj->guild_id = gj->guild_id;
	outgj->level = gj->level;
	strcpy(outgj->name, gj->name);
	outgj->rank = gj->rank;
	outgj->zoneid = gj->zoneid;

	mlog(GUILDS__OUT_PACKETS, "Sending OP_GuildManageAdd for join of length %d", outapp->size);
	mpkt(GUILDS__OUT_PACKET_TRACE, outapp);

	FastQueuePacket(&outapp);
}