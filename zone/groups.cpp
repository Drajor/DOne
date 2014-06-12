/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

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
#include "NpcAI.h"
#include "../common/packet_functions.h"
#include "../common/packet_dump.h"
#include "../common/StringUtil.h"
#include "worldserver.h"
extern EntityList entity_list;
extern WorldServer worldserver;

//
// Xorlac: This will need proper synchronization to make it work correctly.
//			Also, should investigate client ack for packet to ensure proper synch.
//

/*

note about how groups work:
A group contains 2 list, a list of pointers to members and a
list of member names. All members of a group should have their
name in the membername array, wether they are in the zone or not.
Only members in this zone will have non-null pointers in the
members array.

*/

//create a group which should allready exist in the database
Group::Group(uint32 gid)
: GroupIDConsumer(gid)
{
	mLeader = nullptr;
	memset(mMembers, 0, sizeof(Mob*)* MAX_GROUP_MEMBERS);
	mAssistTargetID = 0;
	mTankTargetID = 0;
	mPullerTargetID = 0;

	memset(&mLeaderAbilities, 0, sizeof(GroupLeadershipAA_Struct));
	uint32 i;
	for (i = 0; i < MAX_GROUP_MEMBERS; i++)
	{
		memset(mMemberNames[i], 0, 64);
		mMemberRoles[i] = 0;
	}

	if (gid != 0) {
		if (!learnMembers())
			SetID(0);
	}
	for (int i = 0; i < MAX_MARKED_NPCS; ++i)
		mMarkedNPCs[i] = 0;

	mNPCMarkerID = 0;
}

//creating a new group
Group::Group(Mob* leader)
: GroupIDConsumer()
{
	memset(mMembers, 0, sizeof(mMembers));
	mMembers[0] = leader;
	leader->SetGrouped(true);
	setLeader(leader);
	mAssistTargetID = 0;
	mTankTargetID = 0;
	mPullerTargetID = 0;
	memset(&mLeaderAbilities, 0, sizeof(GroupLeadershipAA_Struct));
	uint32 i;
	for (i = 0; i < MAX_GROUP_MEMBERS; i++)
	{
		memset(mMemberNames[i], 0, 64);
		mMemberRoles[i] = 0;
	}
	strcpy(mMemberNames[0], leader->GetName());

	if (leader->IsClient())
		strcpy(leader->CastToClient()->GetPP().groupMembers[0], leader->GetName());

	for (int i = 0; i < MAX_MARKED_NPCS; ++i)
		mMarkedNPCs[i] = 0;

	mNPCMarkerID = 0;
}

Group::~Group()
{
	for (int i = 0; i < MAX_MARKED_NPCS; ++i)
	if (mMarkedNPCs[i])
	{
		Mob* m = entity_list.GetMob(mMarkedNPCs[i]);
		if (m)
			m->IsTargeted(-1);
	}
}

//Cofruben:Split money used in OP_Split.
//Rewritten by Father Nitwit
void Group::splitMoney(uint32 copper, uint32 silver, uint32 gold, uint32 platinum, Client *splitter) {
	//avoid unneeded work
	if (copper == 0 && silver == 0 && gold == 0 && platinum == 0)
		return;

	uint32 i;
	uint8 membercount = 0;
	for (i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (mMembers[i] != nullptr) {
			membercount++;
		}
	}

	if (membercount == 0)
		return;

	uint32 mod;
	//try to handle round off error a little better
	if (membercount > 1) {
		mod = platinum % membercount;
		if ((mod) > 0) {
			platinum -= mod;
			gold += 10 * mod;
		}
		mod = gold % membercount;
		if ((mod) > 0) {
			gold -= mod;
			silver += 10 * mod;
		}
		mod = silver % membercount;
		if ((mod) > 0) {
			silver -= mod;
			copper += 10 * mod;
		}
	}

	//calculate the splits
	//We can still round off copper pieces, but I dont care
	uint32 sc;
	uint32 cpsplit = copper / membercount;
	sc = copper % membercount;
	uint32 spsplit = silver / membercount;
	uint32 gpsplit = gold / membercount;
	uint32 ppsplit = platinum / membercount;

	char buf[128];
	buf[63] = '\0';
	std::string msg = "You receive";
	bool one = false;

	if (ppsplit > 0) {
		snprintf(buf, 63, " %u platinum", ppsplit);
		msg += buf;
		one = true;
	}
	if (gpsplit > 0) {
		if (one)
			msg += ",";
		snprintf(buf, 63, " %u gold", gpsplit);
		msg += buf;
		one = true;
	}
	if (spsplit > 0) {
		if (one)
			msg += ",";
		snprintf(buf, 63, " %u silver", spsplit);
		msg += buf;
		one = true;
	}
	if (cpsplit > 0) {
		if (one)
			msg += ",";
		//this message is not 100% accurate for the splitter
		//if they are receiving any roundoff
		snprintf(buf, 63, " %u copper", cpsplit);
		msg += buf;
		one = true;
	}
	msg += " as your split";

	for (i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (mMembers[i] != nullptr && mMembers[i]->IsClient()) { // If Group Member is Client
			Client *c = mMembers[i]->CastToClient();
			//I could not get MoneyOnCorpse to work, so we use this
			c->AddMoneyToPP(cpsplit, spsplit, gpsplit, ppsplit, true);

			c->Message(2, msg.c_str());
		}
	}
}

bool Group::addMember(Mob* newmember, const char *NewMemberName, uint32 CharacterID)
{
	bool InZone = true;
	bool ismerc = false;

	// This method should either be passed a Mob*, if the new member is in this zone, or a nullptr Mob*
	// and the name and CharacterID of the new member, if they are out of zone.
	//
	if (!newmember && !NewMemberName)
		return false;

	if (groupCount() >= MAX_GROUP_MEMBERS) //Sanity check for merging groups together.
		return false;

	if (!newmember)
		InZone = false;
	else
	{
		NewMemberName = newmember->GetCleanName();

		if (newmember->IsClient())
			CharacterID = newmember->CastToClient()->CharacterID();
		if (newmember->IsMerc())
		{
			Client* owner = newmember->CastToMerc()->GetMercOwner();
			if (owner)
			{
				CharacterID = owner->CastToClient()->CharacterID();
				NewMemberName = newmember->GetName();
				ismerc = true;
			}
		}
	}

	uint32 i = 0;

	// See if they are already in the group
	//
	for (i = 0; i < MAX_GROUP_MEMBERS; ++i)
	if (!strcasecmp(mMemberNames[i], NewMemberName))
		return false;

	// Put them in the group
	for (i = 0; i < MAX_GROUP_MEMBERS; ++i)
	{
		if (mMemberNames[i][0] == '\0')
		{
			if (InZone)
				mMembers[i] = newmember;

			break;
		}
	}

	if (i == MAX_GROUP_MEMBERS)
		return false;

	strcpy(mMemberNames[i], NewMemberName);
	mMemberRoles[i] = 0;

	int x = 1;

	//build the template join packet
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupJoin_Struct));
	GroupJoin_Struct* gj = (GroupJoin_Struct*)outapp->pBuffer;
	strcpy(gj->membername, NewMemberName);
	gj->action = groupActJoin;

	gj->leader_aas = mLeaderAbilities;

	for (i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (mMembers[i] != nullptr && mMembers[i] != newmember) {
			//fill in group join & send it
			if (mMembers[i]->IsMerc())
			{
				strcpy(gj->yourname, mMembers[i]->GetName());
			}
			else
			{
				strcpy(gj->yourname, mMembers[i]->GetCleanName());
			}
			if (mMembers[i]->IsClient()) {
				mMembers[i]->CastToClient()->QueuePacket(outapp);

				//put new member into existing person's list
				strcpy(mMembers[i]->CastToClient()->GetPP().groupMembers[this->groupCount() - 1], NewMemberName);
			}

			//put this existing person into the new member's list
			if (InZone && newmember->IsClient()) {
				if (isLeader(mMembers[i]))
					strcpy(newmember->CastToClient()->GetPP().groupMembers[0], mMembers[i]->GetCleanName());
				else {
					strcpy(newmember->CastToClient()->GetPP().groupMembers[x], mMembers[i]->GetCleanName());
					x++;
				}
			}
		}
	}

	if (InZone)
	{
		//put new member in his own list.
		newmember->SetGrouped(true);

		if (newmember->IsClient())
		{
			strcpy(newmember->CastToClient()->GetPP().groupMembers[x], NewMemberName);
			newmember->CastToClient()->Save();
			database.SetGroupID(NewMemberName, GetID(), newmember->CastToClient()->CharacterID(), false);
			sendMarkedNPCsToMember(newmember->CastToClient());

			notifyMainTank(newmember->CastToClient(), 1);
			notifyMainAssist(newmember->CastToClient(), 1);
			notifyPuller(newmember->CastToClient(), 1);
		}

		if (newmember->IsMerc())
		{
			Client* owner = newmember->CastToMerc()->GetMercOwner();
			if (owner)
			{
				database.SetGroupID(newmember->GetName(), GetID(), owner->CharacterID(), true);
			}
		}
	}
	else
		database.SetGroupID(NewMemberName, GetID(), CharacterID, ismerc);

	safe_delete(outapp);

	return true;
}

void Group::addMember(const char *NewMemberName)
{
	// This method should be called when both the new member and the group leader are in a different zone to this one.
	//
	for (uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i)
	if (!strcasecmp(mMemberNames[i], NewMemberName))
	{
		return;
	}

	for (uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i)
	{
		if (mMemberNames[i][0] == '\0')
		{
			strcpy(mMemberNames[i], NewMemberName);
			mMemberRoles[i] = 0;
			break;
		}
	}
}


void Group::queuePacket(const EQApplicationPacket *app, bool ack_req)
{
	uint32 i;
	for (i = 0; i < MAX_GROUP_MEMBERS; i++)
	if (mMembers[i] && mMembers[i]->IsClient())
		mMembers[i]->CastToClient()->QueuePacket(app, ack_req);
}

// solar: sends the rest of the group's hps to member. this is useful when
// someone first joins a group, but otherwise there shouldn't be a need to
// call it
void Group::sendHPPacketsTo(Mob *member)
{
	if (member && member->IsClient())
	{
		EQApplicationPacket hpapp;
		EQApplicationPacket outapp(OP_MobManaUpdate, sizeof(MobManaUpdate_Struct));

		for (uint32 i = 0; i < MAX_GROUP_MEMBERS; i++)
		{
			if (mMembers[i] && mMembers[i] != member)
			{
				mMembers[i]->CreateHPPacket(&hpapp);
				member->CastToClient()->QueuePacket(&hpapp, false);
				if (member->CastToClient()->GetClientVersion() >= EQClientSoD)
				{
					outapp.SetOpcode(OP_MobManaUpdate);
					MobManaUpdate_Struct *mmus = (MobManaUpdate_Struct *)outapp.pBuffer;
					mmus->spawn_id = mMembers[i]->GetID();
					mmus->mana = mMembers[i]->GetManaPercent();
					member->CastToClient()->QueuePacket(&outapp, false);
					MobEnduranceUpdate_Struct *meus = (MobEnduranceUpdate_Struct *)outapp.pBuffer;
					outapp.SetOpcode(OP_MobEnduranceUpdate);
					meus->endurance = mMembers[i]->GetEndurancePercent();
					member->CastToClient()->QueuePacket(&outapp, false);
				}
			}
		}
	}
}

void Group::sendHPPacketsFrom(Mob *member)
{
	EQApplicationPacket hp_app;
	if (!member)
		return;

	member->CreateHPPacket(&hp_app);
	EQApplicationPacket outapp(OP_MobManaUpdate, sizeof(MobManaUpdate_Struct));

	uint32 i;
	for (i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (mMembers[i] && mMembers[i] != member && mMembers[i]->IsClient())
		{
			mMembers[i]->CastToClient()->QueuePacket(&hp_app);
			if (mMembers[i]->CastToClient()->GetClientVersion() >= EQClientSoD)
			{
				outapp.SetOpcode(OP_MobManaUpdate);
				MobManaUpdate_Struct *mmus = (MobManaUpdate_Struct *)outapp.pBuffer;
				mmus->spawn_id = member->GetID();
				mmus->mana = member->GetManaPercent();
				mMembers[i]->CastToClient()->QueuePacket(&outapp, false);
				MobEnduranceUpdate_Struct *meus = (MobEnduranceUpdate_Struct *)outapp.pBuffer;
				outapp.SetOpcode(OP_MobEnduranceUpdate);
				meus->endurance = member->GetEndurancePercent();
				mMembers[i]->CastToClient()->QueuePacket(&outapp, false);
			}
		}
	}
}

//updates a group member's client pointer when they zone in
//if the group was in the zone allready
bool Group::updatePlayer(Mob* update){

	verifyGroup();

	uint32 i = 0;
	if (update->IsClient()) {
		//update their player profile
		PlayerProfile_Struct &pp = update->CastToClient()->GetPP();
		for (i = 0; i < MAX_GROUP_MEMBERS; i++) {
			if (mMemberNames[0] == '\0')
				memset(pp.groupMembers[i], 0, 64);
			else
				strn0cpy(pp.groupMembers[i], mMemberNames[i], 64);
		}
		if (isNPCMarker(update->CastToClient()))
		{
			mNPCMarkerID = update->GetID();
			sendLeadershipAAUpdate();
		}
	}

	for (i = 0; i < MAX_GROUP_MEMBERS; i++)
	{
		if (!strcasecmp(mMemberNames[i], update->GetName()))
		{
			mMembers[i] = update;
			mMembers[i]->SetGrouped(true);
			return true;
		}
	}
	return false;
}


void Group::memberZoned(Mob* removemob) {
	uint32 i;

	if (removemob == nullptr)
		return;

	if (removemob == getLeader())
		setLeader(nullptr);

	for (i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (mMembers[i] == removemob) {
			mMembers[i] = nullptr;
			//should NOT clear the name, it is used for world communication.
			break;
		}
	}
	if (removemob->IsClient() && hasRole(removemob, RoleAssist))
		setGroupAssistTarget(0);

	if (removemob->IsClient() && hasRole(removemob, RoleTank))
		setGroupTankTarget(0);

	if (removemob->IsClient() && hasRole(removemob, RolePuller))
		setGroupPullerTarget(0);
}

bool Group::delMemberOOZ(const char *Name) {

	if (!Name) return false;

	// If a member out of zone has disbanded, clear out their name.
	//
	for (unsigned int i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (!strcasecmp(Name, mMemberNames[i]))
			// This shouldn't be called if the member is in this zone.
		if (!mMembers[i]) {
			if (!strncmp(getLeaderName(), Name, 64))
			{
				//TODO: Transfer leadership if leader disbands OOZ.
				updateGroupAAs();
			}

			memset(mMemberNames[i], 0, 64);
			mMemberRoles[i] = 0;
			if (groupCount() < 3)
			{
				unDelegateMarkNPC(mNPCMarkerName.c_str());
				if (getLeader() && getLeader()->IsClient() && getLeader()->CastToClient()->GetClientVersion() < EQClientSoD) {
					undelegateMainAssist(mMainAssistName.c_str());
				}
				clearAllNPCMarks();
			}
			return true;
		}
	}

	return false;
}

bool Group::delMember(Mob* oldmember, bool ignoresender)
{
	if (oldmember == nullptr){
		return false;
	}

	for (uint32 i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (mMembers[i] == oldmember) {
			mMembers[i] = nullptr;
			mMemberNames[i][0] = '\0';
			memset(mMemberNames[i], 0, 64);
			mMemberRoles[i] = 0;
			break;
		}
	}

	//handle leader quitting group gracefully
	if (oldmember == getLeader() && groupCount() >= 2) {
		for (uint32 nl = 0; nl < MAX_GROUP_MEMBERS; nl++) {
			if (mMembers[nl]) {
				changeLeader(mMembers[nl]);
				break;
			}
		}
	}

	ServerPacket* pack = new ServerPacket(ServerOP_GroupLeave, sizeof(ServerGroupLeave_Struct));
	ServerGroupLeave_Struct* gl = (ServerGroupLeave_Struct*)pack->pBuffer;
	gl->gid = GetID();
	gl->zoneid = zone->GetZoneID();
	gl->instance_id = zone->GetInstanceID();
	strcpy(gl->member_name, oldmember->GetName());
	worldserver.SendPacket(pack);
	safe_delete(pack);

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupJoin_Struct));
	GroupJoin_Struct* gu = (GroupJoin_Struct*)outapp->pBuffer;
	gu->action = groupActLeave;
	strcpy(gu->membername, oldmember->GetCleanName());
	strcpy(gu->yourname, oldmember->GetCleanName());

	gu->leader_aas = mLeaderAbilities;

	for (uint32 i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (mMembers[i] == nullptr) {
			//if (DEBUG>=5) LogFile->write(EQEMuLog::Debug, "Group::DelMember() null member at slot %i", i);
			continue;
		}
		if (mMembers[i] != oldmember) {
			strcpy(gu->yourname, mMembers[i]->GetCleanName());
			if (mMembers[i]->IsClient())
				mMembers[i]->CastToClient()->QueuePacket(outapp);
		}
	}

	if (!ignoresender) {
		strcpy(gu->yourname, oldmember->GetCleanName());
		strcpy(gu->membername, oldmember->GetCleanName());
		gu->action = groupActLeave;

		if (oldmember->IsClient())
			oldmember->CastToClient()->QueuePacket(outapp);
	}

	if (oldmember->IsClient())
		database.SetGroupID(oldmember->GetCleanName(), 0, oldmember->CastToClient()->CharacterID());

	oldmember->SetGrouped(false);
	mDisbandCheck = true;

	safe_delete(outapp);

	if (hasRole(oldmember, RoleTank))
	{
		setGroupTankTarget(0);
		undelegateMainTank(oldmember->GetName());
	}

	if (hasRole(oldmember, RoleAssist))
	{
		setGroupAssistTarget(0);
		undelegateMainAssist(oldmember->GetName());
	}

	if (hasRole(oldmember, RolePuller))
	{
		setGroupPullerTarget(0);
		undelegatePuller(oldmember->GetName());
	}

	if (oldmember->IsClient())
		sendMarkedNPCsToMember(oldmember->CastToClient(), true);

	if (groupCount() < 3)
	{
		unDelegateMarkNPC(mNPCMarkerName.c_str());
		if (getLeader() && getLeader()->IsClient() && getLeader()->CastToClient()->GetClientVersion() < EQClientSoD) {
			undelegateMainAssist(mMainAssistName.c_str());
		}
		clearAllNPCMarks();
	}

	return true;
}

// does the caster + group
void Group::castGroupSpell(Mob* caster, uint16 spell_id) {
	uint32 z;
	float range, distance;

	if (!caster)
		return;

	mCastSpell = true;
	range = caster->GetAOERange(spell_id);

	float range2 = range*range;

	//	caster->SpellOnTarget(spell_id, caster);

	for (z = 0; z < MAX_GROUP_MEMBERS; z++)
	{
		if (mMembers[z] == caster) {
			caster->SpellOnTarget(spell_id, caster);
#ifdef GROUP_BUFF_PETS
			if (caster->GetPet() && caster->HasPetAffinity() && !caster->GetPet()->IsCharmed())
				caster->SpellOnTarget(spell_id, caster->GetPet());
#endif
		}
		else if (mMembers[z] != nullptr)
		{
			distance = caster->DistNoRoot(*mMembers[z]);
			if (distance <= range2) {
				caster->SpellOnTarget(spell_id, mMembers[z]);
#ifdef GROUP_BUFF_PETS
				if (mMembers[z]->GetPet() && mMembers[z]->HasPetAffinity() && !mMembers[z]->GetPet()->IsCharmed())
					caster->SpellOnTarget(spell_id, mMembers[z]->GetPet());
#endif
			}
			else
				_log(SPELLS__CASTING, "Group spell: %s is out of range %f at distance %f from %s", mMembers[z]->GetName(), range, distance, caster->GetName());
		}
	}

	mCastSpell = false;
	mDisbandCheck = true;
}

// does the caster + group
void Group::groupBardPulse(Mob* caster, uint16 spell_id) {
	uint32 z;
	float range, distance;

	if (!caster)
		return;

	mCastSpell = true;
	range = caster->GetAOERange(spell_id);

	float range2 = range*range;

	for (z = 0; z < MAX_GROUP_MEMBERS; z++) {
		if (mMembers[z] == caster) {
			caster->BardPulse(spell_id, caster);
#ifdef GROUP_BUFF_PETS
			if (caster->GetPet() && caster->HasPetAffinity() && !caster->GetPet()->IsCharmed())
				caster->BardPulse(spell_id, caster->GetPet());
#endif
		}
		else if (mMembers[z] != nullptr)
		{
			distance = caster->DistNoRoot(*mMembers[z]);
			if (distance <= range2) {
				mMembers[z]->BardPulse(spell_id, caster);
#ifdef GROUP_BUFF_PETS
				if (mMembers[z]->GetPet() && mMembers[z]->HasPetAffinity() && !mMembers[z]->GetPet()->IsCharmed())
					mMembers[z]->GetPet()->BardPulse(spell_id, caster);
#endif
			}
			else
				_log(SPELLS__BARDS, "Group bard pulse: %s is out of range %f at distance %f from %s", mMembers[z]->GetName(), range, distance, caster->GetName());
		}
	}
}

bool Group::isGroupMember(Mob* client)
{
	bool Result = false;

	if (client) {
		for (uint32 i = 0; i < MAX_GROUP_MEMBERS; i++) {
			if (mMembers[i] == client)
				Result = true;
		}
	}

	return Result;
}

bool Group::isGroupMember(const char *Name)
{
	if (Name)
	for (uint32 i = 0; i < MAX_GROUP_MEMBERS; i++)
	if ((strlen(Name) == strlen(mMemberNames[i])) && !strncmp(mMemberNames[i], Name, strlen(Name)))
		return true;

	return false;
}

void Group::groupMessage(Mob* sender, uint8 language, uint8 lang_skill, const char* message) {
	uint32 i;
	for (i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (!mMembers[i])
			continue;

		if (mMembers[i]->IsClient() && mMembers[i]->CastToClient()->GetFilter(FilterGroupChat) != 0)
			mMembers[i]->CastToClient()->ChannelMessageSend(sender->GetName(), mMembers[i]->GetName(), 2, language, lang_skill, message);
	}

	ServerPacket* pack = new ServerPacket(ServerOP_OOZGroupMessage, sizeof(ServerGroupChannelMessage_Struct)+strlen(message) + 1);
	ServerGroupChannelMessage_Struct* gcm = (ServerGroupChannelMessage_Struct*)pack->pBuffer;
	gcm->zoneid = zone->GetZoneID();
	gcm->groupid = GetID();
	gcm->instanceid = zone->GetInstanceID();
	strcpy(gcm->from, sender->GetName());
	strcpy(gcm->message, message);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

uint32 Group::getTotalGroupDamage(Mob* other) {
	uint32 total = 0;

	uint32 i;
	for (i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (!mMembers[i])
			continue;
		if (other->CheckAggro(mMembers[i]))
			total += other->GetHateAmount(mMembers[i], true);
	}
	return total;
}

void Group::disbandGroup() {
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupUpdate_Struct));

	GroupUpdate_Struct* gu = (GroupUpdate_Struct*)outapp->pBuffer;
	gu->action = groupActDisband;

	Client *Leader = nullptr;

	uint32 i;
	for (i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (mMembers[i] == nullptr) {
			continue;
		}

		if (mMembers[i]->IsClient()) {
			if (isLeader(mMembers[i]))
				Leader = mMembers[i]->CastToClient();

			strcpy(gu->yourname, mMembers[i]->GetName());
			database.SetGroupID(mMembers[i]->GetName(), 0, mMembers[i]->CastToClient()->CharacterID());
			mMembers[i]->CastToClient()->QueuePacket(outapp);
			sendMarkedNPCsToMember(mMembers[i]->CastToClient(), true);

		}

		mMembers[i]->SetGrouped(false);
		mMembers[i] = nullptr;
		mMemberNames[i][0] = '\0';
	}

	clearAllNPCMarks();

	ServerPacket* pack = new ServerPacket(ServerOP_DisbandGroup, sizeof(ServerDisbandGroup_Struct));
	ServerDisbandGroup_Struct* dg = (ServerDisbandGroup_Struct*)pack->pBuffer;
	dg->zoneid = zone->GetZoneID();
	dg->groupid = GetID();
	dg->instance_id = zone->GetInstanceID();
	worldserver.SendPacket(pack);
	safe_delete(pack);

	entity_list.RemoveGroup(GetID());
	if (GetID() != 0)
		database.ClearGroup(GetID());

	if (Leader && (Leader->IsLFP())) {
		Leader->UpdateLFP();
	}

	safe_delete(outapp);
}

bool Group::process() {
	if (mDisbandCheck && !groupCount())
		return false;
	else if (mDisbandCheck && groupCount())
		mDisbandCheck = false;
	return true;
}

void Group::sendUpdate(uint32 type, Mob* member)
{
	if (!member->IsClient())
		return;

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupUpdate2_Struct));
	GroupUpdate2_Struct* gu = (GroupUpdate2_Struct*)outapp->pBuffer;
	gu->action = type;
	strcpy(gu->yourname, member->GetName());

	int x = 0;

	gu->leader_aas = mLeaderAbilities;

	for (uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i)
	if ((mMembers[i] != nullptr) && isLeader(mMembers[i]))
	{
		strcpy(gu->leadersname, mMembers[i]->GetName());
		break;
	}

	for (uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i)
	if (mMembers[i] != nullptr && mMembers[i] != member)
		strcpy(gu->membername[x++], mMembers[i]->GetName());

	member->CastToClient()->QueuePacket(outapp);

	safe_delete(outapp);
}

void Group::sendLeadershipAAUpdate()
{
	// This method updates other members of the group in the current zone with the Leader's group leadership AAs.
	//
	// It is called when the leader purchases a leadership AA or enters a zone.
	//
	// If a group member is not in the same zone as the leader when the leader purchases a new AA, they will not become
	// aware of it until they are next in the same zone as the leader.

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupJoin_Struct));

	GroupJoin_Struct* gu = (GroupJoin_Struct*)outapp->pBuffer;

	gu->action = groupActAAUpdate;

	uint32 i = 0;

	gu->leader_aas = mLeaderAbilities;

	gu->NPCMarkerID = getNPCMarkerID();

	for (i = 0; i < MAX_GROUP_MEMBERS; ++i)
	if (mMembers[i] && mMembers[i]->IsClient())
	{
		strcpy(gu->yourname, mMembers[i]->GetName());
		strcpy(gu->membername, mMembers[i]->GetName());
		mMembers[i]->CastToClient()->QueuePacket(outapp);
	}

	safe_delete(outapp);
}

uint8 Group::groupCount() {

	uint8 MemberCount = 0;

	for (uint8 i = 0; i < MAX_GROUP_MEMBERS; ++i)
	if (mMemberNames[i][0])
		++MemberCount;

	return MemberCount;
}

uint32 Group::getHighestLevel()
{
	uint32 level = 1;
	uint32 i;
	for (i = 0; i < MAX_GROUP_MEMBERS; i++)
	{
		if (mMembers[i])
		{
			if (mMembers[i]->GetLevel() > level)
				level = mMembers[i]->GetLevel();
		}
	}
	return level;
}
uint32 Group::getLowestLevel()
{
	uint32 level = 255;
	uint32 i;
	for (i = 0; i < MAX_GROUP_MEMBERS; i++)
	{
		if (mMembers[i])
		{
			if (mMembers[i]->GetLevel() < level)
				level = mMembers[i]->GetLevel();
		}
	}
	return level;
}

void Group::teleportGroup(Mob* sender, uint32 zoneID, uint16 instance_id, float x, float y, float z, float heading)
{
	uint32 i;
	for (i = 0; i < MAX_GROUP_MEMBERS; i++)
	{
		if (mMembers[i] != nullptr && mMembers[i]->IsClient() && mMembers[i] != sender)
		{
			mMembers[i]->CastToClient()->MovePC(zoneID, instance_id, x, y, z, heading, 0, ZoneSolicited);
		}
	}
}

bool Group::learnMembers() {
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	if (database.RunQuery(query, MakeAnyLenString(&query, "SELECT name FROM group_id WHERE groupid=%lu", (unsigned long)GetID()),
		errbuf, &result)){
		safe_delete_array(query);
		if (mysql_num_rows(result) < 1) {	//could prolly be 2
			mysql_free_result(result);
			LogFile->write(EQEMuLog::Error, "Error getting group members for group %lu: %s", (unsigned long)GetID(), errbuf);
			return(false);
		}
		int i = 0;
		while ((row = mysql_fetch_row(result))) {
			if (!row[0])
				continue;
			mMembers[i] = nullptr;
			strn0cpy(mMemberNames[i], row[0], 64);

			i++;
		}
		mysql_free_result(result);
	}

	return(true);
}

void Group::verifyGroup() {
	/*
		The purpose of this method is to make sure that a group
		is in a valid state, to prevent dangling pointers.
		Only called every once in a while (on member re-join for now).
		*/

	uint32 i;
	for (i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (mMemberNames[i][0] == '\0') {
#if EQDEBUG >= 7
			LogFile->write(EQEMuLog::Debug, "Group %lu: Verify %d: Empty.\n", (unsigned long)GetID(), i);
#endif
			mMembers[i] = nullptr;
			continue;
		}

		//it should be safe to use GetClientByName, but Group is trying
		//to be generic, so we'll go for general Mob
		Mob *them = entity_list.GetMob(mMemberNames[i]);
		if (them == nullptr && mMembers[i] != nullptr) {	//they arnt here anymore....
#if EQDEBUG >= 6
			LogFile->write(EQEMuLog::Debug, "Member of group %lu named '%s' has disappeared!!", (unsigned long)GetID(), mMemberNames[i]);
#endif
			mMemberNames[i][0] = '\0';
			mMembers[i] = nullptr;
			continue;
		}

		if (them != nullptr && mMembers[i] != them) {	//our pointer is out of date... not so good.
#if EQDEBUG >= 5
			LogFile->write(EQEMuLog::Debug, "Member of group %lu named '%s' had an out of date pointer!!", (unsigned long)GetID(), mMemberNames[i]);
#endif
			mMembers[i] = them;
			continue;
		}
#if EQDEBUG >= 8
		LogFile->write(EQEMuLog::Debug, "Member of group %lu named '%s' is valid.", (unsigned long)GetID(), mMemberNames[i]);
#endif
	}
}


void Group::groupMessage_StringID(Mob* sender, uint32 type, uint32 string_id, const char* message, const char* message2, const char* message3, const char* message4, const char* message5, const char* message6, const char* message7, const char* message8, const char* message9, uint32 distance) {
	uint32 i;
	for (i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (mMembers[i] == nullptr)
			continue;

		if (mMembers[i] == sender)
			continue;

		mMembers[i]->Message_StringID(type, string_id, message, message2, message3, message4, message5, message6, message7, message8, message9, 0);
	}
}



void Client::LeaveGroup() {
	Group *g = GetGroup();

	if (g) {
		if (g->groupCount() < 3)
			g->disbandGroup();
		else
			g->delMember(this);
	}
	else {
		//force things a little
		database.SetGroupID(GetName(), 0, CharacterID());
	}

	isgrouped = false;
}

void Group::healGroup(uint32 heal_amt, Mob* caster, int32 range)
{
	if (!caster)
		return;

	if (!range)
		range = 200;

	float distance;
	float range2 = range*range;


	int numMem = 0;
	unsigned int gi = 0;
	for (; gi < MAX_GROUP_MEMBERS; gi++)
	{
		if (mMembers[gi]){
			distance = caster->DistNoRoot(*mMembers[gi]);
			if (distance <= range2){
				numMem += 1;
			}
		}
	}

	heal_amt /= numMem;
	for (gi = 0; gi < MAX_GROUP_MEMBERS; gi++)
	{
		if (mMembers[gi]){
			distance = caster->DistNoRoot(*mMembers[gi]);
			if (distance <= range2){
				mMembers[gi]->HealDamage(heal_amt, caster);
				mMembers[gi]->SendHPUpdate();
			}
		}
	}
}


void Group::balanceHP(int32 penalty, int32 range, Mob* caster)
{
	if (!caster)
		return;

	if (!range)
		range = 200;

	int dmgtaken = 0, numMem = 0;

	float distance;
	float range2 = range*range;

	unsigned int gi = 0;
	for (; gi < MAX_GROUP_MEMBERS; gi++)
	{
		if (mMembers[gi]){
			distance = caster->DistNoRoot(*mMembers[gi]);
			if (distance <= range2){
				dmgtaken += (mMembers[gi]->GetMaxHP() - mMembers[gi]->GetHP());
				numMem += 1;
			}
		}
	}

	dmgtaken += dmgtaken * penalty / 100;
	dmgtaken /= numMem;
	for (gi = 0; gi < MAX_GROUP_MEMBERS; gi++)
	{
		if (mMembers[gi]){
			distance = caster->DistNoRoot(*mMembers[gi]);
			if (distance <= range2){
				if ((mMembers[gi]->GetMaxHP() - dmgtaken) < 1){ //this way the ability will never kill someone
					mMembers[gi]->SetHP(1);					//but it will come darn close
					mMembers[gi]->SendHPUpdate();
				}
				else{
					mMembers[gi]->SetHP(mMembers[gi]->GetMaxHP() - dmgtaken);
					mMembers[gi]->SendHPUpdate();
				}
			}
		}
	}
}

void Group::balanceMana(int32 penalty, int32 range, Mob* caster)
{
	if (!caster)
		return;

	if (!range)
		range = 200;

	float distance;
	float range2 = range*range;

	int manataken = 0, numMem = 0;
	unsigned int gi = 0;
	for (; gi < MAX_GROUP_MEMBERS; gi++)
	{
		if (mMembers[gi]){
			distance = caster->DistNoRoot(*mMembers[gi]);
			if (distance <= range2){
				manataken += (mMembers[gi]->GetMaxMana() - mMembers[gi]->GetMana());
				numMem += 1;
			}
		}
	}

	manataken += manataken * penalty / 100;
	manataken /= numMem;
	for (gi = 0; gi < MAX_GROUP_MEMBERS; gi++)
	{
		if (mMembers[gi]){
			distance = caster->DistNoRoot(*mMembers[gi]);
			if (distance <= range2){
				if ((mMembers[gi]->GetMaxMana() - manataken) < 1){
					mMembers[gi]->SetMana(1);
					if (mMembers[gi]->IsClient())
						mMembers[gi]->CastToClient()->SendManaUpdate();
				}
				else{
					mMembers[gi]->SetMana(mMembers[gi]->GetMaxMana() - manataken);
					if (mMembers[gi]->IsClient())
						mMembers[gi]->CastToClient()->SendManaUpdate();
				}
			}
		}
	}
}

uint16 Group::getAvgLevel()
{
	double levelHolder = 0;
	uint8 i = 0;
	uint8 numMem = 0;
	while (i < MAX_GROUP_MEMBERS)
	{
		if (mMembers[i])
		{
			numMem++;
			levelHolder = levelHolder + (mMembers[i]->GetLevel());
		}
		i++;
	}
	levelHolder = ((levelHolder / numMem) + .5); // total levels divided by num of characters
	return (uint16(levelHolder));
}

void Group::markNPC(Mob* Target, int Number)
{
	// Send a packet to all group members in this zone causing the client to prefix the Target mob's name
	// with the specified Number.
	//
	if (!Target || Target->IsClient())
		return;

	if ((Number < 1) || (Number > MAX_MARKED_NPCS))
		return;

	bool AlreadyMarked = false;

	uint16 EntityID = Target->GetID();

	for (int i = 0; i < MAX_MARKED_NPCS; ++i)
	if (mMarkedNPCs[i] == EntityID)
	{
		if (i == (Number - 1))
			return;

		updateXTargetMarkedNPC(i + 1, nullptr);
		mMarkedNPCs[i] = 0;

		AlreadyMarked = true;

		break;
	}

	if (!AlreadyMarked)
	{
		if (mMarkedNPCs[Number - 1])
		{
			Mob* m = entity_list.GetMob(mMarkedNPCs[Number - 1]);
			if (m)
				m->IsTargeted(-1);

			updateXTargetMarkedNPC(Number, nullptr);
		}

		if (EntityID)
		{
			Mob* m = entity_list.GetMob(Target->GetID());
			if (m)
				m->IsTargeted(1);
		}
	}

	mMarkedNPCs[Number - 1] = EntityID;

	EQApplicationPacket *outapp = new EQApplicationPacket(OP_MarkNPC, sizeof(MarkNPC_Struct));

	MarkNPC_Struct* mnpcs = (MarkNPC_Struct *)outapp->pBuffer;

	mnpcs->TargetID = EntityID;

	mnpcs->Number = Number;

	Mob *m = entity_list.GetMob(EntityID);

	if (m)
		sprintf(mnpcs->Name, "%s", m->GetCleanName());

	queuePacket(outapp);

	safe_delete(outapp);

	updateXTargetMarkedNPC(Number, m);
}

void Group::delegateMainTank(const char *NewMainTankName, uint8 toggle)
{
	// This method is called when the group leader Delegates the Main Tank role to a member of the group
	// (or himself). All group members in the zone are notified of the new Main Tank and it is recorded
	// in the group_leaders table so as to persist across zones.
	//

	bool updateDB = false;

	if (!NewMainTankName)
		return;

	Mob *m = entity_list.GetMob(NewMainTankName);

	if (!m)
		return;

	if (mMainTankName != NewMainTankName || !toggle)
		updateDB = true;

	if (m->GetTarget())
		mTankTargetID = m->GetTarget()->GetID();
	else
		mTankTargetID = 0;

	Mob *mtt = mTankTargetID ? entity_list.GetMob(mTankTargetID) : 0;

	setMainTank(NewMainTankName);

	for (uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i)
	{
		if (mMembers[i] && mMembers[i]->IsClient())
		{
			notifyMainTank(mMembers[i]->CastToClient(), toggle);
			mMembers[i]->CastToClient()->UpdateXTargetType(GroupTank, m, NewMainTankName);
			mMembers[i]->CastToClient()->UpdateXTargetType(GroupTankTarget, mtt);
		}
	}

	if (updateDB) {
		char errbuff[MYSQL_ERRMSG_SIZE];

		char *Query = nullptr;

		if (!database.RunQuery(Query, MakeAnyLenString(&Query, "UPDATE group_leaders SET maintank='%s' WHERE gid=%i LIMIT 1",
			mMainTankName.c_str(), GetID()), errbuff))
			LogFile->write(EQEMuLog::Error, "Unable to set group main tank: %s\n", errbuff);

		safe_delete_array(Query);
	}
}

void Group::delegateMainAssist(const char *NewMainAssistName, uint8 toggle)
{
	// This method is called when the group leader Delegates the Main Assist role to a member of the group
	// (or himself). All group members in the zone are notified of the new Main Assist and it is recorded
	// in the group_leaders table so as to persist across zones.
	//

	bool updateDB = false;

	if (!NewMainAssistName)
		return;

	Mob *m = entity_list.GetMob(NewMainAssistName);

	if (!m)
		return;

	if (mMainAssistName != NewMainAssistName || !toggle)
		updateDB = true;

	if (m->GetTarget())
		mAssistTargetID = m->GetTarget()->GetID();
	else
		mAssistTargetID = 0;

	setMainAssist(NewMainAssistName);

	for (uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i) {
		if (mMembers[i] && mMembers[i]->IsClient())
		{
			notifyMainAssist(mMembers[i]->CastToClient(), toggle);
			mMembers[i]->CastToClient()->UpdateXTargetType(GroupAssist, m, NewMainAssistName);
			mMembers[i]->CastToClient()->UpdateXTargetType(GroupAssistTarget, m->GetTarget());
		}
	}

	if (updateDB) {
		char errbuff[MYSQL_ERRMSG_SIZE];

		char *Query = nullptr;

		if (!database.RunQuery(Query, MakeAnyLenString(&Query, "UPDATE group_leaders SET assist='%s' WHERE gid=%i LIMIT 1",
			mMainAssistName.c_str(), GetID()), errbuff))
			LogFile->write(EQEMuLog::Error, "Unable to set group main assist: %s\n", errbuff);

		safe_delete_array(Query);
	}
}

void Group::delegatePuller(const char *NewPullerName, uint8 toggle)
{
	// This method is called when the group leader Delegates the Puller role to a member of the group
	// (or himself). All group members in the zone are notified of the new Puller and it is recorded
	// in the group_leaders table so as to persist across zones.
	//

	bool updateDB = false;

	if (!NewPullerName)
		return;

	Mob *m = entity_list.GetMob(NewPullerName);

	if (!m)
		return;

	if (mPullerName != NewPullerName || !toggle)
		updateDB = true;

	if (m->GetTarget())
		mPullerTargetID = m->GetTarget()->GetID();
	else
		mPullerTargetID = 0;

	setPuller(NewPullerName);

	for (uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i) {
		if (mMembers[i] && mMembers[i]->IsClient())
		{
			notifyPuller(mMembers[i]->CastToClient(), toggle);
			mMembers[i]->CastToClient()->UpdateXTargetType(Puller, m, NewPullerName);
			mMembers[i]->CastToClient()->UpdateXTargetType(PullerTarget, m->GetTarget());
		}
	}

	if (updateDB) {
		char errbuff[MYSQL_ERRMSG_SIZE];

		char *Query = nullptr;

		if (!database.RunQuery(Query, MakeAnyLenString(&Query, "UPDATE group_leaders SET puller='%s' WHERE gid=%i LIMIT 1",
			mPullerName.c_str(), GetID()), errbuff))
			LogFile->write(EQEMuLog::Error, "Unable to set group main puller: %s\n", errbuff);

		safe_delete_array(Query);
	}

}

void Group::notifyMainTank(Client *c, uint8 toggle)
{
	// Send a packet to the specified Client notifying them who the new Main Tank is. This causes the client to display
	// a message with the name of the Main Tank.
	//

	if (!c)
		return;

	if (!mMainTankName.size())
		return;

	if (c->GetClientVersion() < EQClientSoD)
	{
		if (toggle)
			c->Message(0, "%s is now Main Tank.", mMainTankName.c_str());
		else
			c->Message(0, "%s is no longer Main Tank.", mMainTankName.c_str());
	}
	else
	{
		EQApplicationPacket *outapp = new EQApplicationPacket(OP_GroupRoles, sizeof(GroupRole_Struct));

		GroupRole_Struct *grs = (GroupRole_Struct*)outapp->pBuffer;

		strn0cpy(grs->Name1, mMainTankName.c_str(), sizeof(grs->Name1));

		strn0cpy(grs->Name2, getLeaderName(), sizeof(grs->Name2));

		grs->RoleNumber = 1;

		grs->Toggle = toggle;

		c->QueuePacket(outapp);

		safe_delete(outapp);
	}

}

void Group::notifyMainAssist(Client *c, uint8 toggle)
{
	// Send a packet to the specified Client notifying them who the new Main Assist is. This causes the client to display
	// a message with the name of the Main Assist.
	//

	if (!c)
		return;

	if (!mMainAssistName.size())
		return;

	if (c->GetClientVersion() < EQClientSoD)
	{
		EQApplicationPacket *outapp = new EQApplicationPacket(OP_DelegateAbility, sizeof(DelegateAbility_Struct));

		DelegateAbility_Struct* das = (DelegateAbility_Struct*)outapp->pBuffer;

		das->DelegateAbility = 0;

		das->MemberNumber = 0;

		das->Action = 0;

		das->EntityID = 0;

		strn0cpy(das->Name, mMainAssistName.c_str(), sizeof(das->Name));

		c->QueuePacket(outapp);

		safe_delete(outapp);
	}
	else
	{
		EQApplicationPacket *outapp = new EQApplicationPacket(OP_GroupRoles, sizeof(GroupRole_Struct));

		GroupRole_Struct *grs = (GroupRole_Struct*)outapp->pBuffer;

		strn0cpy(grs->Name1, mMainAssistName.c_str(), sizeof(grs->Name1));

		strn0cpy(grs->Name2, getLeaderName(), sizeof(grs->Name2));

		grs->RoleNumber = 2;

		grs->Toggle = toggle;

		c->QueuePacket(outapp);

		safe_delete(outapp);
	}

	notifyAssistTarget(c);

}

void Group::notifyPuller(Client *c, uint8 toggle)
{
	// Send a packet to the specified Client notifying them who the new Puller is. This causes the client to display
	// a message with the name of the Puller.
	//

	if (!c)
		return;

	if (!mPullerName.size())
		return;

	if (c->GetClientVersion() < EQClientSoD)
	{
		if (toggle)
			c->Message(0, "%s is now Puller.", mPullerName.c_str());
		else
			c->Message(0, "%s is no longer Puller.", mPullerName.c_str());
	}
	else
	{
		EQApplicationPacket *outapp = new EQApplicationPacket(OP_GroupRoles, sizeof(GroupRole_Struct));

		GroupRole_Struct *grs = (GroupRole_Struct*)outapp->pBuffer;

		strn0cpy(grs->Name1, mPullerName.c_str(), sizeof(grs->Name1));

		strn0cpy(grs->Name2, getLeaderName(), sizeof(grs->Name2));

		grs->RoleNumber = 3;

		grs->Toggle = toggle;

		c->QueuePacket(outapp);

		safe_delete(outapp);
	}

}

void Group::undelegateMainTank(const char *OldMainTankName, uint8 toggle)
{
	// Called when the group Leader removes the Main Tank delegation. Sends a packet to each group member in the zone
	// informing them of the change and update the group_leaders table.
	//
	if (OldMainTankName == mMainTankName) {
		char errbuff[MYSQL_ERRMSG_SIZE];

		char *Query = 0;

		if (!database.RunQuery(Query, MakeAnyLenString(&Query, "UPDATE group_leaders SET maintank='' WHERE gid=%i LIMIT 1",
			GetID()), errbuff))
			LogFile->write(EQEMuLog::Error, "Unable to clear group main tank: %s\n", errbuff);

		safe_delete_array(Query);

		if (!toggle) {
			for (uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i) {
				if (mMembers[i] && mMembers[i]->IsClient())
				{
					notifyMainTank(mMembers[i]->CastToClient(), toggle);
					mMembers[i]->CastToClient()->UpdateXTargetType(GroupTank, nullptr, "");
					mMembers[i]->CastToClient()->UpdateXTargetType(GroupTankTarget, nullptr);
				}
			}
		}

		setMainTank("");
	}
}

void Group::undelegateMainAssist(const char *OldMainAssistName, uint8 toggle)
{
	// Called when the group Leader removes the Main Assist delegation. Sends a packet to each group member in the zone
	// informing them of the change and update the group_leaders table.
	//
	if (OldMainAssistName == mMainAssistName) {
		EQApplicationPacket *outapp = new EQApplicationPacket(OP_DelegateAbility, sizeof(DelegateAbility_Struct));

		DelegateAbility_Struct* das = (DelegateAbility_Struct*)outapp->pBuffer;

		das->DelegateAbility = 0;

		das->MemberNumber = 0;

		das->Action = 1;

		das->EntityID = 0;

		strn0cpy(das->Name, OldMainAssistName, sizeof(das->Name));

		for (uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i)
		if (mMembers[i] && mMembers[i]->IsClient())
		{
			mMembers[i]->CastToClient()->QueuePacket(outapp);
			mMembers[i]->CastToClient()->UpdateXTargetType(GroupAssist, nullptr, "");
		}

		safe_delete(outapp);

		char errbuff[MYSQL_ERRMSG_SIZE];

		char *Query = 0;

		if (!database.RunQuery(Query, MakeAnyLenString(&Query, "UPDATE group_leaders SET assist='' WHERE gid=%i LIMIT 1",
			GetID()), errbuff))
			LogFile->write(EQEMuLog::Error, "Unable to clear group main assist: %s\n", errbuff);

		safe_delete_array(Query);

		if (!toggle)
		{
			for (uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i)
			{
				if (mMembers[i] && mMembers[i]->IsClient())
				{
					notifyMainAssist(mMembers[i]->CastToClient(), toggle);
					mMembers[i]->CastToClient()->UpdateXTargetType(GroupAssistTarget, nullptr);
				}
			}
		}

		setMainAssist("");
	}
}

void Group::undelegatePuller(const char *OldPullerName, uint8 toggle)
{
	// Called when the group Leader removes the Puller delegation. Sends a packet to each group member in the zone
	// informing them of the change and update the group_leaders table.
	//
	if (OldPullerName == mPullerName) {
		char errbuff[MYSQL_ERRMSG_SIZE];

		char *Query = 0;

		if (!database.RunQuery(Query, MakeAnyLenString(&Query, "UPDATE group_leaders SET puller='' WHERE gid=%i LIMIT 1",
			GetID()), errbuff))
			LogFile->write(EQEMuLog::Error, "Unable to clear group main puller: %s\n", errbuff);

		safe_delete_array(Query);

		if (!toggle) {
			for (uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i) {
				if (mMembers[i] && mMembers[i]->IsClient())
				{
					notifyPuller(mMembers[i]->CastToClient(), toggle);
					mMembers[i]->CastToClient()->UpdateXTargetType(Puller, nullptr, "");
					mMembers[i]->CastToClient()->UpdateXTargetType(PullerTarget, nullptr);
				}
			}
		}

		setPuller("");
	}
}

bool Group::isNPCMarker(Client *c)
{
	// Returns true if the specified client has been delegated the NPC Marker Role
	//
	if (!c)
		return false;

	if (mNPCMarkerName.size())
		return(c->GetName() == mNPCMarkerName);

	return false;

}

void Group::setGroupAssistTarget(Mob *m)
{
	// Notify all group members in the zone of the new target the Main Assist has selected.
	//
	mAssistTargetID = m ? m->GetID() : 0;

	for (uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i)
	{
		if (mMembers[i] && mMembers[i]->IsClient())
		{
			notifyAssistTarget(mMembers[i]->CastToClient());
		}
	}
}

void Group::setGroupTankTarget(Mob *m)
{
	mTankTargetID = m ? m->GetID() : 0;

	for (uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i)
	{
		if (mMembers[i] && mMembers[i]->IsClient())
		{
			mMembers[i]->CastToClient()->UpdateXTargetType(GroupTankTarget, m);
		}
	}
}

void Group::setGroupPullerTarget(Mob *m)
{
	mPullerTargetID = m ? m->GetID() : 0;

	for (uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i)
	{
		if (mMembers[i] && mMembers[i]->IsClient())
		{
			mMembers[i]->CastToClient()->UpdateXTargetType(PullerTarget, m);
		}
	}
}

void Group::notifyAssistTarget(Client *c)
{
	// Send a packet to the specified client notifying them of the group target selected by the Main Assist.

	if (!c)
		return;

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_SetGroupTarget, sizeof(MarkNPC_Struct));

	MarkNPC_Struct* mnpcs = (MarkNPC_Struct *)outapp->pBuffer;

	mnpcs->TargetID = mAssistTargetID;

	mnpcs->Number = 0;

	c->QueuePacket(outapp);

	safe_delete(outapp);

	Mob *m = entity_list.GetMob(mAssistTargetID);

	c->UpdateXTargetType(GroupAssistTarget, m);

}

void Group::notifyTankTarget(Client *c)
{
	if (!c)
		return;

	Mob *m = entity_list.GetMob(mTankTargetID);

	c->UpdateXTargetType(GroupTankTarget, m);
}

void Group::notifyPullerTarget(Client *c)
{
	if (!c)
		return;

	Mob *m = entity_list.GetMob(mPullerTargetID);

	c->UpdateXTargetType(PullerTarget, m);
}

void Group::delegateMarkNPC(const char *NewNPCMarkerName)
{
	// Called when the group leader has delegated the Mark NPC ability to a group member.
	// Notify all group members in the zone of the change and save the change in the group_leaders
	// table to persist across zones.
	//
	if (mNPCMarkerName.size() > 0)
		unDelegateMarkNPC(mNPCMarkerName.c_str());

	if (!NewNPCMarkerName)
		return;

	setNPCMarker(NewNPCMarkerName);

	for (uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i)
	if (mMembers[i] && mMembers[i]->IsClient())
		notifyMarkNPC(mMembers[i]->CastToClient());

	char errbuff[MYSQL_ERRMSG_SIZE];

	char *Query = 0;

	if (!database.RunQuery(Query, MakeAnyLenString(&Query, "UPDATE group_leaders SET marknpc='%s' WHERE gid=%i LIMIT 1",
		NewNPCMarkerName, GetID()), errbuff))
		LogFile->write(EQEMuLog::Error, "Unable to set group mark npc: %s\n", errbuff);

	safe_delete_array(Query);

}

void Group::notifyMarkNPC(Client *c)
{
	// Notify the specified client who the group member is who has been delgated the Mark NPC ability.

	if (!c)
		return;

	if (!mNPCMarkerName.size())
		return;

	EQApplicationPacket *outapp = new EQApplicationPacket(OP_DelegateAbility, sizeof(DelegateAbility_Struct));

	DelegateAbility_Struct* das = (DelegateAbility_Struct*)outapp->pBuffer;

	das->DelegateAbility = 1;

	das->MemberNumber = 0;

	das->Action = 0;

	das->EntityID = mNPCMarkerID;

	strn0cpy(das->Name, mNPCMarkerName.c_str(), sizeof(das->Name));

	c->QueuePacket(outapp);

	safe_delete(outapp);

}
void Group::setNPCMarker(const char *NewNPCMarkerName)
{
	mNPCMarkerName = NewNPCMarkerName;

	Client *m = entity_list.GetClientByName(mNPCMarkerName.c_str());

	if (!m)
		mNPCMarkerID = 0;
	else
		mNPCMarkerID = m->GetID();
}

void Group::unDelegateMarkNPC(const char *OldNPCMarkerName)
{
	// Notify all group members in the zone that the Mark NPC ability has been rescinded from the specified
	// group member.

	if (!OldNPCMarkerName)
		return;

	if (!mNPCMarkerName.size())
		return;

	EQApplicationPacket *outapp = new EQApplicationPacket(OP_DelegateAbility, sizeof(DelegateAbility_Struct));

	DelegateAbility_Struct* das = (DelegateAbility_Struct*)outapp->pBuffer;

	das->DelegateAbility = 1;

	das->MemberNumber = 0;

	das->Action = 1;

	das->EntityID = 0;

	strn0cpy(das->Name, OldNPCMarkerName, sizeof(das->Name));

	for (uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i)
	if (mMembers[i] && mMembers[i]->IsClient())
		mMembers[i]->CastToClient()->QueuePacket(outapp);

	safe_delete(outapp);

	mNPCMarkerName.clear();

	char errbuff[MYSQL_ERRMSG_SIZE];

	char *Query = 0;

	if (!database.RunQuery(Query, MakeAnyLenString(&Query, "UPDATE group_leaders SET marknpc='' WHERE gid=%i LIMIT 1",
		GetID()), errbuff))
		LogFile->write(EQEMuLog::Error, "Unable to clear group marknpc: %s\n", errbuff);

	safe_delete_array(Query);
}

void Group::saveGroupLeaderAA()
{
	// Stores the Group Leaders Leadership AA data from the Player Profile as a blob in the group_leaders table.
	// This is done so that group members not in the same zone as the Leader still have access to this information.

	char *Query = new char[200 + sizeof(GroupLeadershipAA_Struct)* 2];

	char *End = Query;

	End += sprintf(End, "UPDATE group_leaders SET leadershipaa='");

	End += database.DoEscapeString(End, (char*)&mLeaderAbilities, sizeof(GroupLeadershipAA_Struct));

	End += sprintf(End, "' WHERE gid=%i LIMIT 1", GetID());

	char errbuff[MYSQL_ERRMSG_SIZE];
	if (!database.RunQuery(Query, End - Query, errbuff))
		LogFile->write(EQEMuLog::Error, "Unable to store LeadershipAA: %s\n", errbuff);

	safe_delete_array(Query);
}

void Group::unMarkNPC(uint16 ID)
{
	// Called from entity_list when the mob with the specified ID is being destroyed.
	//
	// If the given mob has been marked by this group, it is removed from the list of marked NPCs.
	// The primary reason for doing this is so that when a new group member joins or zones in, we
	// send them correct details of which NPCs are currently marked.

	if (mAssistTargetID == ID)
		mAssistTargetID = 0;


	if (mTankTargetID == ID)
		mTankTargetID = 0;

	if (mPullerTargetID == ID)
		mPullerTargetID = 0;

	for (int i = 0; i < MAX_MARKED_NPCS; ++i)
	{
		if (mMarkedNPCs[i] == ID)
		{
			mMarkedNPCs[i] = 0;
			updateXTargetMarkedNPC(i + 1, nullptr);
		}
	}
}

void Group::sendMarkedNPCsToMember(Client *c, bool Clear)
{
	// Send the Entity IDs of the NPCs marked by the Group Leader or delegate to the specified client.
	// If Clear == true, then tell the client to unmark the NPCs (when a member disbands).
	//
	//
	if (!c)
		return;

	EQApplicationPacket *outapp = new EQApplicationPacket(OP_MarkNPC, sizeof(MarkNPC_Struct));

	MarkNPC_Struct *mnpcs = (MarkNPC_Struct *)outapp->pBuffer;

	for (int i = 0; i < MAX_MARKED_NPCS; ++i)
	{
		if (mMarkedNPCs[i])
		{
			mnpcs->TargetID = mMarkedNPCs[i];

			Mob *m = entity_list.GetMob(mMarkedNPCs[i]);

			if (m)
				sprintf(mnpcs->Name, "%s", m->GetCleanName());

			if (!Clear)
				mnpcs->Number = i + 1;
			else
				mnpcs->Number = 0;

			c->QueuePacket(outapp);
			c->UpdateXTargetType((mnpcs->Number == 1) ? GroupMarkTarget1 : ((mnpcs->Number == 2) ? GroupMarkTarget2 : GroupMarkTarget3), m);
		}
	}

	safe_delete(outapp);
}

void Group::clearAllNPCMarks()
{
	// This method is designed to be called when the number of members in the group drops below 3 and leadership AA
	// may no longer be used. It removes all NPC marks.
	//
	for (uint8 i = 0; i < MAX_GROUP_MEMBERS; ++i)
	if (mMembers[i] && mMembers[i]->IsClient())
		sendMarkedNPCsToMember(mMembers[i]->CastToClient(), true);

	for (int i = 0; i < MAX_MARKED_NPCS; ++i)
	{
		if (mMarkedNPCs[i])
		{
			Mob* m = entity_list.GetMob(mMarkedNPCs[i]);

			if (m)
				m->IsTargeted(-1);
		}

		mMarkedNPCs[i] = 0;
	}

}

int8 Group::getNumberNeedingHealedInGroup(int8 hpr, bool includePets) {
	int8 needHealed = 0;

	for (int i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (mMembers[i] && !mMembers[i]->qglobal) {

			if (mMembers[i]->GetHPRatio() <= hpr)
				needHealed++;

			if (includePets) {
				if (mMembers[i]->GetPet() && mMembers[i]->GetPet()->GetHPRatio() <= hpr) {
					needHealed++;
				}
			}
		}
	}


	return needHealed;
}

void Group::updateGroupAAs()
{
	// This method updates the Groups Leadership abilities from the Player Profile of the Leader.
	//
	Mob *m = getLeader();

	if (m && m->IsClient())
		m->CastToClient()->GetGroupAAs(&mLeaderAbilities);
	else
		memset(&mLeaderAbilities, 0, sizeof(GroupLeadershipAA_Struct));

	saveGroupLeaderAA();
}

void Group::queueHPPacketsForNPCHealthAA(Mob* sender, const EQApplicationPacket* app)
{
	// Send a mobs HP packets to group members if the leader has the NPC Health AA and the mob is the
	// target of the group's main assist, or is marked, and the member doesn't already have the mob targeted.

	if (!sender || !app || !getLeadershipAA(groupAANPCHealth))
		return;

	uint16 SenderID = sender->GetID();

	if (SenderID != mAssistTargetID)
	{
		bool Marked = false;

		for (int i = 0; i < MAX_MARKED_NPCS; ++i)
		{
			if (mMarkedNPCs[i] == SenderID)
			{
				Marked = true;
				break;
			}
		}

		if (!Marked)
			return;

	}

	for (unsigned int i = 0; i < MAX_GROUP_MEMBERS; ++i)
	if (mMembers[i] && mMembers[i]->IsClient())
	{
		if (!mMembers[i]->GetTarget() || (mMembers[i]->GetTarget()->GetID() != SenderID))
		{
			mMembers[i]->CastToClient()->QueuePacket(app);
		}
	}

}

void Group::changeLeader(Mob* newleader)
{
	// this changes the current group leader, notifies other members, and updates leadship AA

	// if the new leader is invalid, do nothing
	if (!newleader)
		return;

	Mob* oldleader = getLeader();

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupJoin_Struct));
	GroupJoin_Struct* gu = (GroupJoin_Struct*)outapp->pBuffer;
	gu->action = groupActMakeLeader;

	strcpy(gu->membername, newleader->GetName());
	strcpy(gu->yourname, oldleader->GetName());
	setLeader(newleader);
	database.SetGroupLeaderName(GetID(), newleader->GetName());
	updateGroupAAs();
	gu->leader_aas = mLeaderAbilities;
	for (uint32 i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (mMembers[i] && mMembers[i]->IsClient())
		{
			if (mMembers[i]->CastToClient()->GetClientVersion() >= EQClientSoD)
				mMembers[i]->CastToClient()->SendGroupLeaderChangePacket(newleader->GetName());

			mMembers[i]->CastToClient()->QueuePacket(outapp);
		}
	}
	safe_delete(outapp);
}

const char* Group::getClientNameByIndex(uint8 pIndex)
{
	return mMemberNames[pIndex];
}

void Group::updateXTargetMarkedNPC(uint32 pNumber, Mob* pMOB)
{
	for (uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i)
	{
		if (mMembers[i] && mMembers[i]->IsClient())
		{
			mMembers[i]->CastToClient()->UpdateXTargetType((pNumber == 1) ? GroupMarkTarget1 : ((pNumber == 2) ? GroupMarkTarget2 : GroupMarkTarget3), pMOB);
		}
	}

}

void Group::setMainTank(const char *NewMainTankName)
{
	mMainTankName = NewMainTankName;

	for (uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i)
	{
		if (!strncasecmp(mMemberNames[i], NewMainTankName, 64))
			mMemberRoles[i] |= RoleTank;
		else
			mMemberRoles[i] &= ~RoleTank;
	}
}

void Group::setMainAssist(const char *NewMainAssistName)
{
	mMainAssistName = NewMainAssistName;

	for (uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i)
	{
		if (!strncasecmp(mMemberNames[i], NewMainAssistName, 64))
			mMemberRoles[i] |= RoleAssist;
		else
			mMemberRoles[i] &= ~RoleAssist;
	}
}

void Group::setPuller(const char *NewPullerName)
{
	mPullerName = NewPullerName;

	for (uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i)
	{
		if (!strncasecmp(mMemberNames[i], NewPullerName, 64))
			mMemberRoles[i] |= RolePuller;
		else
			mMemberRoles[i] &= ~RolePuller;
	}
}

bool Group::hasRole(Mob *m, uint8 Role)
{
	if (!m)
		return false;

	for (uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i)
	{
		if ((m == mMembers[i]) && (mMemberRoles[i] & Role))
			return true;
	}
	return false;
}

