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
Group::Group(uint32 pGroupID)
: GroupIDConsumer(pGroupID)
{
	mLeader = nullptr;
	memset(mMembers, 0, sizeof(Mob*)* MAX_GROUP_MEMBERS);
	mAssistTargetID = 0;
	mTankTargetID = 0;
	mPullerTargetID = 0;

	memset(&mLeaderAbilities, 0, sizeof(GroupLeadershipAA_Struct));

	for (int i = 0; i < MAX_GROUP_MEMBERS; i++)
	{
		memset(mMemberNames[i], 0, 64);
		mMemberRoles[i] = 0;
	}

	if (pGroupID != 0) {
		if (!learnMembers())
			SetID(0);
	}
	for (int i = 0; i < MAX_MARKED_NPCS; ++i)
		mMarkedNPCs[i] = 0;

	mNPCMarkerID = 0;
}

//creating a new group
Group::Group(Mob* pLeader)
: GroupIDConsumer()
{
	memset(mMembers, 0, sizeof(mMembers));
	mMembers[0] = pLeader;
	pLeader->SetGrouped(true);
	setLeader(pLeader);
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
	strcpy(mMemberNames[0], pLeader->getName());

	if (pLeader->isClient())
		strcpy(pLeader->castToClient()->GetPP().groupMembers[0], pLeader->getName());

	for (int i = 0; i < MAX_MARKED_NPCS; ++i)
		mMarkedNPCs[i] = 0;

	mNPCMarkerID = 0;
}

Group::~Group()
{
	for (int i = 0; i < MAX_MARKED_NPCS; ++i)
	if (mMarkedNPCs[i])
	{
		Mob* m = entity_list.getMOB(mMarkedNPCs[i]);
		if (m)
			m->IsTargeted(-1);
	}
}

// Split money used in OP_Split.
void Group::splitMoney(uint32 pCopper, uint32 pSilver, uint32 pGold, uint32 pPlatinum, Client* pSplitter) {
	//avoid unneeded work
	if (pCopper == 0 && pSilver == 0 && pGold == 0 && pPlatinum == 0)
		return;

	int membercount = 0;
	for (int i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (mMembers[i] != nullptr) {
			membercount++;
		}
	}

	if (membercount == 0)
		return;

	uint32 mod;
	//try to handle round off error a little better
	if (membercount > 1) {
		mod = pPlatinum % membercount;
		if ((mod) > 0) {
			pPlatinum -= mod;
			pGold += 10 * mod;
		}
		mod = pGold % membercount;
		if ((mod) > 0) {
			pGold -= mod;
			pSilver += 10 * mod;
		}
		mod = pSilver % membercount;
		if ((mod) > 0) {
			pSilver -= mod;
			pCopper += 10 * mod;
		}
	}

	//calculate the splits
	//We can still round off copper pieces, but I dont care
	uint32 sc;
	uint32 cpsplit = pCopper / membercount;
	sc = pCopper % membercount;
	uint32 spsplit = pSilver / membercount;
	uint32 gpsplit = pGold / membercount;
	uint32 ppsplit = pPlatinum / membercount;

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

	for (int i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (mMembers[i] != nullptr && mMembers[i]->isClient()) { // If Group Member is Client
			Client *c = mMembers[i]->castToClient();
			//I could not get MoneyOnCorpse to work, so we use this
			c->AddMoneyToPP(cpsplit, spsplit, gpsplit, ppsplit, true);

			c->Message(2, msg.c_str());
		}
	}
}

bool Group::addMember(Mob* pNewMember, const char *pNewMemberName, uint32 pCharacterID)
{
	bool InZone = true;
	bool ismerc = false;

	// This method should either be passed a Mob*, if the new member is in this zone, or a nullptr Mob*
	// and the name and CharacterID of the new member, if they are out of zone.
	//
	if (!pNewMember && !pNewMemberName)
		return false;

	if (groupCount() >= MAX_GROUP_MEMBERS) //Sanity check for merging groups together.
		return false;

	if (!pNewMember)
		InZone = false;
	else
	{
		pNewMemberName = pNewMember->GetCleanName();

		if (pNewMember->isClient())
			pCharacterID = pNewMember->castToClient()->CharacterID();
		if (pNewMember->isMerc())
		{
			Client* owner = pNewMember->castToMerc()->GetMercOwner();
			if (owner)
			{
				pCharacterID = owner->castToClient()->CharacterID();
				pNewMemberName = pNewMember->getName();
				ismerc = true;
			}
		}
	}

	uint32 i = 0;

	// See if they are already in the group
	//
	for (i = 0; i < MAX_GROUP_MEMBERS; ++i)
	if (!strcasecmp(mMemberNames[i], pNewMemberName))
		return false;

	// Put them in the group
	for (i = 0; i < MAX_GROUP_MEMBERS; ++i)
	{
		if (mMemberNames[i][0] == '\0')
		{
			if (InZone)
				mMembers[i] = pNewMember;

			break;
		}
	}

	if (i == MAX_GROUP_MEMBERS)
		return false;

	strcpy(mMemberNames[i], pNewMemberName);
	mMemberRoles[i] = 0;

	int x = 1;

	//build the template join packet
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupJoin_Struct));
	GroupJoin_Struct* gj = (GroupJoin_Struct*)outapp->pBuffer;
	strcpy(gj->membername, pNewMemberName);
	gj->action = groupActJoin;

	gj->leader_aas = mLeaderAbilities;

	for (i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (mMembers[i] != nullptr && mMembers[i] != pNewMember) {
			//fill in group join & send it
			if (mMembers[i]->isMerc())
			{
				strcpy(gj->yourname, mMembers[i]->getName());
			}
			else
			{
				strcpy(gj->yourname, mMembers[i]->GetCleanName());
			}
			if (mMembers[i]->isClient()) {
				mMembers[i]->castToClient()->QueuePacket(outapp);

				//put new member into existing person's list
				strcpy(mMembers[i]->castToClient()->GetPP().groupMembers[this->groupCount() - 1], pNewMemberName);
			}

			//put this existing person into the new member's list
			if (InZone && pNewMember->isClient()) {
				if (isLeader(mMembers[i]))
					strcpy(pNewMember->castToClient()->GetPP().groupMembers[0], mMembers[i]->GetCleanName());
				else {
					strcpy(pNewMember->castToClient()->GetPP().groupMembers[x], mMembers[i]->GetCleanName());
					x++;
				}
			}
		}
	}

	if (InZone)
	{
		//put new member in his own list.
		pNewMember->SetGrouped(true);

		if (pNewMember->isClient())
		{
			strcpy(pNewMember->castToClient()->GetPP().groupMembers[x], pNewMemberName);
			pNewMember->castToClient()->save();
			database.SetGroupID(pNewMemberName, GetID(), pNewMember->castToClient()->CharacterID(), false);
			sendMarkedNPCsToMember(pNewMember->castToClient());

			notifyMainTank(pNewMember->castToClient(), 1);
			notifyMainAssist(pNewMember->castToClient(), 1);
			notifyPuller(pNewMember->castToClient(), 1);
		}

		if (pNewMember->isMerc())
		{
			Client* owner = pNewMember->castToMerc()->GetMercOwner();
			if (owner)
			{
				database.SetGroupID(pNewMember->getName(), GetID(), owner->CharacterID(), true);
			}
		}
	}
	else
		database.SetGroupID(pNewMemberName, GetID(), pCharacterID, ismerc);

	safe_delete(outapp);

	return true;
}

void Group::addMember(const char* pNewMemberName)
{
	// This method should be called when both the new member and the group leader are in a different zone to this one.
	//
	for (int i = 0; i < MAX_GROUP_MEMBERS; ++i)
	if (!strcasecmp(mMemberNames[i], pNewMemberName))
	{
		return;
	}

	for (int i = 0; i < MAX_GROUP_MEMBERS; ++i)
	{
		if (mMemberNames[i][0] == '\0')
		{
			strcpy(mMemberNames[i], pNewMemberName);
			mMemberRoles[i] = 0;
			break;
		}
	}
}


void Group::queuePacket(const EQApplicationPacket* pApp, bool pAckReq)
{
	for (int i = 0; i < MAX_GROUP_MEMBERS; i++)
	if (mMembers[i] && mMembers[i]->isClient())
		mMembers[i]->castToClient()->QueuePacket(pApp, pAckReq);
}

// solar: sends the rest of the group's hps to member. this is useful when
// someone first joins a group, but otherwise there shouldn't be a need to
// call it
void Group::sendHPPacketsTo(Mob* pNewMember)
{
	if (pNewMember && pNewMember->isClient())
	{
		EQApplicationPacket hpapp;
		EQApplicationPacket outapp(OP_MobManaUpdate, sizeof(MobManaUpdate_Struct));

		for (int i = 0; i < MAX_GROUP_MEMBERS; i++)
		{
			if (mMembers[i] && mMembers[i] != pNewMember)
			{
				mMembers[i]->CreateHPPacket(&hpapp);
				pNewMember->castToClient()->QueuePacket(&hpapp, false);
				if (pNewMember->castToClient()->GetClientVersion() >= EQClientSoD)
				{
					outapp.SetOpcode(OP_MobManaUpdate);
					MobManaUpdate_Struct *mmus = (MobManaUpdate_Struct *)outapp.pBuffer;
					mmus->spawn_id = mMembers[i]->getID();
					mmus->mana = mMembers[i]->GetManaPercent();
					pNewMember->castToClient()->QueuePacket(&outapp, false);
					MobEnduranceUpdate_Struct *meus = (MobEnduranceUpdate_Struct *)outapp.pBuffer;
					outapp.SetOpcode(OP_MobEnduranceUpdate);
					meus->endurance = mMembers[i]->GetEndurancePercent();
					pNewMember->castToClient()->QueuePacket(&outapp, false);
				}
			}
		}
	}
}

void Group::sendHPPacketsFrom(Mob *pNewMember)
{
	EQApplicationPacket hp_app;
	if (!pNewMember)
		return;

	pNewMember->CreateHPPacket(&hp_app);
	EQApplicationPacket outapp(OP_MobManaUpdate, sizeof(MobManaUpdate_Struct));

	for (int i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (mMembers[i] && mMembers[i] != pNewMember && mMembers[i]->isClient())
		{
			mMembers[i]->castToClient()->QueuePacket(&hp_app);
			if (mMembers[i]->castToClient()->GetClientVersion() >= EQClientSoD)
			{
				outapp.SetOpcode(OP_MobManaUpdate);
				MobManaUpdate_Struct *mmus = (MobManaUpdate_Struct *)outapp.pBuffer;
				mmus->spawn_id = pNewMember->getID();
				mmus->mana = pNewMember->GetManaPercent();
				mMembers[i]->castToClient()->QueuePacket(&outapp, false);
				MobEnduranceUpdate_Struct *meus = (MobEnduranceUpdate_Struct *)outapp.pBuffer;
				outapp.SetOpcode(OP_MobEnduranceUpdate);
				meus->endurance = pNewMember->GetEndurancePercent();
				mMembers[i]->castToClient()->QueuePacket(&outapp, false);
			}
		}
	}
}

//updates a group member's client pointer when they zone in
//if the group was in the zone allready
bool Group::updatePlayer(Mob* pUpdate){

	verifyGroup();

	if (pUpdate->isClient()) {
		//update their player profile
		PlayerProfile_Struct &pp = pUpdate->castToClient()->GetPP();
		for (int i = 0; i < MAX_GROUP_MEMBERS; i++) {
			if (mMemberNames[0] == '\0')
				memset(pp.groupMembers[i], 0, 64);
			else
				strn0cpy(pp.groupMembers[i], mMemberNames[i], 64);
		}
		if (isNPCMarker(pUpdate->castToClient()))
		{
			mNPCMarkerID = pUpdate->getID();
			sendLeadershipAAUpdate();
		}
	}

	for (int i = 0; i < MAX_GROUP_MEMBERS; i++)
	{
		if (!strcasecmp(mMemberNames[i], pUpdate->getName()))
		{
			mMembers[i] = pUpdate;
			mMembers[i]->SetGrouped(true);
			return true;
		}
	}
	return false;
}


void Group::memberZoned(Mob* pRemove) {
	if (pRemove == nullptr)
		return;

	if (pRemove == getLeader())
		setLeader(nullptr);

	for (int i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (mMembers[i] == pRemove) {
			mMembers[i] = nullptr;
			//should NOT clear the name, it is used for world communication.
			break;
		}
	}
	if (pRemove->isClient() && hasRole(pRemove, RoleAssist))
		setGroupAssistTarget(0);

	if (pRemove->isClient() && hasRole(pRemove, RoleTank))
		setGroupTankTarget(0);

	if (pRemove->isClient() && hasRole(pRemove, RolePuller))
		setGroupPullerTarget(0);
}

bool Group::delMemberOOZ(const char* pName) {

	if (!pName) return false;

	// If a member out of zone has disbanded, clear out their name.
	//
	for (int i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (!strcasecmp(pName, mMemberNames[i]))
			// This shouldn't be called if the member is in this zone.
		if (!mMembers[i]) {
			if (!strncmp(getLeaderName(), pName, 64))
			{
				//TODO: Transfer leadership if leader disbands OOZ.
				updateGroupAAs();
			}

			memset(mMemberNames[i], 0, 64);
			mMemberRoles[i] = 0;
			if (groupCount() < 3)
			{
				undelegateMarkNPC(mNPCMarkerName.c_str());
				if (getLeader() && getLeader()->isClient() && getLeader()->castToClient()->GetClientVersion() < EQClientSoD) {
					undelegateMainAssist(mMainAssistName.c_str());
				}
				clearAllNPCMarks();
			}
			return true;
		}
	}

	return false;
}

bool Group::delMember(Mob* pOldMember, bool pIgnoreSender)
{
	if (pOldMember == nullptr){
		return false;
	}

	for (int i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (mMembers[i] == pOldMember) {
			mMembers[i] = nullptr;
			mMemberNames[i][0] = '\0';
			memset(mMemberNames[i], 0, 64);
			mMemberRoles[i] = 0;
			break;
		}
	}

	//handle leader quitting group gracefully
	if (pOldMember == getLeader() && groupCount() >= 2) {
		for (int i = 0; i < MAX_GROUP_MEMBERS; i++) {
			if (mMembers[i]) {
				changeLeader(mMembers[i]);
				break;
			}
		}
	}

	ServerPacket* pack = new ServerPacket(ServerOP_GroupLeave, sizeof(ServerGroupLeave_Struct));
	ServerGroupLeave_Struct* gl = (ServerGroupLeave_Struct*)pack->pBuffer;
	gl->gid = GetID();
	gl->zoneid = zone->GetZoneID();
	gl->instance_id = zone->GetInstanceID();
	strcpy(gl->member_name, pOldMember->getName());
	worldserver.SendPacket(pack);
	safe_delete(pack);

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupJoin_Struct));
	GroupJoin_Struct* gu = (GroupJoin_Struct*)outapp->pBuffer;
	gu->action = groupActLeave;
	strcpy(gu->membername, pOldMember->GetCleanName());
	strcpy(gu->yourname, pOldMember->GetCleanName());

	gu->leader_aas = mLeaderAbilities;

	for (int i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (mMembers[i] == nullptr) {
			//if (DEBUG>=5) LogFile->write(EQEMuLog::Debug, "Group::DelMember() null member at slot %i", i);
			continue;
		}
		if (mMembers[i] != pOldMember) {
			strcpy(gu->yourname, mMembers[i]->GetCleanName());
			if (mMembers[i]->isClient())
				mMembers[i]->castToClient()->QueuePacket(outapp);
		}
	}

	if (!pIgnoreSender) {
		strcpy(gu->yourname, pOldMember->GetCleanName());
		strcpy(gu->membername, pOldMember->GetCleanName());
		gu->action = groupActLeave;

		if (pOldMember->isClient())
			pOldMember->castToClient()->QueuePacket(outapp);
	}

	if (pOldMember->isClient())
		database.SetGroupID(pOldMember->GetCleanName(), 0, pOldMember->castToClient()->CharacterID());

	pOldMember->SetGrouped(false);
	mDisbandCheck = true;

	safe_delete(outapp);

	if (hasRole(pOldMember, RoleTank))
	{
		setGroupTankTarget(0);
		undelegateMainTank(pOldMember->getName());
	}

	if (hasRole(pOldMember, RoleAssist))
	{
		setGroupAssistTarget(0);
		undelegateMainAssist(pOldMember->getName());
	}

	if (hasRole(pOldMember, RolePuller))
	{
		setGroupPullerTarget(0);
		undelegatePuller(pOldMember->getName());
	}

	if (pOldMember->isClient())
		sendMarkedNPCsToMember(pOldMember->castToClient(), true);

	if (groupCount() < 3)
	{
		undelegateMarkNPC(mNPCMarkerName.c_str());
		if (getLeader() && getLeader()->isClient() && getLeader()->castToClient()->GetClientVersion() < EQClientSoD) {
			undelegateMainAssist(mMainAssistName.c_str());
		}
		clearAllNPCMarks();
	}

	return true;
}

// does the caster + group
void Group::castGroupSpell(Mob* pCaster, uint16 pSpellID) {
	float range, distance;

	if (!pCaster)
		return;

	mCastSpell = true;
	range = pCaster->GetAOERange(pSpellID);

	float range2 = range*range;

	//	caster->SpellOnTarget(spell_id, caster);

	for (int i = 0; i < MAX_GROUP_MEMBERS; i++)
	{
		if (mMembers[i] == pCaster) {
			pCaster->SpellOnTarget(pSpellID, pCaster);
#ifdef GROUP_BUFF_PETS
			if (pCaster->GetPet() && pCaster->HasPetAffinity() && !pCaster->GetPet()->IsCharmed())
				pCaster->SpellOnTarget(pSpellID, pCaster->GetPet());
#endif
		}
		else if (mMembers[i] != nullptr)
		{
			distance = pCaster->DistNoRoot(*mMembers[i]);
			if (distance <= range2) {
				pCaster->SpellOnTarget(pSpellID, mMembers[i]);
#ifdef GROUP_BUFF_PETS
				if (mMembers[i]->GetPet() && mMembers[i]->HasPetAffinity() && !mMembers[i]->GetPet()->IsCharmed())
					pCaster->SpellOnTarget(pSpellID, mMembers[i]->GetPet());
#endif
			}
			else
				_log(SPELLS__CASTING, "Group spell: %s is out of range %f at distance %f from %s", mMembers[i]->getName(), range, distance, pCaster->getName());
		}
	}

	mCastSpell = false;
	mDisbandCheck = true;
}

// does the caster + group
void Group::groupBardPulse(Mob* pCaster, uint16 pSpellID) {
	float range, distance;

	if (!pCaster)
		return;

	mCastSpell = true;
	range = pCaster->GetAOERange(pSpellID);

	float range2 = range*range;

	for (int i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (mMembers[i] == pCaster) {
			pCaster->BardPulse(pSpellID, pCaster);
#ifdef GROUP_BUFF_PETS
			if (pCaster->GetPet() && pCaster->HasPetAffinity() && !pCaster->GetPet()->IsCharmed())
				pCaster->BardPulse(pSpellID, pCaster->GetPet());
#endif
		}
		else if (mMembers[i] != nullptr)
		{
			distance = pCaster->DistNoRoot(*mMembers[i]);
			if (distance <= range2) {
				mMembers[i]->BardPulse(pSpellID, pCaster);
#ifdef GROUP_BUFF_PETS
				if (mMembers[i]->GetPet() && mMembers[i]->HasPetAffinity() && !mMembers[i]->GetPet()->IsCharmed())
					mMembers[i]->GetPet()->BardPulse(pSpellID, pCaster);
#endif
			}
			else
				_log(SPELLS__BARDS, "Group bard pulse: %s is out of range %f at distance %f from %s", mMembers[i]->getName(), range, distance, pCaster->getName());
		}
	}
}

bool Group::isGroupMember(Mob* pClient)
{
	if (pClient) {
		for (int i = 0; i < MAX_GROUP_MEMBERS; i++) {
			if (mMembers[i] == pClient)
				return true;
		}
	}

	return false;
}

bool Group::isGroupMember(const char* pName)
{
	if (pName) {
		for (int i = 0; i < MAX_GROUP_MEMBERS; i++)
			if ((strlen(pName) == strlen(mMemberNames[i])) && !strncmp(mMemberNames[i], pName, strlen(pName)))
				return true;
	}
	return false;
}

void Group::groupMessage(Mob* pSender, uint8 pLanguage, uint8 pLanguageSkill, const char* pMessage) {
	for (int i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (!mMembers[i])
			continue;

		if (mMembers[i]->isClient() && mMembers[i]->castToClient()->GetFilter(FilterGroupChat) != 0)
			mMembers[i]->castToClient()->ChannelMessageSend(pSender->getName(), mMembers[i]->getName(), 2, pLanguage, pLanguageSkill, pMessage);
	}

	ServerPacket* pack = new ServerPacket(ServerOP_OOZGroupMessage, sizeof(ServerGroupChannelMessage_Struct)+strlen(pMessage) + 1);
	ServerGroupChannelMessage_Struct* gcm = (ServerGroupChannelMessage_Struct*)pack->pBuffer;
	gcm->zoneid = zone->GetZoneID();
	gcm->groupid = GetID();
	gcm->instanceid = zone->GetInstanceID();
	strcpy(gcm->from, pSender->getName());
	strcpy(gcm->message, pMessage);
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

uint32 Group::getTotalGroupDamage(Mob* pOther) {
	uint32 total = 0;
	for (int i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (!mMembers[i])
			continue;
		if (pOther->CheckAggro(mMembers[i]))
			total += pOther->GetHateAmount(mMembers[i], true);
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

		if (mMembers[i]->isClient()) {
			if (isLeader(mMembers[i]))
				Leader = mMembers[i]->castToClient();

			strcpy(gu->yourname, mMembers[i]->getName());
			database.SetGroupID(mMembers[i]->getName(), 0, mMembers[i]->castToClient()->CharacterID());
			mMembers[i]->castToClient()->QueuePacket(outapp);
			sendMarkedNPCsToMember(mMembers[i]->castToClient(), true);

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

	entity_list.removeGroup(GetID());
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

void Group::sendUpdate(uint32 pType, Mob* pMember)
{
	if (!pMember->isClient())
		return;

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupUpdate2_Struct));
	GroupUpdate2_Struct* gu = (GroupUpdate2_Struct*)outapp->pBuffer;
	gu->action = pType;
	strcpy(gu->yourname, pMember->getName());

	int x = 0;

	gu->leader_aas = mLeaderAbilities;

	for (int i = 0; i < MAX_GROUP_MEMBERS; ++i)
	if ((mMembers[i] != nullptr) && isLeader(mMembers[i]))
	{
		strcpy(gu->leadersname, mMembers[i]->getName());
		break;
	}

	for (int i = 0; i < MAX_GROUP_MEMBERS; ++i)
	if (mMembers[i] != nullptr && mMembers[i] != pMember)
		strcpy(gu->membername[x++], mMembers[i]->getName());

	pMember->castToClient()->QueuePacket(outapp);

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
	if (mMembers[i] && mMembers[i]->isClient())
	{
		strcpy(gu->yourname, mMembers[i]->getName());
		strcpy(gu->membername, mMembers[i]->getName());
		mMembers[i]->castToClient()->QueuePacket(outapp);
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

void Group::teleportGroup(Mob* pSender, uint32 pZoneID, uint16 pInstanceID, float pX, float pY, float pZ, float pHeading)
{
	for (int i = 0; i < MAX_GROUP_MEMBERS; i++)
	{
		if (mMembers[i] != nullptr && mMembers[i]->isClient() && mMembers[i] != pSender)
		{
			mMembers[i]->castToClient()->MovePC(pZoneID, pInstanceID, pX, pY, pZ, pHeading, 0, ZoneSolicited);
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
		Mob *them = entity_list.getMOB(mMemberNames[i]);
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


void Group::groupMessage_StringID(Mob* pSender, uint32 pType, uint32 pStringID, const char* pMessage, const char* pMessage2, const char* pMessage3, const char* pMessage4, const char* pMessage5, const char* pMessage6, const char* pMessage7, const char* pMessage8, const char* pMessage9, uint32 pDistance) {
	for (int i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (mMembers[i] == nullptr)
			continue;

		if (mMembers[i] == pSender)
			continue;

		mMembers[i]->Message_StringID(pType, pStringID, pMessage, pMessage2, pMessage3, pMessage4, pMessage5, pMessage6, pMessage7, pMessage8, pMessage9, 0);
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
		database.SetGroupID(getName(), 0, CharacterID());
	}

	isgrouped = false;
}

void Group::healGroup(uint32 pAmount, Mob* pCaster, int32 pRange)
{
	if (!pCaster)
		return;

	if (!pRange)
		pRange = 200;

	float distance;
	float range2 = pRange*pRange;

	int numMem = 0;
	for (int i = 0; i < MAX_GROUP_MEMBERS; i++)
	{
		if (mMembers[i]){
			distance = pCaster->DistNoRoot(*mMembers[i]);
			if (distance <= range2){
				numMem += 1;
			}
		}
	}

	pAmount /= numMem;
	for (int i = 0; i < MAX_GROUP_MEMBERS; i++)
	{
		if (mMembers[i]){
			distance = pCaster->DistNoRoot(*mMembers[i]);
			if (distance <= range2){
				mMembers[i]->HealDamage(pAmount, pCaster);
				mMembers[i]->SendHPUpdate();
			}
		}
	}
}


void Group::balanceHP(int32 pPenalty, int32 pRange, Mob* pCaster)
{
	if (!pCaster)
		return;

	if (!pRange)
		pRange = 200;

	int dmgtaken = 0, numMem = 0;

	float distance;
	float range2 = pRange*pRange;

	for (int i = 0; i < MAX_GROUP_MEMBERS; i++)
	{
		if (mMembers[i]){
			distance = pCaster->DistNoRoot(*mMembers[i]);
			if (distance <= range2){
				dmgtaken += (mMembers[i]->GetMaxHP() - mMembers[i]->GetHP());
				numMem += 1;
			}
		}
	}

	dmgtaken += dmgtaken * pPenalty / 100;
	dmgtaken /= numMem;
	for (int i = 0; i < MAX_GROUP_MEMBERS; i++)
	{
		if (mMembers[i]){
			distance = pCaster->DistNoRoot(*mMembers[i]);
			if (distance <= range2){
				if ((mMembers[i]->GetMaxHP() - dmgtaken) < 1){ //this way the ability will never kill someone
					mMembers[i]->SetHP(1);					//but it will come darn close
					mMembers[i]->SendHPUpdate();
				}
				else{
					mMembers[i]->SetHP(mMembers[i]->GetMaxHP() - dmgtaken);
					mMembers[i]->SendHPUpdate();
				}
			}
		}
	}
}

void Group::balanceMana(int32 pPenalty, int32 pRange, Mob* pCaster)
{
	if (!pCaster)
		return;

	if (!pRange)
		pRange = 200;

	float distance;
	float range2 = pRange*pRange;

	int manataken = 0, numMem = 0;
	for (int i = 0; i < MAX_GROUP_MEMBERS; i++)
	{
		if (mMembers[i]){
			distance = pCaster->DistNoRoot(*mMembers[i]);
			if (distance <= range2){
				manataken += (mMembers[i]->GetMaxMana() - mMembers[i]->GetMana());
				numMem += 1;
			}
		}
	}

	manataken += manataken * pPenalty / 100;
	manataken /= numMem;
	for (int i = 0; i < MAX_GROUP_MEMBERS; i++)
	{
		if (mMembers[i]){
			distance = pCaster->DistNoRoot(*mMembers[i]);
			if (distance <= range2){
				if ((mMembers[i]->GetMaxMana() - manataken) < 1){
					mMembers[i]->SetMana(1);
					if (mMembers[i]->isClient())
						mMembers[i]->castToClient()->SendManaUpdate();
				}
				else{
					mMembers[i]->SetMana(mMembers[i]->GetMaxMana() - manataken);
					if (mMembers[i]->isClient())
						mMembers[i]->castToClient()->SendManaUpdate();
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

void Group::markNPC(Mob* pTarget, int pNumber)
{
	// Send a packet to all group members in this zone causing the client to prefix the Target mob's name
	// with the specified Number.
	//
	if (!pTarget || pTarget->isClient())
		return;

	if ((pNumber < 1) || (pNumber > MAX_MARKED_NPCS))
		return;

	bool AlreadyMarked = false;

	uint16 EntityID = pTarget->getID();

	for (int i = 0; i < MAX_MARKED_NPCS; ++i)
	if (mMarkedNPCs[i] == EntityID)
	{
		if (i == (pNumber - 1))
			return;

		updateXTargetMarkedNPC(i + 1, nullptr);
		mMarkedNPCs[i] = 0;

		AlreadyMarked = true;

		break;
	}

	if (!AlreadyMarked)
	{
		if (mMarkedNPCs[pNumber - 1])
		{
			Mob* m = entity_list.getMOB(mMarkedNPCs[pNumber - 1]);
			if (m)
				m->IsTargeted(-1);

			updateXTargetMarkedNPC(pNumber, nullptr);
		}

		if (EntityID)
		{
			Mob* m = entity_list.getMOB(pTarget->getID());
			if (m)
				m->IsTargeted(1);
		}
	}

	mMarkedNPCs[pNumber - 1] = EntityID;

	EQApplicationPacket *outapp = new EQApplicationPacket(OP_MarkNPC, sizeof(MarkNPC_Struct));

	MarkNPC_Struct* mnpcs = (MarkNPC_Struct *)outapp->pBuffer;

	mnpcs->TargetID = EntityID;

	mnpcs->Number = pNumber;

	Mob *m = entity_list.getMOB(EntityID);

	if (m)
		sprintf(mnpcs->Name, "%s", m->GetCleanName());

	queuePacket(outapp);

	safe_delete(outapp);

	updateXTargetMarkedNPC(pNumber, m);
}

void Group::delegateMainTank(const char *pNewMainTankName, uint8 pToggle)
{
	// This method is called when the group leader Delegates the Main Tank role to a member of the group
	// (or himself). All group members in the zone are notified of the new Main Tank and it is recorded
	// in the group_leaders table so as to persist across zones.
	//

	bool updateDB = false;

	if (!pNewMainTankName)
		return;

	Mob *m = entity_list.getMOB(pNewMainTankName);

	if (!m)
		return;

	if (mMainTankName != pNewMainTankName || !pToggle)
		updateDB = true;

	if (m->GetTarget())
		mTankTargetID = m->GetTarget()->getID();
	else
		mTankTargetID = 0;

	Mob *mtt = mTankTargetID ? entity_list.getMOB(mTankTargetID) : 0;

	setMainTank(pNewMainTankName);

	for (int i = 0; i < MAX_GROUP_MEMBERS; ++i)
	{
		if (mMembers[i] && mMembers[i]->isClient())
		{
			notifyMainTank(mMembers[i]->castToClient(), pToggle);
			mMembers[i]->castToClient()->UpdateXTargetType(GroupTank, m, pNewMainTankName);
			mMembers[i]->castToClient()->UpdateXTargetType(GroupTankTarget, mtt);
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

void Group::delegateMainAssist(const char *pNewMainAssistName, uint8 pToggle)
{
	// This method is called when the group leader Delegates the Main Assist role to a member of the group
	// (or himself). All group members in the zone are notified of the new Main Assist and it is recorded
	// in the group_leaders table so as to persist across zones.
	//

	bool updateDB = false;

	if (!pNewMainAssistName)
		return;

	Mob *m = entity_list.getMOB(pNewMainAssistName);

	if (!m)
		return;

	if (mMainAssistName != pNewMainAssistName || !pToggle)
		updateDB = true;

	if (m->GetTarget())
		mAssistTargetID = m->GetTarget()->getID();
	else
		mAssistTargetID = 0;

	setMainAssist(pNewMainAssistName);

	for (int i = 0; i < MAX_GROUP_MEMBERS; ++i) {
		if (mMembers[i] && mMembers[i]->isClient())
		{
			notifyMainAssist(mMembers[i]->castToClient(), pToggle);
			mMembers[i]->castToClient()->UpdateXTargetType(GroupAssist, m, pNewMainAssistName);
			mMembers[i]->castToClient()->UpdateXTargetType(GroupAssistTarget, m->GetTarget());
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

void Group::delegatePuller(const char *pNewPullerName, uint8 pToggle)
{
	// This method is called when the group leader Delegates the Puller role to a member of the group
	// (or himself). All group members in the zone are notified of the new Puller and it is recorded
	// in the group_leaders table so as to persist across zones.
	//

	bool updateDB = false;

	if (!pNewPullerName)
		return;

	Mob *m = entity_list.getMOB(pNewPullerName);

	if (!m)
		return;

	if (mPullerName != pNewPullerName || !pToggle)
		updateDB = true;

	if (m->GetTarget())
		mPullerTargetID = m->GetTarget()->getID();
	else
		mPullerTargetID = 0;

	setPuller(pNewPullerName);

	for (int i = 0; i < MAX_GROUP_MEMBERS; ++i) {
		if (mMembers[i] && mMembers[i]->isClient())
		{
			notifyPuller(mMembers[i]->castToClient(), pToggle);
			mMembers[i]->castToClient()->UpdateXTargetType(Puller, m, pNewPullerName);
			mMembers[i]->castToClient()->UpdateXTargetType(PullerTarget, m->GetTarget());
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

void Group::notifyMainTank(Client* pClient, uint8 pToggle)
{
	// Send a packet to the specified Client notifying them who the new Main Tank is. This causes the client to display
	// a message with the name of the Main Tank.
	//

	if (!pClient)
		return;

	if (!mMainTankName.size())
		return;

	if (pClient->GetClientVersion() < EQClientSoD)
	{
		if (pToggle)
			pClient->Message(0, "%s is now Main Tank.", mMainTankName.c_str());
		else
			pClient->Message(0, "%s is no longer Main Tank.", mMainTankName.c_str());
	}
	else
	{
		EQApplicationPacket *outapp = new EQApplicationPacket(OP_GroupRoles, sizeof(GroupRole_Struct));

		GroupRole_Struct *grs = (GroupRole_Struct*)outapp->pBuffer;

		strn0cpy(grs->Name1, mMainTankName.c_str(), sizeof(grs->Name1));

		strn0cpy(grs->Name2, getLeaderName(), sizeof(grs->Name2));

		grs->RoleNumber = 1;

		grs->Toggle = pToggle;

		pClient->QueuePacket(outapp);

		safe_delete(outapp);
	}

}

void Group::notifyMainAssist(Client* pClient, uint8 pToggle)
{
	// Send a packet to the specified Client notifying them who the new Main Assist is. This causes the client to display
	// a message with the name of the Main Assist.
	//

	if (!pClient)
		return;

	if (!mMainAssistName.size())
		return;

	if (pClient->GetClientVersion() < EQClientSoD)
	{
		EQApplicationPacket *outapp = new EQApplicationPacket(OP_DelegateAbility, sizeof(DelegateAbility_Struct));

		DelegateAbility_Struct* das = (DelegateAbility_Struct*)outapp->pBuffer;

		das->DelegateAbility = 0;

		das->MemberNumber = 0;

		das->Action = 0;

		das->EntityID = 0;

		strn0cpy(das->Name, mMainAssistName.c_str(), sizeof(das->Name));

		pClient->QueuePacket(outapp);

		safe_delete(outapp);
	}
	else
	{
		EQApplicationPacket *outapp = new EQApplicationPacket(OP_GroupRoles, sizeof(GroupRole_Struct));

		GroupRole_Struct *grs = (GroupRole_Struct*)outapp->pBuffer;

		strn0cpy(grs->Name1, mMainAssistName.c_str(), sizeof(grs->Name1));

		strn0cpy(grs->Name2, getLeaderName(), sizeof(grs->Name2));

		grs->RoleNumber = 2;

		grs->Toggle = pToggle;

		pClient->QueuePacket(outapp);

		safe_delete(outapp);
	}

	notifyAssistTarget(pClient);

}

void Group::notifyPuller(Client* pClient, uint8 pToggle)
{
	// Send a packet to the specified Client notifying them who the new Puller is. This causes the client to display
	// a message with the name of the Puller.
	//

	if (!pClient)
		return;

	if (!mPullerName.size())
		return;

	if (pClient->GetClientVersion() < EQClientSoD)
	{
		if (pToggle)
			pClient->Message(0, "%s is now Puller.", mPullerName.c_str());
		else
			pClient->Message(0, "%s is no longer Puller.", mPullerName.c_str());
	}
	else
	{
		EQApplicationPacket *outapp = new EQApplicationPacket(OP_GroupRoles, sizeof(GroupRole_Struct));

		GroupRole_Struct *grs = (GroupRole_Struct*)outapp->pBuffer;

		strn0cpy(grs->Name1, mPullerName.c_str(), sizeof(grs->Name1));

		strn0cpy(grs->Name2, getLeaderName(), sizeof(grs->Name2));

		grs->RoleNumber = 3;

		grs->Toggle = pToggle;

		pClient->QueuePacket(outapp);

		safe_delete(outapp);
	}

}

void Group::undelegateMainTank(const char* pOldMainTankName, uint8 pToggle)
{
	// Called when the group Leader removes the Main Tank delegation. Sends a packet to each group member in the zone
	// informing them of the change and update the group_leaders table.
	//
	if (pOldMainTankName == mMainTankName) {
		char errbuff[MYSQL_ERRMSG_SIZE];

		char *Query = 0;

		if (!database.RunQuery(Query, MakeAnyLenString(&Query, "UPDATE group_leaders SET maintank='' WHERE gid=%i LIMIT 1",
			GetID()), errbuff))
			LogFile->write(EQEMuLog::Error, "Unable to clear group main tank: %s\n", errbuff);

		safe_delete_array(Query);

		if (!pToggle) {
			for (int i = 0; i < MAX_GROUP_MEMBERS; ++i) {
				if (mMembers[i] && mMembers[i]->isClient())
				{
					notifyMainTank(mMembers[i]->castToClient(), pToggle);
					mMembers[i]->castToClient()->UpdateXTargetType(GroupTank, nullptr, "");
					mMembers[i]->castToClient()->UpdateXTargetType(GroupTankTarget, nullptr);
				}
			}
		}

		setMainTank("");
	}
}

void Group::undelegateMainAssist(const char* pOldMainAssistName, uint8 pToggle)
{
	// Called when the group Leader removes the Main Assist delegation. Sends a packet to each group member in the zone
	// informing them of the change and update the group_leaders table.
	//
	if (pOldMainAssistName == mMainAssistName) {
		EQApplicationPacket *outapp = new EQApplicationPacket(OP_DelegateAbility, sizeof(DelegateAbility_Struct));

		DelegateAbility_Struct* das = (DelegateAbility_Struct*)outapp->pBuffer;

		das->DelegateAbility = 0;

		das->MemberNumber = 0;

		das->Action = 1;

		das->EntityID = 0;

		strn0cpy(das->Name, pOldMainAssistName, sizeof(das->Name));

		for (int i = 0; i < MAX_GROUP_MEMBERS; ++i)
		if (mMembers[i] && mMembers[i]->isClient())
		{
			mMembers[i]->castToClient()->QueuePacket(outapp);
			mMembers[i]->castToClient()->UpdateXTargetType(GroupAssist, nullptr, "");
		}

		safe_delete(outapp);

		char errbuff[MYSQL_ERRMSG_SIZE];

		char *Query = 0;

		if (!database.RunQuery(Query, MakeAnyLenString(&Query, "UPDATE group_leaders SET assist='' WHERE gid=%i LIMIT 1",
			GetID()), errbuff))
			LogFile->write(EQEMuLog::Error, "Unable to clear group main assist: %s\n", errbuff);

		safe_delete_array(Query);

		if (!pToggle)
		{
			for (int i = 0; i < MAX_GROUP_MEMBERS; ++i)
			{
				if (mMembers[i] && mMembers[i]->isClient())
				{
					notifyMainAssist(mMembers[i]->castToClient(), pToggle);
					mMembers[i]->castToClient()->UpdateXTargetType(GroupAssistTarget, nullptr);
				}
			}
		}

		setMainAssist("");
	}
}

void Group::undelegatePuller(const char* pOldPullerName, uint8 pToggle)
{
	// Called when the group Leader removes the Puller delegation. Sends a packet to each group member in the zone
	// informing them of the change and update the group_leaders table.
	//
	if (pOldPullerName == mPullerName) {
		char errbuff[MYSQL_ERRMSG_SIZE];

		char *Query = 0;

		if (!database.RunQuery(Query, MakeAnyLenString(&Query, "UPDATE group_leaders SET puller='' WHERE gid=%i LIMIT 1",
			GetID()), errbuff))
			LogFile->write(EQEMuLog::Error, "Unable to clear group main puller: %s\n", errbuff);

		safe_delete_array(Query);

		if (!pToggle) {
			for (int i = 0; i < MAX_GROUP_MEMBERS; ++i) {
				if (mMembers[i] && mMembers[i]->isClient())
				{
					notifyPuller(mMembers[i]->castToClient(), pToggle);
					mMembers[i]->castToClient()->UpdateXTargetType(Puller, nullptr, "");
					mMembers[i]->castToClient()->UpdateXTargetType(PullerTarget, nullptr);
				}
			}
		}

		setPuller("");
	}
}

bool Group::isNPCMarker(Client* pClient)
{
	// Returns true if the specified client has been delegated the NPC Marker Role
	//
	if (!pClient)
		return false;

	if (mNPCMarkerName.size())
		return(pClient->getName() == mNPCMarkerName);

	return false;

}

void Group::setGroupAssistTarget(Mob* pMOB)
{
	// Notify all group members in the zone of the new target the Main Assist has selected.
	//
	mAssistTargetID = pMOB ? pMOB->getID() : 0;

	for (int i = 0; i < MAX_GROUP_MEMBERS; ++i)
	{
		if (mMembers[i] && mMembers[i]->isClient())
		{
			notifyAssistTarget(mMembers[i]->castToClient());
		}
	}
}

void Group::setGroupTankTarget(Mob* pMOB)
{
	mTankTargetID = pMOB ? pMOB->getID() : 0;

	for (int i = 0; i < MAX_GROUP_MEMBERS; ++i)
	{
		if (mMembers[i] && mMembers[i]->isClient())
		{
			mMembers[i]->castToClient()->UpdateXTargetType(GroupTankTarget, pMOB);
		}
	}
}

void Group::setGroupPullerTarget(Mob* pMOB)
{
	mPullerTargetID = pMOB ? pMOB->getID() : 0;

	for (int i = 0; i < MAX_GROUP_MEMBERS; ++i)
	{
		if (mMembers[i] && mMembers[i]->isClient())
		{
			mMembers[i]->castToClient()->UpdateXTargetType(PullerTarget, pMOB);
		}
	}
}

void Group::notifyAssistTarget(Client* pClient)
{
	// Send a packet to the specified client notifying them of the group target selected by the Main Assist.

	if (!pClient)
		return;

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_SetGroupTarget, sizeof(MarkNPC_Struct));

	MarkNPC_Struct* mnpcs = (MarkNPC_Struct *)outapp->pBuffer;

	mnpcs->TargetID = mAssistTargetID;

	mnpcs->Number = 0;

	pClient->QueuePacket(outapp);

	safe_delete(outapp);

	Mob *m = entity_list.getMOB(mAssistTargetID);

	pClient->UpdateXTargetType(GroupAssistTarget, m);

}

void Group::notifyTankTarget(Client* pClient)
{
	if (!pClient)
		return;

	Mob *m = entity_list.getMOB(mTankTargetID);

	pClient->UpdateXTargetType(GroupTankTarget, m);
}

void Group::notifyPullerTarget(Client* pClient)
{
	if (!pClient)
		return;

	Mob *m = entity_list.getMOB(mPullerTargetID);

	pClient->UpdateXTargetType(PullerTarget, m);
}

void Group::delegateMarkNPC(const char* pNewNPCMarkerName)
{
	// Called when the group leader has delegated the Mark NPC ability to a group member.
	// Notify all group members in the zone of the change and save the change in the group_leaders
	// table to persist across zones.
	//
	if (mNPCMarkerName.size() > 0)
		undelegateMarkNPC(mNPCMarkerName.c_str());

	if (!pNewNPCMarkerName)
		return;

	setNPCMarker(pNewNPCMarkerName);

	for (uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i)
	if (mMembers[i] && mMembers[i]->isClient())
		notifyMarkNPC(mMembers[i]->castToClient());

	char errbuff[MYSQL_ERRMSG_SIZE];

	char *Query = 0;

	if (!database.RunQuery(Query, MakeAnyLenString(&Query, "UPDATE group_leaders SET marknpc='%s' WHERE gid=%i LIMIT 1",
		pNewNPCMarkerName, GetID()), errbuff))
		LogFile->write(EQEMuLog::Error, "Unable to set group mark npc: %s\n", errbuff);

	safe_delete_array(Query);

}

void Group::notifyMarkNPC(Client* pClient)
{
	// Notify the specified client who the group member is who has been delgated the Mark NPC ability.

	if (!pClient)
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

	pClient->QueuePacket(outapp);

	safe_delete(outapp);

}
void Group::setNPCMarker(const char *pNewNPCMarkerName)
{
	mNPCMarkerName = pNewNPCMarkerName;

	Client *m = entity_list.getClientByName(mNPCMarkerName.c_str());

	if (!m)
		mNPCMarkerID = 0;
	else
		mNPCMarkerID = m->getID();
}

void Group::undelegateMarkNPC(const char* pOldNPCMarkerName)
{
	// Notify all group members in the zone that the Mark NPC ability has been rescinded from the specified
	// group member.

	if (!pOldNPCMarkerName)
		return;

	if (!mNPCMarkerName.size())
		return;

	EQApplicationPacket *outapp = new EQApplicationPacket(OP_DelegateAbility, sizeof(DelegateAbility_Struct));

	DelegateAbility_Struct* das = (DelegateAbility_Struct*)outapp->pBuffer;

	das->DelegateAbility = 1;

	das->MemberNumber = 0;

	das->Action = 1;

	das->EntityID = 0;

	strn0cpy(das->Name, pOldNPCMarkerName, sizeof(das->Name));

	for (uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i)
	if (mMembers[i] && mMembers[i]->isClient())
		mMembers[i]->castToClient()->QueuePacket(outapp);

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

void Group::unMarkNPC(uint16 pID)
{
	// Called from entity_list when the mob with the specified ID is being destroyed.
	//
	// If the given mob has been marked by this group, it is removed from the list of marked NPCs.
	// The primary reason for doing this is so that when a new group member joins or zones in, we
	// send them correct details of which NPCs are currently marked.

	if (mAssistTargetID == pID)
		mAssistTargetID = 0;


	if (mTankTargetID == pID)
		mTankTargetID = 0;

	if (mPullerTargetID == pID)
		mPullerTargetID = 0;

	for (int i = 0; i < MAX_MARKED_NPCS; ++i)
	{
		if (mMarkedNPCs[i] == pID)
		{
			mMarkedNPCs[i] = 0;
			updateXTargetMarkedNPC(i + 1, nullptr);
		}
	}
}

void Group::sendMarkedNPCsToMember(Client* pClient, bool pClear)
{
	// Send the Entity IDs of the NPCs marked by the Group Leader or delegate to the specified client.
	// If Clear == true, then tell the client to unmark the NPCs (when a member disbands).
	//
	//
	if (!pClient)
		return;

	EQApplicationPacket *outapp = new EQApplicationPacket(OP_MarkNPC, sizeof(MarkNPC_Struct));

	MarkNPC_Struct *mnpcs = (MarkNPC_Struct *)outapp->pBuffer;

	for (int i = 0; i < MAX_MARKED_NPCS; ++i)
	{
		if (mMarkedNPCs[i])
		{
			mnpcs->TargetID = mMarkedNPCs[i];

			Mob *m = entity_list.getMOB(mMarkedNPCs[i]);

			if (m)
				sprintf(mnpcs->Name, "%s", m->GetCleanName());

			if (!pClear)
				mnpcs->Number = i + 1;
			else
				mnpcs->Number = 0;

			pClient->QueuePacket(outapp);
			pClient->UpdateXTargetType((mnpcs->Number == 1) ? GroupMarkTarget1 : ((mnpcs->Number == 2) ? GroupMarkTarget2 : GroupMarkTarget3), m);
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
	if (mMembers[i] && mMembers[i]->isClient())
		sendMarkedNPCsToMember(mMembers[i]->castToClient(), true);

	for (int i = 0; i < MAX_MARKED_NPCS; ++i)
	{
		if (mMarkedNPCs[i])
		{
			Mob* m = entity_list.getMOB(mMarkedNPCs[i]);

			if (m)
				m->IsTargeted(-1);
		}

		mMarkedNPCs[i] = 0;
	}

}

int8 Group::getNumberNeedingHealing(int8 pHPRatio, bool pIncludePets) {
	int8 needHealing = 0;

	for (int i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (mMembers[i] && !mMembers[i]->qglobal) {

			if (mMembers[i]->GetHPRatio() <= pHPRatio)
				needHealing++;

			if (pIncludePets) {
				if (mMembers[i]->GetPet() && mMembers[i]->GetPet()->GetHPRatio() <= pHPRatio) {
					needHealing++;
				}
			}
		}
	}


	return needHealing;
}

void Group::updateGroupAAs()
{
	// This method updates the Groups Leadership abilities from the Player Profile of the Leader.
	//
	Mob *m = getLeader();

	if (m && m->isClient())
		m->castToClient()->GetGroupAAs(&mLeaderAbilities);
	else
		memset(&mLeaderAbilities, 0, sizeof(GroupLeadershipAA_Struct));

	saveGroupLeaderAA();
}

void Group::queueHPPacketsForNPCHealthAA(Mob* pSender, const EQApplicationPacket* pApp)
{
	// Send a mobs HP packets to group members if the leader has the NPC Health AA and the mob is the
	// target of the group's main assist, or is marked, and the member doesn't already have the mob targeted.

	if (!pSender || !pApp || !getLeadershipAA(groupAANPCHealth))
		return;

	uint16 senderID = pSender->getID();

	if (senderID != mAssistTargetID)
	{
		bool Marked = false;

		for (int i = 0; i < MAX_MARKED_NPCS; ++i)
		{
			if (mMarkedNPCs[i] == senderID)
			{
				Marked = true;
				break;
			}
		}

		if (!Marked)
			return;

	}

	for (unsigned int i = 0; i < MAX_GROUP_MEMBERS; ++i)
	if (mMembers[i] && mMembers[i]->isClient())
	{
		if (!mMembers[i]->GetTarget() || (mMembers[i]->GetTarget()->getID() != senderID))
		{
			mMembers[i]->castToClient()->QueuePacket(pApp);
		}
	}

}

void Group::changeLeader(Mob* pNewLeader)
{
	// this changes the current group leader, notifies other members, and updates leadship AA

	// if the new leader is invalid, do nothing
	if (!pNewLeader)
		return;

	Mob* oldleader = getLeader();

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupJoin_Struct));
	GroupJoin_Struct* gu = (GroupJoin_Struct*)outapp->pBuffer;
	gu->action = groupActMakeLeader;

	strcpy(gu->membername, pNewLeader->getName());
	strcpy(gu->yourname, oldleader->getName());
	setLeader(pNewLeader);
	database.SetGroupLeaderName(GetID(), pNewLeader->getName());
	updateGroupAAs();
	gu->leader_aas = mLeaderAbilities;
	for (uint32 i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (mMembers[i] && mMembers[i]->isClient())
		{
			if (mMembers[i]->castToClient()->GetClientVersion() >= EQClientSoD)
				mMembers[i]->castToClient()->SendGroupLeaderChangePacket(pNewLeader->getName());

			mMembers[i]->castToClient()->QueuePacket(outapp);
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
		if (mMembers[i] && mMembers[i]->isClient())
		{
			mMembers[i]->castToClient()->UpdateXTargetType((pNumber == 1) ? GroupMarkTarget1 : ((pNumber == 2) ? GroupMarkTarget2 : GroupMarkTarget3), pMOB);
		}
	}

}

void Group::setMainTank(const char* pNewMainTankName)
{
	mMainTankName = pNewMainTankName;

	for (uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i)
	{
		if (!strncasecmp(mMemberNames[i], pNewMainTankName, 64))
			mMemberRoles[i] |= RoleTank;
		else
			mMemberRoles[i] &= ~RoleTank;
	}
}

void Group::setMainAssist(const char* pNewMainAssistName)
{
	mMainAssistName = pNewMainAssistName;

	for (uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i)
	{
		if (!strncasecmp(mMemberNames[i], pNewMainAssistName, 64))
			mMemberRoles[i] |= RoleAssist;
		else
			mMemberRoles[i] &= ~RoleAssist;
	}
}

void Group::setPuller(const char* pNewPullerName)
{
	mPullerName = pNewPullerName;

	for (uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i)
	{
		if (!strncasecmp(mMemberNames[i], pNewPullerName, 64))
			mMemberRoles[i] |= RolePuller;
		else
			mMemberRoles[i] &= ~RolePuller;
	}
}

bool Group::hasRole(Mob* pMOB, uint8 pRole)
{
	if (!pMOB)
		return false;

	for (uint32 i = 0; i < MAX_GROUP_MEMBERS; ++i)
	{
		if ((pMOB == mMembers[i]) && (mMemberRoles[i] & pRole))
			return true;
	}
	return false;
}

void Group::splitExp(uint32 pExp, Mob* pOther) {
	if (pOther->castToNPC()->MerchantType != 0) // Ensure NPC isn't a merchant
		return;

	if (pOther->GetOwner() && pOther->GetOwner()->isClient()) // Ensure owner isn't pc
		return;

	uint32 groupexp = pExp;
	uint8 membercount = 0;
	uint8 maxlevel = 1;

	for (int i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (mMembers[i] != nullptr) {
			if (mMembers[i]->GetLevel() > maxlevel)
				maxlevel = mMembers[i]->GetLevel();

			membercount++;
		}
	}

	float groupmod;
	if (membercount > 1 && membercount < 6)
		groupmod = 1 + .2*(membercount - 1); //2members=1.2exp, 3=1.4, 4=1.6, 5=1.8
	else if (membercount == 6)
		groupmod = 2.16;
	else
		groupmod = 1.0;

	groupexp += (uint32)((float)pExp * groupmod * (RuleR(Character, GroupExpMultiplier)));

	int conlevel = Mob::GetLevelCon(maxlevel, pOther->GetLevel());
	if (conlevel == CON_GREEN)
		return;	//no exp for greenies...

	if (membercount == 0)
		return;

	for (int i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (mMembers[i] != nullptr && mMembers[i]->isClient()) // If Group Member is Client
		{
			Client *cmember = mMembers[i]->castToClient();
			// add exp + exp cap
			int16 diff = cmember->GetLevel() - maxlevel;
			int16 maxdiff = -(cmember->GetLevel() * 15 / 10 - cmember->GetLevel());
			if (maxdiff > -5)
				maxdiff = -5;
			if (diff >= (maxdiff)) { /*Instead of person who killed the mob, the person who has the highest level in the group*/
				uint32 tmp = (cmember->GetLevel() + 3) * (cmember->GetLevel() + 3) * 75 * 35 / 10;
				uint32 tmp2 = groupexp / membercount;
				cmember->AddEXP(tmp < tmp2 ? tmp : tmp2, conlevel);
			}
		}
	}
}