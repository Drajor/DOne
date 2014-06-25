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
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <iostream>

#ifdef _WINDOWS
#include <process.h>
#else
#include <pthread.h>
#include "../common/unix.h"
#endif

#include "net.h"
#include "masterentity.h"
#include "worldserver.h"
#include "../common/guilds.h"
#include "../common/packet_dump.h"
#include "../common/packet_functions.h"
#include "petitions.h"
#include "../common/spdat.h"
#include "../common/features.h"
#include "StringIDs.h"
#include "../common/dbasync.h"
#include "guild_mgr.h"
#include "raids.h"
#include "QuestParserCollection.h"

#ifdef _WINDOWS
#define snprintf	_snprintf
#define strncasecmp	_strnicmp
#define strcasecmp	_stricmp
#endif

extern Zone *zone;
extern volatile bool ZoneLoaded;
extern WorldServer worldserver;
extern NetConnection net;
extern uint32 numclients;
extern PetitionList petition_list;
extern DBAsync *dbasync;

extern char errorname[32];
extern uint16 adverrornum;

#define _A(container) decltype(*std::begin(container))

Entity::Entity()
{
	mID = 0;
	mDBAsyncWorkID = 0;
}

Entity::~Entity()
{
	dbasync->CancelWork(mDBAsyncWorkID);
}

Client* Entity::castToClient()
{
	if (this == 0x00) {
		std::cout << "castToClient error (nullptr)" << std::endl;
		DebugBreak();
		return 0;
	}
#ifdef _EQDEBUG
	if (!isClient()) {
		std::cout << "castToClient error (not client?)" << std::endl;
		DebugBreak();
		return 0;
	}
#endif
	return static_cast<Client* >(this);
}

NPC *Entity::castToNPC()
{
#ifdef _EQDEBUG
	if (!isNPC()) {
		std::cout << "castToNPC error" << std::endl;
		DebugBreak();
		return 0;
	}
#endif
	return static_cast<NPC *>(this);
}

Mob *Entity::castToMOB()
{
#ifdef _EQDEBUG
	if (!isMOB()) {
		std::cout << "castToMOB error" << std::endl;
		DebugBreak();
		return 0;
	}
#endif
	return static_cast<Mob *>(this);
}

Trap *Entity::castToTrap()
{
#ifdef DEBUG
	if (!isTrap()) {
		//std::cout << "CastToTrap error" << std::endl;
		return 0;
	}
#endif
	return static_cast<Trap *>(this);
}

Corpse *Entity::castToCorpse()
{
#ifdef _EQDEBUG
	if (!isCorpse()) {
		std::cout << "CastToCorpse error" << std::endl;
		DebugBreak();
		return 0;
	}
#endif
	return static_cast<Corpse *>(this);
}

Object *Entity::castToObject()
{
#ifdef _EQDEBUG
	if (!isObject()) {
		std::cout << "CastToObject error" << std::endl;
		DebugBreak();
		return 0;
	}
#endif
	return static_cast<Object *>(this);
}

/*Group* Entity::CastToGroup() {
#ifdef _EQDEBUG
	if(!IsGroup()) {
	std::cout << "CastToGroup error" << std::endl;
	DebugBreak();
	return 0;
	}
	#endif
	return static_cast<Group*>(this);
	}*/

Doors* Entity::castToDoors()
{
	return static_cast<Doors* >(this);
}

Beacon *Entity::castToBeacon()
{
	return static_cast<Beacon *>(this);
}

const Client* Entity::castToClient() const
{
	if (this == 0x00) {
		std::cout << "castToClient error (nullptr)" << std::endl;
		DebugBreak();
		return 0;
	}
#ifdef _EQDEBUG
	if (!isClient()) {
		std::cout << "castToClient error (not client?)" << std::endl;
		DebugBreak();
		return 0;
	}
#endif
	return static_cast<const Client* >(this);
}

const NPC *Entity::castToNPC() const
{
#ifdef _EQDEBUG
	if (!isNPC()) {
		std::cout << "castToNPC error" << std::endl;
		DebugBreak();
		return 0;
	}
#endif
	return static_cast<const NPC *>(this);
}

const Mob *Entity::castToMOB() const
{
#ifdef _EQDEBUG
	if (!isMOB()) {
		std::cout << "castToMOB error" << std::endl;
		DebugBreak();
		return 0;
	}
#endif
	return static_cast<const Mob *>(this);
}

const Trap *Entity::castToTrap() const
{
#ifdef DEBUG
	if (!isTrap()) {
		//std::cout << "CastToTrap error" << std::endl;
		return 0;
	}
#endif
	return static_cast<const Trap *>(this);
}

const Corpse *Entity::castToCorpse() const
{
#ifdef _EQDEBUG
	if (!isCorpse()) {
		std::cout << "CastToCorpse error" << std::endl;
		DebugBreak();
		return 0;
	}
#endif
	return static_cast<const Corpse *>(this);
}

const Object *Entity::castToObject() const
{
#ifdef _EQDEBUG
	if (!isObject()) {
		std::cout << "CastToObject error" << std::endl;
		DebugBreak();
		return 0;
	}
#endif
	return static_cast<const Object *>(this);
}

const Doors* Entity::castToDoors() const
{
	return static_cast<const Doors* >(this);
}

const Beacon* Entity::castToBeacon() const
{
	return static_cast<const Beacon *>(this);
}

EntityList::EntityList()
{
	// set up ids between 1 and 1500
	// neither client or server performs well if you have
	// enough entities to exhaust this list
	for (uint16 i = 1; i <= 1500; i++)
		mFreeIDs.push(i);
}

EntityList::~EntityList()
{
	//must call this before the list is destroyed, or else it will try to
	//delete the NPCs in the list, which it cannot do.
	removeAllLocalities();
}

bool EntityList::canAddHateForMOB(Mob *p)
{
	int count = 0;

	auto it = mNPCs.begin();
	while (it != mNPCs.end()) {
		NPC *npc = it->second;
		if (npc->IsOnHatelist(p))
			count++;
		// no need to continue if we already hit the limit
		if (count > 3)
			return false;
		++it;
	}

	if (count <= 2)
		return true;
	return false;
}

void EntityList::addClient(Client* pClient) {
	pClient->setID(getFreeID());
	mClients.insert(std::pair<uint16, Client* >(pClient->getID(), pClient));
	mMOBs.insert(std::pair<uint16, Mob *>(pClient->getID(), pClient));
}


void EntityList::trapProcess()
{
	if (numclients < 1)
		return;

	if (mTraps.empty()) {
		net.trap_timer.Disable();
		return;
	}

	auto it = mTraps.begin();
	while (it != mTraps.end()) {
		if (!it->second->process()) {
			safe_delete(it->second);
			mFreeIDs.push(it->first);
			it = mTraps.erase(it);
		}
		else {
			++it;
		}
	}
}


// Debug function -- checks to see if group_list has any nullptr entries.
// Meant to be called after each group-related function, in order
// to track down bugs.
void EntityList::checkGroupList(const char* pFileName, const int pFileLine)
{
	std::list<Group *>::iterator it;

	for (it = mGroups.begin(); it != mGroups.end(); ++it)
	{
		if (*it == nullptr)
		{
			LogFile->write(EQEMuLog::Error, "nullptr group, %s:%i", pFileName, pFileLine);
		}
	}
}

void EntityList::groupProcess()
{
	if (numclients < 1)
		return;

	if (mGroups.empty()) {
		net.group_timer.Disable();
		return;
	}

	auto it = mGroups.begin();
	while (it != mGroups.end()) {
		(*it)->process();
		++it;
	}

#if EQDEBUG >= 5
	checkGroupList(__FILE__, __LINE__);
#endif
}

void EntityList::queueToGroupsForNPCHealthAA(Mob* pSender, const EQApplicationPacket* pApp)
{
	auto it = mGroups.begin();
	while (it != mGroups.end()) {
		(*it)->queueHPPacketsForNPCHealthAA(pSender, pApp);
		++it;
	}
}

void EntityList::raidProcess()
{
	if (numclients < 1)
		return;

	if (mRaids.empty()) {
		net.raid_timer.Disable();
		return;
	}

	auto it = mRaids.begin();
	while (it != mRaids.end()) {
		(*it)->Process();
		++it;
	}
}

void EntityList::doorProcess()
{
#ifdef IDLE_WHEN_EMPTY
	if (numclients < 1)
		return;
#endif
	if (mDoors.empty()) {
		net.door_timer.Disable();
		return;
	}

	auto it = mDoors.begin();
	while (it != mDoors.end()) {
		if (!it->second->process()) {
			safe_delete(it->second);
			mFreeIDs.push(it->first);
			it = mDoors.erase(it);
		}
		++it;
	}
}

void EntityList::objectProcess()
{
	if (mObjects.empty()) {
		net.object_timer.Disable();
		return;
	}

	auto it = mObjects.begin();
	while (it != mObjects.end()) {
		if (!it->second->process()) {
			safe_delete(it->second);
			mFreeIDs.push(it->first);
			it = mObjects.erase(it);
		}
		else {
			++it;
		}
	}
}

void EntityList::corpseProcess()
{
	if (mCorpses.empty()) {
		net.corpse_timer.Disable(); // No corpses in list
		return;
	}

	auto it = mCorpses.begin();
	while (it != mCorpses.end()) {
		if (!it->second->process()) {
			safe_delete(it->second);
			mFreeIDs.push(it->first);
			it = mCorpses.erase(it);
		}
		else {
			++it;
		}
	}
}

void EntityList::MOBProcess()
{
#ifdef IDLE_WHEN_EMPTY
	if (numclients < 1)
		return;
#endif
	auto it = mMOBs.begin();
	while (it != mMOBs.end()) {
		if (!it->second) {
			++it;
			continue;
		}
		if (!it->second->process()) {
			Mob *mob = it->second;
			uint16 tempid = it->first;
			++it; // we don't erase here because the destructor will
			if (mob->isNPC()) {
				entity_list.removeNPC(mob->castToNPC()->getID());
			}
			else {
#ifdef _WINDOWS
				struct in_addr in;
				in.s_addr = mob->castToClient()->GetIP();
				std::cout << "Dropping client: Process=false, ip=" << inet_ntoa(in) << ", port=" << mob->castToClient()->GetPort() << std::endl;
#endif
				zone->StartShutdownTimer();
				Group *g = getGroupByMOB(mob);
				if (g) {
					LogFile->write(EQEMuLog::Error, "About to delete a client still in a group.");
					g->delMember(mob);
				}
				Raid *r = entity_list.getRaidByClient(mob->castToClient());
				if (r) {
					LogFile->write(EQEMuLog::Error, "About to delete a client still in a raid.");
					r->MemberZoned(mob->castToClient());
				}
				entity_list.removeClient(mob->getID());
			}
			entity_list.removeMOB(tempid);
		}
		else {
			++it;
		}
	}
}

void EntityList::beaconProcess()
{
	auto it = mBeacons.begin();
	while (it != mBeacons.end()) {
		if (!it->second->process()) {
			safe_delete(it->second);
			mFreeIDs.push(it->first);
			it = mBeacons.erase(it);
		}
		else {
			++it;
		}
	}
}

void EntityList::addGroup(Group* pGroup) {
	if (pGroup == nullptr)	//this seems to be happening somehow...
		return;

	uint32 gid = worldserver.NextGroupID();
	if (gid == 0) {
		LogFile->write(EQEMuLog::Error,
			"Unable to get new group ID from world server. group is going to be broken.");
		return;
	}

	addGroup(pGroup, gid);
#if EQDEBUG >= 5
	checkGroupList(__FILE__, __LINE__);
#endif
}

void EntityList::addGroup(Group* pGroup, uint32 pID) {
	pGroup->SetID(pID);
	mGroups.push_back(pGroup);
	if (!net.group_timer.Enabled())
		net.group_timer.Start();
#if EQDEBUG >= 5
	checkGroupList(__FILE__, __LINE__);
#endif
}

void EntityList::addRaid(Raid* pRaid) {
	if (pRaid == nullptr)
		return;

	uint32 gid = worldserver.NextGroupID();
	if (gid == 0) {
		LogFile->write(EQEMuLog::Error,
			"Unable to get new group ID from world server. group is going to be broken.");
		return;
	}

	addRaid(pRaid, gid);
}

void EntityList::addRaid(Raid* pRaid, uint32 pID) {
	pRaid->SetID(pID);
	mRaids.push_back(pRaid);
	if (!net.raid_timer.Enabled())
		net.raid_timer.Start();
}


void EntityList::addCorpse(Corpse* pCorpse, uint32 pID) {
	if (pCorpse == 0)
		return;

	if (pID == 0xFFFFFFFF)
		pCorpse->setID(getFreeID());
	else
		pCorpse->setID(pID);

	pCorpse->CalcCorpseName();
	mCorpses.insert(std::pair<uint16, Corpse *>(pCorpse->getID(), pCorpse));

	if (!net.corpse_timer.Enabled())
		net.corpse_timer.Start();
}

void EntityList::addNPC(NPC* pNPC, bool pSendSpawnPacket, bool pDontQueue) {
	pNPC->setID(getFreeID());
	parse->EventNPC(EVENT_SPAWN, pNPC, nullptr, "", 0);

	uint16 emoteid = pNPC->GetEmoteID();
	if (emoteid != 0)
		pNPC->DoNPCEmote(ONSPAWN, emoteid);

	if (pSendSpawnPacket) {
		if (pDontQueue) { // aka, SEND IT NOW BITCH!
			EQApplicationPacket *app = new EQApplicationPacket;
			pNPC->CreateSpawnPacket(app, pNPC);
			queueClients(pNPC, app);
			safe_delete(app);
		}
		else {
			NewSpawn_Struct *ns = new NewSpawn_Struct;
			memset(ns, 0, sizeof(NewSpawn_Struct));
			pNPC->FillSpawnStruct(ns, 0);	// Not working on player newspawns, so it's safe to use a ForWho of 0
			addToSpawnQueue(pNPC->getID(), &ns);
			safe_delete(ns);
		}
		if (pNPC->IsFindable())
			updateFindableNPCState(pNPC, false);
	}

	mNPCs.insert(std::pair<uint16, NPC *>(pNPC->getID(), pNPC));
	mMOBs.insert(std::pair<uint16, Mob *>(pNPC->getID(), pNPC));
}

void EntityList::addObject(Object* pObject, bool pSendSpawnPacket) {
	pObject->setID(getFreeID());

	if (pSendSpawnPacket) {
		EQApplicationPacket app;
		pObject->CreateSpawnPacket(&app);
#if (EQDEBUG >= 6)
		DumpPacket(&app);
#endif
		queueClients(0, &app, false);
	}

	mObjects.insert(std::pair<uint16, Object *>(pObject->getID(), pObject));

	if (!net.object_timer.Enabled())
		net.object_timer.Start();
}

void EntityList::addDoor(Doors* pDoor) {
	pDoor->SetEntityID(getFreeID());
	mDoors.insert(std::pair<uint16, Doors* >(pDoor->GetEntityID(), pDoor));

	if (!net.door_timer.Enabled())
		net.door_timer.Start();
}

void EntityList::addTrap(Trap* pTrap) {
	pTrap->setID(getFreeID());
	mTraps.insert(std::pair<uint16, Trap *>(pTrap->getID(), pTrap));
	if (!net.trap_timer.Enabled())
		net.trap_timer.Start();
}

void EntityList::addBeacon(Beacon* pBeacon) {
	pBeacon->setID(getFreeID());
	mBeacons.insert(std::pair<uint16, Beacon *>(pBeacon->getID(), pBeacon));
}

void EntityList::addToSpawnQueue(uint16 pEntityID, NewSpawn_Struct** pApp) {
	if (mClients.size() == 0) return;

	SpawnQueue.Append(*pApp);
	NumSpawnsOnQueue++;
	if (tsFirstSpawnOnQueue == 0xFFFFFFFF)
		tsFirstSpawnOnQueue = Timer::GetCurrentTime();
	*pApp = nullptr;
}

void EntityList::checkSpawnQueue() {
	// Send the stuff if the oldest packet on the queue is older than 50ms -Quagmire
	if (tsFirstSpawnOnQueue != 0xFFFFFFFF && (Timer::GetCurrentTime() - tsFirstSpawnOnQueue) > 50) {
		LinkedListIterator<NewSpawn_Struct *> iterator(SpawnQueue);
		EQApplicationPacket *outapp = 0;

		iterator.Reset();
		while (iterator.MoreElements()) {
			outapp = new EQApplicationPacket;
			Mob::CreateSpawnPacket(outapp, iterator.GetData());
			queueClients(0, outapp);
			safe_delete(outapp);
			iterator.RemoveCurrent();
		}
		tsFirstSpawnOnQueue = 0xFFFFFFFF;
		NumSpawnsOnQueue = 0;
	}
}

Doors* EntityList::findDoor(uint8 pID) {
	if (pID == 0 || mDoors.empty())
		return nullptr;

	for (auto i = mDoors.begin(); i != mDoors.end(); i++) {
		if (i->second->GetDoorID() == pID)
			return i->second;
	}
	return nullptr;
}

Object* EntityList::findObject(uint32 pID) {
	if (pID == 0 || mObjects.empty())
		return nullptr;

	for (auto i = mObjects.begin(); i != mObjects.end(); i++) {
		if (i->second->GetDBID() == pID)
			return i->second;
	}
	return nullptr;
}

Object* EntityList::findNearbyObject(float pX, float pY, float pZ, float pRadius) {
	if (mObjects.empty())
		return nullptr;

	float ox;
	float oy;
	float oz;
	
	for (auto i = mObjects.begin(); i != mObjects.end(); i++) {
		Object* object = i->second;
		object->GetLocation(&ox, &oy, &oz);

		ox = (pX < ox) ? (ox - pX) : (pX - ox);
		oy = (pY < oy) ? (oy - pY) : (pY - oy);
		oz = (pZ < oz) ? (oz - pZ) : (pZ - oz);

		if ((ox <= pRadius) && (oy <= pRadius) && (oz <= pRadius))
			return object;
	}
	return nullptr;
}

bool EntityList::makeDoorSpawnPacket(EQApplicationPacket* pApp, Client* pClient) {
	if (mDoors.empty())
		return false;

	uint32 mask_test = pClient->GetClientVersionBit();
	int count = 0;

	auto it = mDoors.begin();
	while (it != mDoors.end()) {
		if ((it->second->GetClientVersionMask() & mask_test) &&
			strlen(it->second->GetDoorName()) > 3)
			count++;
		++it;
	}

	if (count == 0 || count > 500)
		return false;

	uint32 length = count * sizeof(Door_Struct);
	uchar *packet_buffer = new uchar[length];
	memset(packet_buffer, 0, length);
	uchar *ptr = packet_buffer;
	Doors* door;
	Door_Struct nd;

	it = mDoors.begin();
	while (it != mDoors.end()) {
		door = it->second;
		if (door && (door->GetClientVersionMask() & mask_test) &&
			strlen(door->GetDoorName()) > 3) {
			memset(&nd, 0, sizeof(nd));
			memcpy(nd.name, door->GetDoorName(), 32);
			nd.xPos = door->GetX();
			nd.yPos = door->GetY();
			nd.zPos = door->GetZ();
			nd.heading = door->GetHeading();
			nd.incline = door->GetIncline();
			nd.size = door->GetSize();
			nd.doorId = door->GetDoorID();
			nd.opentype = door->GetOpenType();
			nd.state_at_spawn = door->GetInvertState() ? !door->IsDoorOpen() : door->IsDoorOpen();
			nd.invert_state = door->GetInvertState();
			nd.door_param = door->GetDoorParam();
			memcpy(ptr, &nd, sizeof(nd));
			ptr += sizeof(nd);
			*(ptr - 1) = 0x01;
			*(ptr - 3) = 0x01;
		}
		++it;
	}

	pApp->SetOpcode(OP_SpawnDoor);
	pApp->size = length;
	pApp->pBuffer = packet_buffer;
	return true;
}

Entity* EntityList::getEntityMOB(uint16 pID) {
	return mMOBs.count(pID) ? mMOBs.at(pID) : nullptr;
}

Entity* EntityList::getEntityMOB(const char* pName) {
	if (pName == 0 || mMOBs.empty())
		return nullptr;

	for (auto i = mMOBs.begin(); i != mMOBs.end(); i++) {
		if (strcasecmp(i->second->getName(), pName) == 0)
			return i->second;
	}
	return nullptr;
}

Entity* EntityList::getEntityDoor(uint16 pID) {
	return mDoors.count(pID) ? mDoors.at(pID) : nullptr;
}

Entity *EntityList::getEntityCorpse(uint16 pID) {
	return mCorpses.count(pID) ? mCorpses.at(pID) : nullptr;
}

Entity* EntityList::getEntityCorpse(const char* pName) {
	if (pName == 0 || mCorpses.empty())
		return nullptr;

	for (auto i = mCorpses.begin(); i != mCorpses.end(); i++) {
		if (strcasecmp(i->second->getName(), pName) == 0)
			return i->second;
	}
	return nullptr;
}

Entity *EntityList::getEntityTrap(uint16 pID) {
	return mTraps.count(pID) ? mTraps.at(pID) : nullptr;
}

Entity *EntityList::getEntityObject(uint16 pID) {
	return mObjects.count(pID) ? mObjects.at(pID) : nullptr;
}

Entity *EntityList::getEntityBeacon(uint16 pID) {
	return mBeacons.count(pID) ? mBeacons.at(pID) : nullptr;
}

Entity *EntityList::getID(uint16 pID) {
	Entity *entity = 0;
	if ((entity = entity_list.getEntityMOB(pID)) != 0)
		return entity;
	else if ((entity = entity_list.getEntityDoor(pID)) != 0)
		return entity;
	else if ((entity = entity_list.getEntityCorpse(pID)) != 0)
		return entity;
	else if ((entity = entity_list.getEntityObject(pID)) != 0)
		return entity;
	else if ((entity = entity_list.getEntityTrap(pID)) != 0)
		return entity;
	else if ((entity = entity_list.getEntityBeacon(pID)) != 0)
		return entity;
	else
		return 0;
}

NPC *EntityList::getNPCByNPCTypeID(uint32 pNPCTypeID) {
	if (pNPCTypeID == 0 || mNPCs.empty())
		return nullptr;

	auto it = mNPCs.begin();
	while (it != mNPCs.end()) {
		if (it->second->GetNPCTypeID() == pNPCTypeID)
			return it->second;
		++it;
	}

	return nullptr;
}

Mob *EntityList::getMOB(uint16 pID) {
	Entity *entity = nullptr;

	if (pID == 0)
		return nullptr;

	if ((entity = entity_list.getEntityMOB(pID)))
		return entity->castToMOB();
	else if ((entity = entity_list.getEntityCorpse(pID)))
		return entity->castToMOB();

	return nullptr;
}

Mob *EntityList::getMOB(const char* pName) {
	Entity* entity = nullptr;

	if (pName == 0)
		return nullptr;

	if ((entity = entity_list.getEntityMOB(pName)))
		return entity->castToMOB();
	else if ((entity = entity_list.getEntityCorpse(pName)))
		return entity->castToMOB();

	return nullptr;
}

Mob *EntityList::getMOBByNpcTypeID(uint32 pID) {
	if (pID == 0 || mMOBs.empty())
		return 0;

	for (auto i = mMOBs.begin(); i != mMOBs.end(); i++) {
		if (i->second->GetNPCTypeID() == pID)
			return i->second;
	}
	return nullptr;
}

bool EntityList::isMOBSpawnedByNpcTypeID(uint32 pID) {
	if (pID == 0 || mNPCs.empty())
		return false;

	auto it = mNPCs.begin();
	while (it != mNPCs.end()) {
		// Mobs will have a 0 as their GetID() if they're dead
		if (it->second->GetNPCTypeID() == pID && it->second->getID() != 0)
			return true;
		++it;
	}

	return false;
}

Object* EntityList::getObjectByDatabaseID(uint32 pID) {
	if (pID == 0 || mObjects.empty())
		return nullptr;

	for (auto i = mObjects.begin(); i != mObjects.end(); i++) {
		if (i->second->GetDBID() == pID)
			return i->second;
	}
	return nullptr;
}

Doors* EntityList::getDoorsByDatabaseID(uint32 pID) {
	if (pID == 0 || mDoors.empty())
		return nullptr;

	for (auto i = mDoors.begin(); i != mDoors.end(); i++) {
		if (i->second->GetDoorDBID() == pID)
			return i->second;
	}
	return nullptr;
}

Doors* EntityList::getDoorsByDoorID(uint32 pID) {
	if (pID == 0 || mDoors.empty())
		return nullptr;

	for (auto i = mDoors.begin(); i != mDoors.end(); i++) {
		if (i->second->castToDoors()->GetDoorID() == pID)
			return i->second;
	}
	return nullptr;
}

uint16 EntityList::getFreeID() {
	if (mFreeIDs.empty()) { // hopefully this will never be true
		// The client has a hard cap on entity count some where
		// Neither the client or server performs well with a lot entities either
		uint16 newid = 1500;
		while (true) {
			newid++;
			if (getID(newid) == nullptr)
				return newid;
		}
	}

	uint16 newid = mFreeIDs.front();
	mFreeIDs.pop();
	return newid;
}

// if no language skill is specified, sent with 100 skill
void EntityList::channelMessage(Mob* pFrom, uint8 pChannelNumber, uint8 pLanguage, const char* pMessage, ...)
{
	channelMessage(pFrom, pChannelNumber, pLanguage, 100, pMessage);
}

void EntityList::channelMessage(Mob* pFrom, uint8 pChannelNumber, uint8 pLanguage, uint8 pLanguageSkill, const char* pMessage, ...) {
	va_list argptr;
	char buffer[4096];

	va_start(argptr, pMessage);
	vsnprintf(buffer, 4096, pMessage, argptr);
	va_end(argptr);

	for (auto i = mClients.begin(); i != mClients.end(); i++) {
		Client* client = i->second;
		eqFilterType filter = FilterNone;
		if (pChannelNumber == 3) //shout
			filter = FilterShouts;
		else if (pChannelNumber == 4) //auction
			filter = FilterAuctions;

		// Only say is limited in range
		if (pChannelNumber != 8 || client->Dist(*pFrom) < 200)
		if (filter == FilterNone || client->GetFilter(filter) != FilterHide)
			client->ChannelMessageSend(pFrom->getName(), 0, pChannelNumber, pLanguage, pLanguageSkill, buffer);
	}
}

void EntityList::channelMessageSend(Mob* pTo, uint8 pChannelNumber, uint8 pLanguage, const char* pMessage, ...) {
	va_list argptr;
	char buffer[4096];

	va_start(argptr, pMessage);
	vsnprintf(buffer, 4096, pMessage, argptr);
	va_end(argptr);

	if (mClients.count(pTo->getID()))
		mClients.at(pTo->getID())->ChannelMessageSend(0, 0, pChannelNumber, pLanguage, buffer);
}

void EntityList::sendZoneSpawns(Client* pClient) {
	EQApplicationPacket* app;
	auto i = mMOBs.begin();
	while (i != mMOBs.end()) {
		Mob *ent = i->second;
		if (!(ent->isInZone()) || (ent->isClient())) {
			if (ent->castToClient()->GMHideMe(pClient) || ent->castToClient()->IsHoveringForRespawn()) {
				++i;
				continue;
			}
		}

		app = new EQApplicationPacket;
		i->second->castToMOB()->CreateSpawnPacket(app); // TODO: Use zonespawns opcode instead
		pClient->QueuePacket(app, true, Client::CLIENT_CONNECTED);
		safe_delete(app);
		++i;
	}
}

void EntityList::sendZoneSpawnsBulk(Client* pClient) {
	NewSpawn_Struct ns;
	Mob *spawn;
	uint32 maxspawns = 100;

	if (maxspawns > mMOBs.size())
		maxspawns = mMOBs.size();
	BulkZoneSpawnPacket *bzsp = new BulkZoneSpawnPacket(pClient, maxspawns);
	for (auto i = mMOBs.begin(); i != mMOBs.end(); i++) {
		spawn = i->second;
		if (spawn && spawn->isInZone()) {
			if (spawn->isClient() && (spawn->castToClient()->GMHideMe(pClient) ||
				spawn->castToClient()->IsHoveringForRespawn()))
				continue;
			memset(&ns, 0, sizeof(NewSpawn_Struct));
			spawn->FillSpawnStruct(&ns, pClient);
			bzsp->AddSpawn(&ns);
		}
	}
	safe_delete(bzsp);
}

//this is a hack to handle a broken spawn struct
void EntityList::sendZonePVPUpdates(Client* pClient) {
	for (auto i = mClients.begin(); i != mClients.end(); i++) {
		Client* client = i->second;
		if (client->GetPVP())
			client->SendAppearancePacket(AT_PVP, client->GetPVP(), true, false, pClient);
	}
}

void EntityList::sendZoneCorpses(Client* pClient) {
	EQApplicationPacket* app;
	for (auto i = mCorpses.begin(); i != mCorpses.end(); i++) {
		Corpse *ent = i->second;
		app = new EQApplicationPacket();
		ent->CreateSpawnPacket(app);
		pClient->QueuePacket(app, true, Client::CLIENT_CONNECTED);
		safe_delete(app);
	}
}

void EntityList::sendZoneCorpsesBulk(Client* pClient) {
	NewSpawn_Struct ns;
	Corpse *spawn;
	uint32 maxspawns = 100;

	BulkZoneSpawnPacket* bzsp = new BulkZoneSpawnPacket(pClient, maxspawns);

	for (auto i = mCorpses.begin(); i != mCorpses.end(); ++i) {
		spawn = i->second;
		if (spawn && spawn->isInZone()) {
			memset(&ns, 0, sizeof(NewSpawn_Struct));
			spawn->FillSpawnStruct(&ns, pClient);
			bzsp->AddSpawn(&ns);
		}
	}
	safe_delete(bzsp);
}

void EntityList::sendZoneObjects(Client* pClient) {
	for (auto i = mObjects.begin(); i != mObjects.end(); i++) {
		EQApplicationPacket* app = new EQApplicationPacket();
		i->second->CreateSpawnPacket(app);
		pClient->FastQueuePacket(&app);
	}
}

void EntityList::save() {
	std::for_each(mClients.begin(), mClients.end(), [](_A(mClients)& pValue){ pValue.second->save(); });
}

void EntityList::replaceWithTarget(Mob* pOldMOB, Mob* pNewTarget) {
	if (!pNewTarget) return;
	auto f = [pOldMOB, pNewTarget](_A(mMOBs)& pValue) {
		auto mob = pValue.second;
		if (mob->IsAIControlled() && mob->RemoveFromHateList(pOldMOB))
			mob->AddToHateList(pNewTarget, 1, 0);
	};
	std::for_each(mMOBs.begin(), mMOBs.end(), f);
}

void EntityList::removeFromTargets(Mob* pMOB, bool pRemoveFromXTargets) {
	auto i = mMOBs.begin();
	while (i != mMOBs.end()) {
		Mob *m = i->second;
		++i;

		if (!m)
			continue;

		m->RemoveFromHateList(pMOB);

		if (pRemoveFromXTargets) {
			if (m->isClient())
				m->castToClient()->RemoveXTarget(pMOB, false);
			// FadingMemories calls this function passing the client.
			else if (pMOB->isClient())
				pMOB->castToClient()->RemoveXTarget(m, false);
		}
	}
}

void EntityList::removeFromXTargets(Mob* pMOB) {
	auto i = mClients.begin();
	while (i != mClients.end()) {
		i->second->RemoveXTarget(pMOB, false);
		++i;
	}
}

void EntityList::removeFromAutoXTargets(Mob* pMOB) {
	auto i = mClients.begin();
	while (i != mClients.end()) {
		i->second->RemoveXTarget(pMOB, true);
		++i;
	}
}

void EntityList::refreshAutoXTargets(Client* pClient) {
	if (!pClient) return;

	for (auto& i : mMOBs) {
		Mob* mob = i.second;

		if (!mob || mob->GetHP() <= 0)
			continue;

		if (mob->CheckAggro(pClient) && !pClient->IsXTarget(mob)) {
			pClient->AddAutoXTarget(mob);
			break;
		}
	}
}

void EntityList::refreshClientXTargets(Client* pClient) {
	if (!pClient) return;

	for (auto& i : mClients) {
		Client* client = i.second;
		if (client->IsClientXTarget(pClient))
			client->UpdateClientXTarget(pClient);
	}
}

void EntityList::queueClientsByTarget(Mob* pSender, const EQApplicationPacket* pApp, bool pSendToSender, Mob* pSkipThisMOB, bool pAckReq, bool pHoTT, uint32 pClientVersionBits)
{
	auto i = mClients.begin();
	while (i != mClients.end()) {
		Client* client = i->second;
		++i; // TODO: This looks bad.

		Mob *Target = client->GetTarget();

		if (!Target)
			continue;

		Mob *TargetsTarget = nullptr;

		if (Target)
			TargetsTarget = Target->GetTarget();

		bool Send = false;

		if (client == pSkipThisMOB)
			continue;

		if (pSendToSender)
		if (client == pSender)
			Send = true;

		if (client != pSender) {
			if (Target == pSender)
				Send = true;
			else if (pHoTT)
			if (TargetsTarget == pSender)
				Send = true;
		}

		if (Send && (client->GetClientVersionBit() & pClientVersionBits))
			client->QueuePacket(pApp, pAckReq);
	}
}

void EntityList::queueClientsByXTarget(Mob* pSender, const EQApplicationPacket* pApp, bool pSendToSender) {
	auto i = mClients.begin();
	while (i != mClients.end()) {
		Client* c = i->second;
		++i;

		if (!c || ((c == pSender) && !pSendToSender))
			continue;

		if (!c->IsXTarget(pSender))
			continue;

		c->QueuePacket(pApp);
	}
}

void EntityList::queueCloseClients(Mob* pSender, const EQApplicationPacket* pApp, bool pIgnoreSender, float pDistance, Mob* pSkipThisMob, bool pAckReq, eqFilterType pFilter) {
	if (pSender == nullptr) {
		queueClients(pSender, pApp, pIgnoreSender);
		return;
	}

	if (pDistance <= 0)
		pDistance = 600;
	float dist2 = pDistance * pDistance; //pow(dist, 2);

	auto i = mClients.begin();
	while (i != mClients.end()) {
		Client* ent = i->second;

		if ((!pIgnoreSender || ent != pSender) && (ent != pSkipThisMob)) {
			eqFilterMode filter2 = ent->GetFilter(pFilter);
			if (ent->Connected() &&
				(pFilter == FilterNone
				|| filter2 == FilterShow
				|| (filter2 == FilterShowGroupOnly && (pSender == ent ||
				(ent->GetGroup() && ent->GetGroup()->isGroupMember(pSender))))
				|| (filter2 == FilterShowSelfOnly && ent == pSender))
				&& (ent->DistNoRoot(*pSender) <= dist2)) {
				ent->QueuePacket(pApp, pAckReq, Client::CLIENT_CONNECTED);
			}
		}
		++i;
	}
}

//sender can be null
void EntityList::queueClients(Mob* pSender, const EQApplicationPacket* pApp, bool pIgnoreSender, bool pAckReq) {
	auto i = mClients.begin();
	while (i != mClients.end()) {
		Client* ent = i->second;

		if ((!pIgnoreSender || ent != pSender))
			ent->QueuePacket(pApp, pAckReq, Client::CLIENT_CONNECTED);

		++i;
	}
}

void EntityList::queueManaged(Mob* pSender, const EQApplicationPacket* pApp, bool pIgnoreSender, bool pAckReq) {
	auto i = mClients.begin();
	while (i != mClients.end()) {
		Client* ent = i->second;

		if ((!pIgnoreSender || ent != pSender))
			ent->QueuePacket(pApp, pAckReq, Client::CLIENT_CONNECTED);

		++i;
	}
}


void EntityList::queueClientsStatus(Mob* pSender, const EQApplicationPacket* pApp, bool pIgnoreSender , uint8 pMinStatus, uint8 pMaxStatus) {
	auto i = mClients.begin();
	while (i != mClients.end()) {
		if ((!pIgnoreSender || i->second != pSender) &&
			(i->second->Admin() >= pMinStatus && i->second->Admin() <= pMaxStatus))
			i->second->QueuePacket(pApp);

		++i;
	}
}

void EntityList::duelMessage(Mob* pWinner, Mob* pLoser, bool pFlee)
{
	if (pWinner->GetLevelCon(pWinner->GetLevel(), pLoser->GetLevel()) > 2) {
		std::vector<void*> args;
		args.push_back(pWinner);
		args.push_back(pLoser);

		parse->EventPlayer(EVENT_DUEL_WIN, pWinner->castToClient(), pLoser->getName(), pLoser->castToClient()->CharacterID(), &args);
		parse->EventPlayer(EVENT_DUEL_LOSE, pLoser->castToClient(), pWinner->getName(), pWinner->castToClient()->CharacterID(), &args);
	}

	auto it = mClients.begin();
	while (it != mClients.end()) {
		Client* cur = it->second;
		//might want some sort of distance check in here?
		if (cur != pWinner && cur != pLoser) {
			if (pFlee)
				cur->Message_StringID(15, DUEL_FLED, pWinner->getName(), pLoser->getName(), pLoser->getName());
			else
				cur->Message_StringID(15, DUEL_FINISHED, pWinner->getName(), pLoser->getName());
		}
		++it;
	}
}

Client* EntityList::getClientByName(const char* pName) {
	for (auto i = mClients.begin(); i != mClients.end(); i++) {
		if (strcasecmp(i->second->getName(), pName) == 0)
			return i->second;
	}
	return nullptr;
}

Client* EntityList::getClientByCharacterID(uint32 pCharacterID) {
	for (auto i = mClients.begin(); i != mClients.end(); i++) {
		if (i->second->CharacterID() == pCharacterID)
			return i->second;
	}
	return nullptr;
}

Client* EntityList::getClientByWID(uint32 pWID) {
	for (auto i = mClients.end(); i != mClients.end(); i++) {
		if (i->second->GetWID() == pWID)
			return i->second;
	}
	return nullptr;
}

Client* EntityList::getRandomClient(float pX, float pY, float pZ, float pDistance, Client* pExcludeClient)
{
	std::vector<Client*> clientsInRange;

	for (auto i = mClients.begin(); i != mClients.end(); i++) {
		if (i->second != pExcludeClient && i->second->DistNoRoot(pX, pY, pZ) <= pDistance)
			clientsInRange.push_back(i->second);
	}
	if (clientsInRange.empty())
		return nullptr;

	return clientsInRange[MakeRandomInt(0, clientsInRange.size() - 1)];
}

Corpse* EntityList::getCorpseByOwner(Client* pClient) {
	for (auto i = mCorpses.begin(); i != mCorpses.end(); i++) {
		if (i->second->isPlayerCorpse() && strcasecmp(i->second->GetOwnerName(), pClient->getName()) == 0)
			return i->second;
	}
	return nullptr;
}

Corpse* EntityList::getCorpseByOwnerWithinRange(Client* pClient, Mob* pCenter, int pRange) {
	Corpse* corpse = nullptr;
	for (auto i = mCorpses.begin(); i != mCorpses.end(); i++) {
		corpse = i->second;
		if (corpse->isPlayerCorpse() && pCenter->DistNoRootNoZ(*i->second) < pRange && strcasecmp(corpse->GetOwnerName(), pClient->getName()) == 0)
			return corpse;
	}
	return nullptr;
}

Corpse* EntityList::getCorpseByDatabaseID(uint32 pDatabaseID) {
	for (auto i = mCorpses.begin(); i != mCorpses.end(); i++) {
		if (i->second->GetDBID() == pDatabaseID)
			return i->second;
	}
	return nullptr;
}

Corpse* EntityList::getCorpseByName(const char* pName) {
	for (auto i = mCorpses.begin(); i != mCorpses.end(); i++) {
		if (strcmp(i->second->getName(), pName) == 0)
			return i->second;
	}
	return nullptr;
}

Spawn2* EntityList::getSpawnByID(uint32 pID) {
	if (!zone)
		return nullptr;

	LinkedListIterator<Spawn2 *> iterator(zone->spawn2_list);
	iterator.Reset();
	while (iterator.MoreElements())
	{
		if (iterator.GetData()->GetID() == pID) {
			return iterator.GetData();
		}
		iterator.Advance();
	}

	return nullptr;
}

void EntityList::removeAllCorpsesByCharacterID(uint32 pCharacterID) {
	auto i = mCorpses.begin();
	while (i != mCorpses.end()) {
		if (i->second->getCharacterID() == pCharacterID) {
			safe_delete(i->second);
			mFreeIDs.push(i->first);
			i = mCorpses.erase(i);
		}
		else {
			++i;
		}
	}
}

void EntityList::removeCorpseByDatabaseID(uint32 pDatabaseID) {
	auto i = mCorpses.begin();
	while (i != mCorpses.end()) {
		if (i->second->GetDBID() == pDatabaseID) {
			safe_delete(i->second);
			mFreeIDs.push(i->first);
			i = mCorpses.erase(i);
		}
		else {
			++i;
		}
	}
}

int EntityList::resurrectAllCorpsesByCharacterID(uint32 pCharacterID)
{
	int experienceReturned = 0;

	for (auto i = mCorpses.begin(); i != mCorpses.end(); i++) {
		if (i->second->getCharacterID() == pCharacterID) {
			experienceReturned += i->second->getResurrectionExp();
			i->second->setResurrected(true);
			i->second->CompleteRezz();
		}
	}
	return experienceReturned;
}

Group* EntityList::getGroupByMOB(Mob *pMOB) {
	for (auto i = mGroups.begin(); i != mGroups.end(); i++) {
		if ((*i)->isGroupMember(pMOB))
			return *i;
	}
	return nullptr;
}

Group* EntityList::getGroupByLeaderName(const char* pLeaderName) {
	for (auto i = mGroups.begin(); i != mGroups.end(); i++) {
		if (!strcmp((*i)->getLeaderName(), pLeaderName))
			return *i;
	}
	return nullptr;
}

Group* EntityList::getGroupByID(uint32 pGroupID) {
	for (auto i = mGroups.begin(); i != mGroups.end(); i++) {
		if ((*i)->GetID() == pGroupID)
			return *i;
	}
	return nullptr;
}

Group* EntityList::getGroupByClient(Client* pClient) {
	for (auto i = mGroups.begin(); i != mGroups.end(); i++) {
		if ((*i)->isGroupMember(pClient->castToMOB()))
			return *i;
	}
	return nullptr;
}

Raid* EntityList::getRaidByID(uint32 pRaidID) {
	for (auto i = mRaids.begin(); i != mRaids.end(); i++) {
		if ((*i)->GetID() == pRaidID)
			return *i;
	}
	return nullptr;
}

Raid* EntityList::getRaidByClient(Client* pClient) {
	Raid* raid = nullptr;
	for (auto i = mRaids.begin(); i != mRaids.end(); i++) {
		raid = *i;
		for (int j = 0; j < MAX_RAID_MEMBERS; j++) {
			if (raid->members[j].member && raid->members[j].member == pClient)
				return raid;
		}
	}
	return nullptr;
}

Client* EntityList::getClientByAccountID(uint32 pAccountID) {
	for (auto i = mClients.begin(); i != mClients.end(); i++) {
		if (i->second->AccountID() == pAccountID)
			return i->second;
	}
	return nullptr;
}

void EntityList::channelMessageFromWorld(const char* pFrom, const char* pTo, uint8 pChannelNumber, uint32 pGuildID, uint8 pLanguage, const char* pMessage) {
	for (auto i = mClients.begin(); i != mClients.end(); i++) {
		Client* client = i->second;
		if (pChannelNumber == 0) {
			if (!client->IsInGuild(pGuildID))
				continue;
			if (!guild_mgr.checkPermission(pGuildID, client->GuildRank(), GUILD_HEAR))
				continue;
			if (client->GetFilter(FilterGuildChat) == FilterHide)
				continue;
		}
		else if (pChannelNumber == 5) {
			if (client->GetFilter(FilterOOC) == FilterHide)
				continue;
		}
		client->ChannelMessageSend(pFrom, pTo, pChannelNumber, pLanguage, pMessage);
	}
}

void EntityList::message(uint32 pGuildID, uint32 pType, const char* pMessage, ...) {
	va_list argptr;
	char buffer[4096];

	va_start(argptr, pMessage);
	vsnprintf(buffer, 4096, pMessage, argptr);
	va_end(argptr);

	for (auto i = mClients.begin(); i != mClients.end(); i++) {
		Client* client = i->second;
		if (pGuildID == 0 || client->IsInGuild(pGuildID))
			client->message(pType, buffer);
	}
}

void EntityList::queueClientsGuild(Mob* pSender, const EQApplicationPacket* pApp, bool pIgnoreSender, uint32 pGuildID) {
	auto i = mClients.begin();
	while (i != mClients.end()) {
		Client* client = i->second;
		if (client->IsInGuild(pGuildID))
			client->QueuePacket(pApp);
		++i;
	}
}

void EntityList::queueClientsGuildBankItemUpdate(const GuildBankItemUpdate_Struct* pUpdate, uint32 pGuildID) {
	EQApplicationPacket *outapp = new EQApplicationPacket(OP_GuildBank, sizeof(GuildBankItemUpdate_Struct));

	GuildBankItemUpdate_Struct *outgbius = (GuildBankItemUpdate_Struct*)outapp->pBuffer;

	memcpy(outgbius, pUpdate, sizeof(GuildBankItemUpdate_Struct));

	const Item_Struct *Item = database.GetItem(pUpdate->ItemID);

	auto i = mClients.begin();
	while (i != mClients.end()) {
		Client* client = i->second;

		if (client->IsInGuild(pGuildID)) {
			if (Item && (pUpdate->Permissions == GuildBankPublicIfUsable))
				outgbius->Useable = Item->IsEquipable(client->GetBaseRace(), client->GetBaseClass());

			client->QueuePacket(outapp);
		}

		++i;
	}
	safe_delete(outapp);
}

void EntityList::messageStatus(uint32 pGuildID, int pMinStatus, uint32 pType, const char* pMessage, ...) {
	va_list argptr;
	char buffer[4096];

	va_start(argptr, pMessage);
	vsnprintf(buffer, 4096, pMessage, argptr);
	va_end(argptr);

	for (auto i = mClients.begin(); i != mClients.end(); i++) {
		Client* client = i->second;
		if ((pGuildID == 0 || client->IsInGuild(pGuildID)) && client->Admin() >= pMinStatus)
			client->message(pType, buffer);
	}
}

// works much like MessageClose, but with formatted strings
void EntityList::messageCloseStringID(Mob* pSender, bool pSkipSender, float pDistance, uint32 pType, uint32 pStringID, const char* pMessage1, const char* pMessage2, const char* pMessage3, const char* pMessage4, const char* pMessage5, const char* pMessage6, const char* pMessage7, const char* pMessage8, const char* pMessage9) {
	Client* client;
	float dist2 = pDistance * pDistance;
	for (auto i = mClients.begin(); i != mClients.end(); ++i) {
		client = i->second;
		if (client && client->DistNoRoot(*pSender) <= dist2 && (!pSkipSender || client != pSender))
			client->Message_StringID(pType, pStringID, pMessage1, pMessage2, pMessage3, pMessage4, pMessage5, pMessage6, pMessage7, pMessage8, pMessage9);
	}
}

void EntityList::filteredMessageCloseStringID(Mob* pSender, bool pSkipSender, float pDistance, uint32 pType, eqFilterType pFilter, uint32 pStringID, const char* pMessage1, const char* pMessage2, const char* pMessage3, const char* pMessage4, const char* pMessage5, const char* pMessage6, const char* pMessage7, const char* pMessage8, const char* pMessage9) {
	Client* client;
	float dist2 = pDistance * pDistance;
	for (auto i = mClients.begin(); i != mClients.end(); ++i) {
		client = i->second;
		if (client && client->DistNoRoot(*pSender) <= dist2 && (!pSkipSender || client != pSender))
			client->FilteredMessage_StringID(pSender, pType, pFilter, pStringID, pMessage1, pMessage2, pMessage3, pMessage4, pMessage5, pMessage6, pMessage7, pMessage8, pMessage9);
	}
}

void EntityList::messageStringID(Mob* pSender, bool pSkipSender, uint32 pType, uint32 pStringID, const char* pMessage1, const char* pMessage2, const char* pMessage3, const char* pMessage4, const char* pMessage5, const char* pMessage6, const char* pMessage7, const char* pMessage8, const char* pMessage9) {
	Client* client;
	for (auto i = mClients.begin(); i != mClients.end(); ++i) {
		client = i->second;
		if (client && (!pSkipSender || client != pSender))
			client->Message_StringID(pType, pStringID, pMessage1, pMessage2, pMessage3, pMessage4, pMessage5, pMessage6, pMessage7, pMessage8, pMessage9);
	}
}

void EntityList::filteredMessageStringID(Mob* pSender, bool pSkipSender, uint32 pType, eqFilterType pFilter, uint32 pStringID, const char* pMessage1, const char* pMessage2, const char* pMessage3, const char* pMessage4, const char* pMessage5, const char* pMessage6, const char* pMessage7, const char* pMessage8, const char* pMessage9) {
	Client* client;
	for (auto i = mClients.begin(); i != mClients.end(); ++i) {
		client = i->second;
		if (client && (!pSkipSender || client != pSender))
			client->FilteredMessage_StringID(pSender, pType, pFilter, pStringID, pMessage1, pMessage2, pMessage3, pMessage4, pMessage5, pMessage6, pMessage7, pMessage8, pMessage9);
	}
}

void EntityList::messageClose(Mob* pSender, bool pSkipSender, float pDistance, uint32 pType, const char* pMessage, ...) {
	va_list argptr;
	char buffer[4096];

	va_start(argptr, pMessage);
	vsnprintf(buffer, 4095, pMessage, argptr);
	va_end(argptr);

	float dist2 = pDistance * pDistance;
	for (auto i = mClients.begin(); i != mClients.end(); i++) {
		if (i->second->DistNoRoot(*pSender) <= dist2 && (!pSkipSender || i->second != pSender))
			i->second->message(pType, buffer);
	}
}

void EntityList::removeAllMobs() {
	auto i = mMOBs.begin();
	while (i != mMOBs.end()) {
		safe_delete(i->second);
		mFreeIDs.push(i->first);
		i = mMOBs.erase(i);
	}
}

void EntityList::removeAllClients() {
	// doesn't clear the data
	mClients.clear();
}

void EntityList::removeAllNPCs() {
	// doesn't clear the data
	mNPCs.clear();
	mNPCLimits.clear();
}

void EntityList::removeAllGroups() {
	while (mGroups.size())
		mGroups.pop_front();
#if EQDEBUG >= 5
	checkGroupList(__FILE__, __LINE__);
#endif
}

void EntityList::removeAllRaids() {
	while (mRaids.size())
		mRaids.pop_front();
}

void EntityList::removeAllDoors() {
	auto i = mDoors.begin();
	while (i != mDoors.end()) {
		safe_delete(i->second);
		mFreeIDs.push(i->first);
		i = mDoors.erase(i);
	}
	despawnAllDoors();
}

void EntityList::despawnAllDoors() {
	EQApplicationPacket *outapp = new EQApplicationPacket(OP_RemoveAllDoors, 0);
	this->queueClients(0, outapp);
	safe_delete(outapp);
}

void EntityList::respawnAllDoors() {
	for (auto i = mClients.begin(); i != mClients.end(); i++) {
		if (i->second) {
			EQApplicationPacket *outapp = new EQApplicationPacket();
			makeDoorSpawnPacket(outapp, i->second);
			i->second->FastQueuePacket(&outapp);
		}
	}
}

void EntityList::removeAllCorpses() {
	auto i = mCorpses.begin();
	while (i != mCorpses.end()) {
		safe_delete(i->second);
		mFreeIDs.push(i->first);
		i = mCorpses.erase(i);
	}
}

void EntityList::removeAllObjects() {
	auto i = mObjects.begin();
	while (i != mObjects.end()) {
		safe_delete(i->second);
		mFreeIDs.push(i->first);
		i = mObjects.erase(i);
	}
}

void EntityList::removeAllTraps() {
	auto i = mTraps.begin();
	while (i != mTraps.end()) {
		safe_delete(i->second);
		mFreeIDs.push(i->first);
		i = mTraps.erase(i);
	}
}

bool EntityList::removeMOB(uint16 pID) {
	if (pID == 0)
		return true;

	auto it = mMOBs.find(pID);
	if (it != mMOBs.end()) {
		if (mNPCs.count(pID))
			entity_list.removeNPC(pID);
		else if (mClients.count(pID))
			entity_list.removeClient(pID);
		safe_delete(it->second);
		if (!mCorpses.count(pID))
			mFreeIDs.push(it->first);
		mMOBs.erase(it);
		return true;
	}
	return false;
}

// This is for if the ID is deleted for some reason
bool EntityList::removeMOB(Mob *pMOB) {
	if (pMOB == 0)
		return true;

	auto it = mMOBs.begin();
	while (it != mMOBs.end()) {
		if (it->second == pMOB) {
			safe_delete(it->second);
			if (!mCorpses.count(it->first))
				mFreeIDs.push(it->first);
			mMOBs.erase(it);
			return true;
		}
		++it;
	}
	return false;
}

bool EntityList::removeNPC(uint16 pID) {
	auto it = mNPCs.find(pID);
	if (it != mNPCs.end()) {
		// make sure its proximity is removed
		removeProximity(pID);
		// remove from the list
		mNPCs.erase(it);
		// remove from limit list if needed
		if (mNPCLimits.count(pID))
			mNPCLimits.erase(pID);
		return true;
	}
	return false;
}

bool EntityList::removeClient(uint16 pID) {
	auto it = mClients.find(pID);
	if (it != mClients.end()) {
		mClients.erase(it); // Already deleted
		return true;
	}
	return false;
}

// If our ID was deleted already
bool EntityList::removeClient(Client* pClient) {
	auto it = mClients.begin();
	while (it != mClients.end()) {
		if (it->second == pClient) {
			mClients.erase(it);
			return true;
		}
		++it;
	}
	return false;
}

bool EntityList::removeObject(uint16 pID) {
	auto i = mObjects.find(pID);
	if (i != mObjects.end()) {
		safe_delete(i->second);
		mFreeIDs.push(i->first);
		mObjects.erase(i);
		return true;
	}
	return false;
}

bool EntityList::removeTrap(uint16 pID) {
	auto i = mTraps.find(pID);
	if (i != mTraps.end()) {
		safe_delete(i->second);
		mFreeIDs.push(i->first);
		mTraps.erase(i);
		return true;
	}
	return false;
}

bool EntityList::removeDoor(uint16 pID) {
	auto i = mDoors.find(pID);
	if (i != mDoors.end()) {
		safe_delete(i->second);
		mFreeIDs.push(i->first);
		mDoors.erase(i);
		return true;
	}
	return false;
}

bool EntityList::removeCorpse(uint16 pID) {
	auto i = mCorpses.find(pID);
	if (i != mCorpses.end()) {
		safe_delete(i->second);
		mFreeIDs.push(i->first);
		mCorpses.erase(i);
		return true;
	}
	return false;
}

bool EntityList::removeGroup(uint32 pID) {
	for (auto i = mGroups.begin(); i != mGroups.end(); i++) {
		if ((*i)->GetID() == pID) {
			mGroups.remove(*i);
			return true;
		}
	}
	return false;
}

bool EntityList::removeRaid(uint32 pID) {
	for (auto i = mRaids.begin(); i != mRaids.end(); i++) {
		if ((*i)->GetID() == pID) {
			mRaids.remove(*i);
			return true;
		}
	}
	return false;
}

void EntityList::clear() {
	removeAllClients();
	entity_list.removeAllTraps(); //we can have child npcs so we go first
	entity_list.removeAllNPCs();
	entity_list.removeAllMobs();
	entity_list.removeAllCorpses();
	entity_list.removeAllGroups();
	entity_list.removeAllDoors();
	entity_list.removeAllObjects();
	entity_list.removeAllRaids();
	entity_list.removeAllLocalities();
}

void EntityList::updateWho(bool pSendFullUpdate) {
	if ((!worldserver.Connected()) || !ZoneLoaded)
		return;
	uint32 tmpNumUpdates = numclients + 5;
	ServerPacket* pack = 0;
	ServerClientListKeepAlive_Struct* sclka = 0;
	if (!pSendFullUpdate) {
		pack = new ServerPacket(ServerOP_ClientListKA, sizeof(ServerClientListKeepAlive_Struct)+(tmpNumUpdates * 4));
		sclka = (ServerClientListKeepAlive_Struct*)pack->pBuffer;
	}

	for (auto i = mClients.begin(); i != mClients.end(); i++) {
		if (i->second->isInZone()) {
			if (pSendFullUpdate) {
				i->second->UpdateWho();
			}
			else {
				if (sclka->numupdates >= tmpNumUpdates) {
					tmpNumUpdates += 10;
					uint8* tmp = pack->pBuffer;
					pack->pBuffer = new uint8[sizeof(ServerClientListKeepAlive_Struct)+(tmpNumUpdates * 4)];
					memset(pack->pBuffer, 0, sizeof(ServerClientListKeepAlive_Struct)+(tmpNumUpdates * 4));
					memcpy(pack->pBuffer, tmp, pack->size);
					pack->size = sizeof(ServerClientListKeepAlive_Struct)+(tmpNumUpdates * 4);
					safe_delete_array(tmp);
				}
				sclka->wid[sclka->numupdates] = i->second->GetWID();
				sclka->numupdates++;
			}
		}
	}
	if (!pSendFullUpdate) {
		pack->size = sizeof(ServerClientListKeepAlive_Struct)+(sclka->numupdates * 4);
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void EntityList::removeEntity(uint16 pID) {
	if (pID == 0)
		return;
	if (entity_list.removeMOB(pID))
		return;
	else if (entity_list.removeCorpse(pID))
		return;
	else if (entity_list.removeDoor(pID))
		return;
	else if (entity_list.removeGroup(pID))
		return;
	else if (entity_list.removeTrap(pID))
		return;
	else
		entity_list.removeObject(pID);
}

void EntityList::process() {
	checkSpawnQueue();
}

void EntityList::depop(bool pStartSpawnTimer) {
	for (auto& i : mNPCs) {
		NPC* npc = i.second;
		Mob* owner = npc->GetOwner();
		//do not depop player's pets...
		if (owner && owner->isClient())
			continue;

		if (npc->IsFindable())
			updateFindableNPCState(npc, true);

		npc->depop(pStartSpawnTimer);
	}
}

void EntityList::depopAll(int pNPCTypeID, bool pStartSpawnTimer) {
	for (auto& i : mNPCs) {
		NPC* npc = i.second;
		if (npc && (npc->GetNPCTypeID() == (uint32)pNPCTypeID))
			npc->depop(pStartSpawnTimer);
	}
}

void EntityList::sendTraders(Client* pClient) {
	Client* otherClient = nullptr;
	for (auto& i : mClients) {
		otherClient = i.second;
		if (otherClient->IsTrader())
			pClient->SendTraderPacket(otherClient);

		if (otherClient->IsBuyer())
			pClient->SendBuyerPacket(otherClient);
	}
}

void EntityList::removeFromHateLists(Mob* pMOB, bool pSetToOne) {
	for (auto& i : mNPCs) {
		if (i.second->CheckAggro(pMOB)) {
			if (!pSetToOne)
				i.second->RemoveFromHateList(pMOB);
			else
				i.second->SetHate(pMOB, 1);
		}
	}
}

void EntityList::removeDebuffs(Mob* pCaster) {
	for (auto& i : mMOBs) {
		i.second->BuffFadeDetrimentalByCaster(pCaster);
	}
}

// Currently, a new packet is sent per entity.
// @todo: Come back and use FLAG_COMBINED to pack
// all updates into one packet.
void EntityList::sendPositionUpdates(Client* pClient, uint32 pLastUpdate, float pRange, Entity* pAlwaysSend, bool pSendEvenIfNotChanged) {
	pRange = pRange * pRange;

	EQApplicationPacket* outapp = 0;
	PlayerPositionUpdateServer_Struct* ppu = 0;
	Mob* mob = 0;

	for (auto i = mMOBs.begin(); i != mMOBs.end(); i++) {
		if (outapp == 0) {
			outapp = new EQApplicationPacket(OP_ClientUpdate, sizeof(PlayerPositionUpdateServer_Struct));
			ppu = (PlayerPositionUpdateServer_Struct*)outapp->pBuffer;
		}
		mob = i->second;
		if (mob && !mob->isCorpse() && (i->second != pClient)
			&& (mob->isClient() || pSendEvenIfNotChanged || (mob->LastChange() >= pLastUpdate))
			&& (!i->second->isClient() || !i->second->castToClient()->GMHideMe(pClient))) {

			//bool Grouped = client->HasGroup() && mob->isClient() && (client->GetGroup() == mob->castToClient()->GetGroup());

			//if (range == 0 || (iterator.GetData() == alwayssend) || Grouped || (mob->DistNoRootNoZ(*client) <= range)) {
			if (pRange == 0 || (i->second == pAlwaysSend) || mob->isClient() || (mob->DistNoRoot(*pClient) <= pRange)) {
				mob->MakeSpawnUpdate(ppu);
			}
			if (mob && mob->isClient() && mob->getID() > 0) {
				pClient->QueuePacket(outapp, false, Client::CLIENT_CONNECTED);
			}
		}
		safe_delete(outapp);
		outapp = 0;
	}

	safe_delete(outapp);
}

char* EntityList::MakeNameUnique(char* pName) {
	bool used[300];
	memset(used, 0, sizeof(used));
	pName[61] = 0; pName[62] = 0; pName[63] = 0;

	int len = strlen(pName);
	for (auto i = mMOBs.begin(); i != mMOBs.end(); i++) {
		if (i->second->isMOB()) {
			if (strncasecmp(i->second->castToMOB()->getName(), pName, len) == 0) {
				if (Seperator::IsNumber(&i->second->castToMOB()->getName()[len])) {
					used[atoi(&i->second->castToMOB()->getName()[len])] = true;
				}
			}
		}
	}
	for (int i = 0; i < 300; i++) {
		if (!used[i]) {
#ifdef _WINDOWS
			snprintf(pName, 64, "%s%03d", pName, i);
#else
			//glibc clears destination of snprintf
			//make a copy of name before snprintf--misanthropicfiend
			char temp_name[64];
			strn0cpy(temp_name, pName, 64);
			snprintf(pName, 64, "%s%03d", temp_name, i);
#endif
			return pName;
		}
	}
	LogFile->write(EQEMuLog::Error, "Fatal error in EntityList::makeNameUnique: Unable to find unique name for '%s'", pName);
	char tmp[64] = "!";
	strn0cpy(&tmp[1], pName, sizeof(tmp)-1);
	strcpy(pName, tmp);
	return MakeNameUnique(pName);
}

char* EntityList::removeNumbers(char* pName) {
	char tmp[64];
	memset(tmp, 0, sizeof(tmp));
	int k = 0;
	for (unsigned int i = 0; i < strlen(pName) && i < sizeof(tmp); i++) {
		if (pName[i] < '0' || pName[i] > '9')
			tmp[k++] = pName[i];
	}
	strn0cpy(pName, tmp, sizeof(tmp));
	return pName;
}

void EntityList::listNPCCorpses(Client* pClient) {
	int count = 0;
	pClient->message(0, "NPC Corpses in the zone:");
	for (auto i = mCorpses.begin(); i != mCorpses.end(); i++) {
		if (i->second->isNPCCorpse()) {
			pClient->message(0, "  %5d: %s", i->first, i->second->getName());
			count++;
		}
	}
	pClient->message(0, "%d npc corpses listed.", count);
}

void EntityList::listPlayerCorpses(Client* pClient) {
	int count = 0;
	pClient->message(0, "Player Corpses in the zone:");
	for (auto i = mCorpses.begin(); i != mCorpses.end(); i++) {
		if (i->second->isPlayerCorpse()) {
			pClient->message(0, "  %5d: %s", i->first, i->second->getName());
			count++;
		}
	}
	pClient->message(0, "%d player corpses listed.", count);
}

// returns the number of corpses deleted.
int32 EntityList::deleteNPCCorpses() {
	int32 count = 0;
	for (auto &i : mCorpses) {
		if (i.second->isNPCCorpse()) {
			i.second->depop();
			count++;
		}
	}
	return count;
}

// returns the number of corpses deleted.
int32 EntityList::deletePlayerCorpses() {
	int32 count = 0;
	for (auto &i : mCorpses) {
		if (i.second->isPlayerCorpse()) {
			i.second->castToCorpse()->Delete();
			count++;
		}
	}
	return count;
}

void EntityList::sendPetitionToAdmins() {
	EQApplicationPacket *outapp = new EQApplicationPacket(OP_PetitionUpdate, sizeof(PetitionUpdate_Struct));
	PetitionUpdate_Struct *pcus = (PetitionUpdate_Struct*)outapp->pBuffer;
	pcus->petnumber = 0;		// Petition Number
	pcus->color = 0;
	pcus->status = 0xFFFFFFFF;
	pcus->senttime = 0;
	strcpy(pcus->accountid, "");
	strcpy(pcus->gmsenttoo, "");
	pcus->quetotal = 0;
	
	for (auto i = mClients.begin(); i != mClients.end(); i++) {
		if (i->second->castToClient()->Admin() >= 80)
			i->second->castToClient()->QueuePacket(outapp);
	}
	safe_delete(outapp);
}

void EntityList::sendPetitionToAdmins(Petition* pPetition) {
	EQApplicationPacket *outapp = new EQApplicationPacket(OP_PetitionUpdate, sizeof(PetitionUpdate_Struct));
	PetitionUpdate_Struct *pcus = (PetitionUpdate_Struct*)outapp->pBuffer;
	pcus->petnumber = pPetition->GetID();		// Petition Number
	if (pPetition->CheckedOut()) {
		pcus->color = 0x00;
		pcus->status = 0xFFFFFFFF;
		pcus->senttime = pPetition->GetSentTime();
		strcpy(pcus->accountid, "");
		strcpy(pcus->gmsenttoo, "");
	}
	else {
		pcus->color = pPetition->GetUrgency();	// 0x00 = green, 0x01 = yellow, 0x02 = red
		pcus->status = pPetition->GetSentTime();
		pcus->senttime = pPetition->GetSentTime();			// 4 has to be 0x1F
		strcpy(pcus->accountid, pPetition->GetAccountName());
		strcpy(pcus->charname, pPetition->GetCharName());
	}
	pcus->quetotal = petition_list.GetTotalPetitions();
	
	for (auto i = mClients.begin(); i != mClients.end(); i++) {
		if (i->second->castToClient()->Admin() >= 80) {
			if (pPetition->CheckedOut())
				strcpy(pcus->gmsenttoo, "");
			else
				strcpy(pcus->gmsenttoo, i->second->castToClient()->getName());
			i->second->castToClient()->QueuePacket(outapp);
		}
	}
	safe_delete(outapp);
}

void EntityList::clearClientPetitionQueue() {
	EQApplicationPacket *outapp = new EQApplicationPacket(OP_PetitionUpdate, sizeof(PetitionUpdate_Struct));
	PetitionUpdate_Struct *pet = (PetitionUpdate_Struct*)outapp->pBuffer;
	pet->color = 0x00;
	pet->status = 0xFFFFFFFF;
	pet->senttime = 0;
	strcpy(pet->accountid, "");
	strcpy(pet->gmsenttoo, "");
	strcpy(pet->charname, "");
	pet->quetotal = petition_list.GetTotalPetitions();
	auto it = mClients.begin();
	while (it != mClients.end()) {
		if (it->second->castToClient()->Admin() >= 100) {
			int x = 0;
			for (x = 0; x < 64; x++) {
				pet->petnumber = x;
				it->second->castToClient()->QueuePacket(outapp);
			}
		}
		++it;
	}
	safe_delete(outapp);
	return;
}

BulkZoneSpawnPacket::BulkZoneSpawnPacket(Client* iSendTo, uint32 iMaxSpawnsPerPacket)
{
	data = 0;
	pSendTo = iSendTo;
	pMaxSpawnsPerPacket = iMaxSpawnsPerPacket;
}

BulkZoneSpawnPacket::~BulkZoneSpawnPacket()
{
	SendBuffer();
	safe_delete_array(data)
}

bool BulkZoneSpawnPacket::AddSpawn(NewSpawn_Struct *ns)
{
	if (!data) {
		data = new NewSpawn_Struct[pMaxSpawnsPerPacket];
		memset(data, 0, sizeof(NewSpawn_Struct)* pMaxSpawnsPerPacket);
		index = 0;
	}
	memcpy(&data[index], ns, sizeof(NewSpawn_Struct));
	index++;
	if (index >= pMaxSpawnsPerPacket) {
		SendBuffer();
		return true;
	}
	return false;
}

void BulkZoneSpawnPacket::SendBuffer() {
	if (!data)
		return;

	uint32 tmpBufSize = (index * sizeof(NewSpawn_Struct));
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_ZoneSpawns, (unsigned char *)data, tmpBufSize);

	if (pSendTo) {
		pSendTo->FastQueuePacket(&outapp);
	}
	else {
		entity_list.queueClients(0, outapp);
		safe_delete(outapp);
	}
	memset(data, 0, sizeof(NewSpawn_Struct)* pMaxSpawnsPerPacket);
	index = 0;
}

//removes "targ" from all hate lists, including feigned, in the zone
void EntityList::clearAggro(Mob* pTarget) {
	auto i = mNPCs.begin();
	while (i != mNPCs.end()) {
		if (i->second->CheckAggro(pTarget))
			i->second->RemoveFromHateList(pTarget);
		i->second->RemoveFromFeignMemory(pTarget->castToClient()); //just in case we feigned
		++i;
	}
}

void EntityList::clearFeignAggro(Mob* pTarget) {
	auto i = mNPCs.begin();
	while (i != mNPCs.end()) {
		if (i->second->CheckAggro(pTarget)) {
			if (i->second->GetSpecialAbility(IMMUNE_FEIGN_DEATH)) {
				++i;
				continue;
			}

			if (pTarget->isClient()) {
				std::vector<void*> args;
				args.push_back(i->second);
				if (parse->EventPlayer(EVENT_FEIGN_DEATH, pTarget->castToClient(), "", 0, &args) != 0) {
					++i;
					continue;
				}

				if (i->second->isNPC()) {
					if (parse->EventNPC(EVENT_FEIGN_DEATH, i->second->castToNPC(), pTarget, "", 0) != 0) {
						++i;
						continue;
					}
				}
			}

			i->second->RemoveFromHateList(pTarget);
			if (pTarget->isClient()) {
				if (i->second->GetLevel() >= 35 && MakeRandomInt(1, 100) <= 60)
					i->second->AddFeignMemory(pTarget->castToClient());
				else
					pTarget->castToClient()->RemoveXTarget(i->second, false);
			}
		}
		++i;
	}
}

void EntityList::clearZoneFeignAggro(Client* pTarget) {
	for (auto& i : mNPCs) {
		i.second->RemoveFromFeignMemory(pTarget);
		pTarget->castToClient()->RemoveXTarget(i.second, false);
	}
}

void EntityList::aggroZone(Mob* pTarget, int pHate) {
	for (auto& i : mNPCs) {
		i.second->AddToHateList(pTarget, pHate);
	}
}

// Signal Quest command function
void EntityList::signalMOBsByNPCID(uint32 pNPCTypeID, int pSignalID) {	
	for (auto i = mNPCs.begin(); i != mNPCs.end(); i++) {
		NPC* npc = i->second;
		if (npc->GetNPCTypeID() == pNPCTypeID)
			npc->SignalNPC(pSignalID);
	}
}

bool EntityList::makeTrackPacket(Client* pClient) {
	uint32 distance = 0;
	float MobDistance;

	if (pClient->GetClass() == DRUID)
		distance = (pClient->GetSkill(SkillTracking) * 10);
	else if (pClient->GetClass() == RANGER)
		distance = (pClient->GetSkill(SkillTracking) * 12);
	else if (pClient->GetClass() == BARD)
		distance = (pClient->GetSkill(SkillTracking) * 7);
	if (distance <= 0)
		return false;
	if (distance < 300)
		distance = 300;

	uint32 spe = 0;
	bool ret = false;

	spe = mMOBs.size() + 50;

	uchar *buffer1 = new uchar[sizeof(Track_Struct)];
	Track_Struct *track_ent = (Track_Struct*)buffer1;

	uchar *buffer2 = new uchar[sizeof(Track_Struct)*spe];
	Tracking_Struct *track_array = (Tracking_Struct*)buffer2;
	memset(track_array, 0, sizeof(Track_Struct)*spe);

	uint32 array_counter = 0;

	Group *g = pClient->GetGroup();

	auto it = mMOBs.begin();
	while (it != mMOBs.end()) {
		if (it->second && ((MobDistance = it->second->DistNoZ(*pClient)) <= distance)) {
			if ((it->second != pClient) && it->second->IsTrackable()) {
				memset(track_ent, 0, sizeof(Track_Struct));
				Mob *cur_entity = it->second;
				track_ent->entityid = cur_entity->getID();
				track_ent->distance = MobDistance;
				track_ent->level = cur_entity->GetLevel();
				track_ent->NPC = !cur_entity->isClient();
				if (g && cur_entity->isClient() && g->isGroupMember(cur_entity->castToMOB()))
					track_ent->GroupMember = 1;
				else
					track_ent->GroupMember = 0;
				strn0cpy(track_ent->name, cur_entity->getName(), sizeof(track_ent->name));
				memcpy(&track_array->Entrys[array_counter], track_ent, sizeof(Track_Struct));
				array_counter++;
			}
		}

		++it;
	}

	if (array_counter <= spe) {
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_Track, sizeof(Track_Struct)*(array_counter));
		memcpy(outapp->pBuffer, track_array, sizeof(Track_Struct)*(array_counter));
		outapp->priority = 6;
		pClient->QueuePacket(outapp);
		safe_delete(outapp);
		ret = true;
	}
	else {
		LogFile->write(EQEMuLog::Status, "ERROR: Unable to transmit a Tracking_Struct packet. Mobs in zone = %i. Mobs in packet = %i", array_counter, spe);
	}

	safe_delete_array(buffer1);
	safe_delete_array(buffer2);

	return ret;
}

void EntityList::messageGroup(Mob* pSender, bool pSkipClose, uint32 pType, const char* pMessage, ...)
{
	va_list argptr;
	char buffer[4096];

	va_start(argptr, pMessage);
	vsnprintf(buffer, 4095, pMessage, argptr);
	va_end(argptr);

	float dist2 = 100;

	if (pSkipClose)
		dist2 = 0;

	for (auto& i : mClients) {
		if (i.second != pSender && (i.second->Dist(*pSender) <= dist2 || i.second->GetGroup() == pSender->castToClient()->GetGroup()))
			i.second->message(pType, buffer);
	}
}

bool EntityList::isFighting(Mob* pTarget) {
	for (auto& i : mNPCs) {
		if (i.second->CheckAggro(pTarget))
			return true;
	}
	return false;
}

void EntityList::addHealAggro(Mob* pTarget, Mob* pCaster, uint16 pDamage) {
	NPC *cur = nullptr;
	uint16 count = 0;
	std::list<NPC *> npc_sub_list;
	auto it = mNPCs.begin();
	while (it != mNPCs.end()) {
		cur = it->second;

		if (!cur->CheckAggro(pTarget)) {
			++it;
			continue;
		}
		if (!cur->IsMezzed() && !cur->IsStunned() && !cur->IsFeared()) {
			npc_sub_list.push_back(cur);
			++count;
		}
		++it;
	}


	if (pDamage > 1) {
		if (count > 0)
			pDamage /= count;

		if (pDamage < 1)
			pDamage = 1;
	}

	cur = nullptr;
	auto sit = npc_sub_list.begin();
	while (sit != npc_sub_list.end()) {
		cur = *sit;

		if (cur->IsPet()) {
			if (pCaster) {
				if (cur->CheckAggro(pCaster)) {
					cur->AddToHateList(pCaster, pDamage);
				}
			}
		}
		else {
			if (pCaster) {
				if (cur->CheckAggro(pCaster)) {
					cur->AddToHateList(pCaster, pDamage);
				}
				else {
					cur->AddToHateList(pCaster, pDamage * 0.33);
				}
			}
		}
		++sit;
	}
}

void EntityList::openDoorsNear(NPC* pOpener) {
	for (auto i = mDoors.begin(); i != mDoors.end(); i++) {
		Doors* cdoor = i->second;
		if (cdoor && !cdoor->IsDoorOpen()) {
			float zdiff = pOpener->GetZ() - cdoor->GetZ();
			if (zdiff < 0)
				zdiff = 0 - zdiff;
			float curdist = 0;
			float tmp = pOpener->GetX() - cdoor->GetX();
			curdist += tmp * tmp;
			tmp = pOpener->GetY() - cdoor->GetY();
			curdist += tmp * tmp;
			if (zdiff < 10 && curdist <= 100)
				cdoor->NPCOpen(pOpener);
		}
	}
}

void EntityList::sendAlarm(Trap* pTrap, Mob* pCurrentTarget, uint8 pKOS) {
	float val2 = pTrap->effectvalue * pTrap->effectvalue;

	auto i = mNPCs.begin();
	while (i != mNPCs.end()) {
		NPC *cur = i->second;
		float curdist = 0;
		float tmp = cur->GetX() - pTrap->x;
		curdist += tmp*tmp;
		tmp = cur->GetY() - pTrap->y;
		curdist += tmp*tmp;
		tmp = cur->GetZ() - pTrap->z;
		curdist += tmp*tmp;
		if (!cur->GetOwner() &&
			/*!cur->castToMOB()->dead && */
			!cur->IsEngaged() &&
			curdist <= val2)
		{
			if (pKOS) {
				uint8 factioncon = pCurrentTarget->GetReverseFactionCon(cur);
				if (factioncon == FACTION_THREATENLY || factioncon == FACTION_SCOWLS) {
					cur->AddToHateList(pCurrentTarget, 1);
				}
			}
			else {
				cur->AddToHateList(pCurrentTarget, 1);
			}
		}
		++i;
	}
}

void EntityList::addProximity(NPC* pProximityFor)
{
	removeProximity(pProximityFor->getID());
	mProximities.push_back(pProximityFor);
	pProximityFor->proximity = new NPCProximity;
}

bool EntityList::removeProximity(uint16 pID) {
	for (auto i = mProximities.begin(); i != mProximities.end(); i++) {
		if ((*i)->getID() == pID) {
			mProximities.erase(i);
			return true;
		}
	}
	return false;
}

void EntityList::removeAllLocalities() {
	mProximities.clear();
}

struct quest_proximity_event {
	QuestEventID event_id;
	Client* client;
	NPC *npc;
	int area_id;
	int area_type;
};

void EntityList::processMove(Client* pClient, float pX, float pY, float pZ) {
	float last_x = pClient->ProximityX();
	float last_y = pClient->ProximityY();
	float last_z = pClient->ProximityZ();

	std::list<quest_proximity_event> events;
	for (auto iter = mProximities.begin(); iter != mProximities.end(); ++iter) {
		NPC *d = (*iter);
		NPCProximity *l = d->proximity;
		if (l == nullptr)
			continue;

		//check both bounding boxes, if either coords pairs
		//cross a boundary, send the event.
		bool old_in = true;
		bool new_in = true;
		if (last_x < l->min_x || last_x > l->max_x ||
			last_y < l->min_y || last_y > l->max_y ||
			last_z < l->min_z || last_z > l->max_z) {
			old_in = false;
		}
		if (pX < l->min_x || pX > l->max_x ||
			pY < l->min_y || pY > l->max_y ||
			pZ < l->min_z || pZ > l->max_z) {
			new_in = false;
		}

		if (old_in && !new_in) {
			quest_proximity_event evt;
			evt.event_id = EVENT_EXIT;
			evt.client = pClient;
			evt.npc = d;
			evt.area_id = 0;
			evt.area_type = 0;
			events.push_back(evt);
		}
		else if (new_in && !old_in) {
			quest_proximity_event evt;
			evt.event_id = EVENT_ENTER;
			evt.client = pClient;
			evt.npc = d;
			evt.area_id = 0;
			evt.area_type = 0;
			events.push_back(evt);
		}
	}

	for (auto iter = mAreas.begin(); iter != mAreas.end(); ++iter) {
		Area& a = (*iter);
		bool old_in = true;
		bool new_in = true;
		if (last_x < a.mMinX || last_x > a.mMaxX ||
			last_y < a.mMinY || last_y > a.mMaxY ||
			last_z < a.mMinZ || last_z > a.mMaxZ) {
			old_in = false;
		}

		if (pX < a.mMinX || pX > a.mMaxX ||
			pY < a.mMinY || pY > a.mMaxY ||
			pZ < a.mMinZ || pZ > a.mMaxZ) {
			new_in = false;
		}

		if (old_in && !new_in) {
			//were in but are no longer.
			quest_proximity_event evt;
			evt.event_id = EVENT_LEAVE_AREA;
			evt.client = pClient;
			evt.npc = nullptr;
			evt.area_id = a.mID;
			evt.area_type = a.mType;
			events.push_back(evt);
		}
		else if (!old_in && new_in) {
			//were not in but now are
			quest_proximity_event evt;
			evt.event_id = EVENT_ENTER_AREA;
			evt.client = pClient;
			evt.npc = nullptr;
			evt.area_id = a.mID;
			evt.area_type = a.mType;
			events.push_back(evt);
		}
	}

	for (auto iter = events.begin(); iter != events.end(); ++iter) {
		quest_proximity_event& evt = (*iter);
		if (evt.npc) {
			parse->EventNPC(evt.event_id, evt.npc, evt.client, "", 0);
		}
		else {
			std::vector<void*> args;
			args.push_back(&evt.area_id);
			args.push_back(&evt.area_type);
			parse->EventPlayer(evt.event_id, evt.client, "", 0, &args);
		}
	}
}

void EntityList::processMove(NPC* pNPC, float pX, float pY, float pZ) {
	float last_x = pNPC->GetX();
	float last_y = pNPC->GetY();
	float last_z = pNPC->GetZ();

	std::list<quest_proximity_event> events;
	for (auto iter = mAreas.begin(); iter != mAreas.end(); ++iter) {
		Area& a = (*iter);
		bool old_in = true;
		bool new_in = true;
		if (last_x < a.mMinX || last_x > a.mMaxX ||
			last_y < a.mMinY || last_y > a.mMaxY ||
			last_z < a.mMinZ || last_z > a.mMaxZ) {
			old_in = false;
		}

		if (pX < a.mMinX || pX > a.mMaxX ||
			pY < a.mMinY || pY > a.mMaxY ||
			pZ < a.mMinZ || pZ > a.mMaxZ) {
			new_in = false;
		}

		if (old_in && !new_in) {
			//were in but are no longer.
			quest_proximity_event evt;
			evt.event_id = EVENT_LEAVE_AREA;
			evt.client = nullptr;
			evt.npc = pNPC;
			evt.area_id = a.mID;
			evt.area_type = a.mType;
			events.push_back(evt);
		}
		else if (!old_in && new_in) {
			//were not in but now are
			quest_proximity_event evt;
			evt.event_id = EVENT_ENTER_AREA;
			evt.client = nullptr;
			evt.npc = pNPC;
			evt.area_id = a.mID;
			evt.area_type = a.mType;
			events.push_back(evt);
		}
	}

	for (auto iter = events.begin(); iter != events.end(); ++iter) {
		quest_proximity_event& evt = (*iter);
		std::vector<void*> args;
		args.push_back(&evt.area_id);
		args.push_back(&evt.area_type);
		parse->EventNPC(evt.event_id, evt.npc, evt.client, "", 0, &args);
	}
}

void EntityList::addArea(int pID, int pType, float pMinX, float pMaxX, float pMinY, float pMaxY, float pMinZ, float pMaxZ) {
	removeArea(pID);
	Area a;
	a.mID = pID;
	a.mType = pType;
	if (pMinX > pMaxX) {
		a.mMinX = pMaxX;
		a.mMaxX = pMinX;
	}
	else {
		a.mMinX = pMinX;
		a.mMaxX = pMaxX;
	}

	if (pMinY > pMaxY) {
		a.mMinY = pMaxY;
		a.mMaxY = pMinY;
	}
	else {
		a.mMinY = pMinY;
		a.mMaxY = pMaxY;
	}

	if (pMinZ > pMaxZ) {
		a.mMinZ = pMaxZ;
		a.mMaxZ = pMinZ;
	}
	else {
		a.mMinZ = pMinZ;
		a.mMaxZ = pMaxZ;
	}

	mAreas.push_back(a);
}

void EntityList::removeArea(int pID) {
	for (auto i = mAreas.begin(); i != mAreas.end(); i++) {
		if ((*i).mID == pID) {
			mAreas.erase(i);
			return;
		}
	}
}

void EntityList::clearAreas()
{
	mAreas.clear();
}

void EntityList::processProximitySay(const char* pMessage, Client* pClient, uint8 pLanguage) {
	if (!pMessage || !pClient)
		return;

	for (auto i = mProximities.begin(); i != mProximities.end(); ++i) {
		NPC *d = (*i);
		NPCProximity *l = d->proximity;
		if (l == nullptr || !l->say)
			continue;

		if (pClient->GetX() < l->min_x || pClient->GetX() > l->max_x
			|| pClient->GetY() < l->min_y || pClient->GetY() > l->max_y
			|| pClient->GetZ() < l->min_z || pClient->GetZ() > l->max_z)
			continue;

		parse->EventNPC(EVENT_PROXIMITY_SAY, d, pClient, pMessage, pLanguage);
	}
}

void EntityList::saveAllClientsTaskState() {
	if (!taskmanager) return;

	for (auto& i : mClients) {
		if (i.second->IsTaskStateLoaded())
			i.second->SaveTaskState();
	}
}

void EntityList::reloadAllClientsTaskState(int pTaskID) {
	if (!taskmanager) return;

	for (auto& i : mClients) {
		Client* client = i.second;
		// If we have been passed a TaskID, only reload the client state if they have that Task active.
		if ((!pTaskID) || (pTaskID && client->IsTaskActive(pTaskID))) {
			_log(TASKS__CLIENTLOAD, "Reloading Task State For Client %s", client->getName());
			client->RemoveClientTaskState();
			client->LoadClientTaskState();
			taskmanager->SendActiveTasksToClient(client);
		}
	}
}

bool EntityList::isMOBInZone(Mob* pMOB)
{
	//We don't use mob_list.find(who) because this code needs to be able to handle dangling pointers for the quest code.
	for (auto i = mMOBs.begin(); i != mMOBs.end(); i++) {
		if (i->second == pMOB)
			return true;
	}
	return false;
}

/*
Code to limit the amount of certain NPCs in a given zone.
Primarily used to make a named mob unique within the zone, but written
to be more generic allowing limits larger than 1.

Maintain this stuff in a separate list since the number
of limited NPCs will most likely be much smaller than the number
of NPCs in the entire zone.
*/
void EntityList::limitAddNPC(NPC* pNPC) {
	if (!pNPC) return;

	SpawnLimitRecord r;

	uint16 eid = pNPC->getID();
	r.mSpawnGroupID = pNPC->GetSp2();
	r.mNPCTypeID = pNPC->GetNPCTypeID();

	mNPCLimits[eid] = r;
}

void EntityList::limitRemoveNPC(NPC* pNPC) {
	if (!pNPC) return;

	uint16 eid = pNPC->getID();
	mNPCLimits.erase(eid);
}

//check a limit over the entire zone.
//returns true if the limit has not been reached
bool EntityList::limitCheckType(uint32 pNPCTypeID, int pCount) {
	if (pCount < 1)
		return true;

	for (auto& i : mNPCLimits) {
		if (i.second.mNPCTypeID == pNPCTypeID) {
			pCount--;
			if (pCount == 0) return false;
		}
	}
	return true;
}

//check limits on an npc type in a given spawn group.
//returns true if the limit has not been reached
bool EntityList::limitCheckGroup(uint32 pSpawnGroupID, int pCount) {
	if (pCount < 1)
		return true;

	for (auto& i : mNPCLimits) {
		if (i.second.mSpawnGroupID == pSpawnGroupID) {
			pCount--;
			if (pCount == 0) return false;
		}
	}
	return true;
}

bool EntityList::limitCheckName(const char* pName) {
	for (auto& i : mNPCs) {
		if (strcasecmp(pName, i.second->GetRawNPCTypeName()) == 0)
			return false;
	}
	return true;
}

void EntityList::radialSetLogging(Mob* pAround, bool pEnabled, bool pClients, bool pNonClients, float pRange)
{
	float range2 = pRange * pRange;

	auto i = mMOBs.begin();
	while (i != mMOBs.end()) {
		Mob *mob = i->second;

		++i;

		if (mob->isClient()) {
			if (!pClients)
				continue;
		}
		else {
			if (!pNonClients)
				continue;
		}

		if (pAround->DistNoRoot(*mob) > range2)
			continue;

		if (pEnabled)
			mob->EnableLogging();
		else
			mob->DisableLogging();
	}
}

void EntityList::updateHoTT(Mob* pTarget) {
	for (auto& i : mClients) {
		Client* client = i.second;
		if (client->GetTarget() == pTarget) {
			if (pTarget->GetTarget())
				client->SetHoTT(pTarget->GetTarget()->getID());
			else
				client->SetHoTT(0);

			client->UpdateXTargetType(TargetsTarget, pTarget->GetTarget());
		}
	}
}

void EntityList::destroyTempPets(Mob* pOwner) {
	for (auto i = mNPCs.begin(); i != mNPCs.end(); i++) {
		NPC* n = i->second;
		if (n->GetSwarmInfo()) {
			if (n->GetSwarmInfo()->owner_id == pOwner->getID()) {
				n->depop();
			}
		}
	}
}

bool Entity::checkCoordLosNoZLeaps(float cur_x, float cur_y, float cur_z,
	float trg_x, float trg_y, float trg_z, float perwalk)
{
	if (zone->zonemap == nullptr)
		return true;

	Map::Vertex myloc;
	Map::Vertex oloc;
	Map::Vertex hit;

	myloc.x = cur_x;
	myloc.y = cur_y;
	myloc.z = cur_z + 5;

	oloc.x = trg_x;
	oloc.y = trg_y;
	oloc.z = trg_z + 5;

	if (myloc.x == oloc.x && myloc.y == oloc.y && myloc.z == oloc.z)
		return true;

	if (!zone->zonemap->LineIntersectsZoneNoZLeaps(myloc, oloc, perwalk, &hit))
		return true;
	return false;
}

void EntityList::questJournalledSayClose(Mob *pSender, Client* pQuestIntiator, float pDistance, const char* pMobName, const char* pMessage) {
	Client* c = nullptr;
	float dist2 = pDistance * pDistance;

	// Send the message to the quest initiator such that the client will enter it into the NPC Quest Journal
	if (pQuestIntiator) {
		char *buf = new char[strlen(pMobName) + strlen(pMessage) + 10];
		sprintf(buf, "%s says, '%s'", pMobName, pMessage);
		pQuestIntiator->QuestJournalledMessage(pMobName, buf);
		safe_delete_array(buf);
	}
	// Use the old method for all other nearby clients
	for (auto it = mClients.begin(); it != mClients.end(); ++it) {
		c = it->second;
		if (c && (c != pQuestIntiator) && c->DistNoRoot(*pSender) <= dist2)
			c->Message_StringID(10, GENERIC_SAY, pMobName, pMessage);
	}
}

Corpse* EntityList::getClosestCorpse(Mob* pSender, const char* pName) {
	if (!pSender) return nullptr;

	uint32 closestDistance = 4294967295u;
	Corpse* closestCorpse = nullptr;

	for (auto& i : mCorpses) {
		Corpse* currentCorpse = i.second;

		if (pName && strcasecmp(currentCorpse->GetOwnerName(), pName))
			continue;

		uint32 currentDistance = ((currentCorpse->GetY() - pSender->GetY()) * (currentCorpse->GetY() - pSender->GetY())) +
			((currentCorpse->GetX() - pSender->GetX()) * (currentCorpse->GetX() - pSender->GetX()));

		if (currentDistance < closestDistance) {
			closestDistance = currentDistance;

			closestCorpse = currentCorpse;

		}
	}
	return closestCorpse;
}

void EntityList::forceGroupUpdate(uint32 pGroupID) {
	for (auto& i : mClients) {
		Group* group = i.second->GetGroup();
		if (group && group->GetID() == pGroupID)
			database.RefreshGroupFromDB(i.second);
	}
}

void EntityList::sendGroupLeave(uint32 pGroupID, const char* pName) {
	for (auto& i : mClients) {
		Client* client = i.second;
		Group * group = client->GetGroup();
		if (group && group->GetID() == pGroupID) {
			EQApplicationPacket* outapp = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupJoin_Struct));
			GroupJoin_Struct* gj = (GroupJoin_Struct*)outapp->pBuffer;
			strcpy(gj->membername, pName);
			gj->action = groupActLeave;
			strcpy(gj->yourname, client->getName());
			Mob *Leader = group->getLeader();
			if (Leader)
				Leader->castToClient()->GetGroupAAs(&gj->leader_aas);
			client->QueuePacket(outapp);
			safe_delete(outapp);
			group->delMemberOOZ(pName);
			if (group->isLeader(client) && client->IsLFP())
				client->UpdateLFP();
		}
	}
}

void EntityList::sendGroupJoin(uint32 pGroupID, const char* pName) {
	for (auto& i : mClients) {
		Client* client = i.second;
		Group* group = client->GetGroup();
		if (group && group->GetID() == pGroupID) {
			EQApplicationPacket* outapp = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupJoin_Struct));
			GroupJoin_Struct* gj = (GroupJoin_Struct*)outapp->pBuffer;
			strcpy(gj->membername, pName);
			gj->action = groupActJoin;
			strcpy(gj->yourname, client->getName());
			Mob *Leader = group->getLeader();
			if (Leader)
				Leader->castToClient()->GetGroupAAs(&gj->leader_aas);

			client->QueuePacket(outapp);
			safe_delete(outapp);
		}
	}
}

void EntityList::groupMessage(uint32 pGroupID, const char* pFrom, const char* pMessage) {
	for (auto& i : mClients) {
		Client* client = i.second;
		Group* group = client->GetGroup();
		if (group && group->GetID() == pGroupID)
			client->ChannelMessageSend(pFrom, client->getName(), 2, 0, pMessage);
	}
}

uint16 EntityList::createGroundObject(uint32 pItemID, float pX, float pY, float pZ, float pHeading, uint32 pDecayTime) {
	const Item_Struct *is = database.GetItem(pItemID);
	if (is) {
		ItemInst *i = new ItemInst(is, is->MaxCharges);
		if (i) {
			Object *object = new Object(i, pX, pY, pZ, pHeading, pDecayTime);
			entity_list.addObject(object, true);

			safe_delete(i);
			if (object)
				return object->getID();
		}
		return 0;
	}
	return 0;
}

uint16 EntityList::createGroundObjectFromModel(const char* pModel, float pX, float pY, float pZ, float pHeading, uint8 pType, uint32 pDecayTime) {
	if (pModel) {
		Object *object = new Object(pModel, pX, pY, pZ, pHeading, pType);
		entity_list.addObject(object, true);

		if (object)
			return object->getID();
	}
	return 0;
}

uint16 EntityList::createDoor(const char* pModel, float pX, float pY, float pZ,	float pHeading, uint8 pType, uint16 pSize) {
	if (pModel) {
		Doors* door = new Doors(pModel, pX, pY, pZ, pHeading, pType, pSize);
		removeAllDoors();
		zone->LoadZoneDoors(zone->GetShortName(), zone->GetInstanceVersion());
		entity_list.addDoor(door);
		entity_list.respawnAllDoors();

		if (door)
			return door->GetEntityID();
	}
	return 0;
}


Mob* EntityList::getTargetForMez(Mob* pCaster) {
	if (!pCaster)
		return nullptr;

	auto it = mMOBs.begin();
	//TODO: make this smarter and not mez targets being damaged by dots
	while (it != mMOBs.end()) {
		Mob *d = it->second;
		if (d) {
			if (d == pCaster) { //caster can't pick himself
				++it;
				continue;
			}

			if (pCaster->GetTarget() == d) { //caster can't pick his target
				++it;
				continue;
			}

			if (!pCaster->CheckAggro(d)) { //caster can't pick targets that aren't aggroed on himself
				++it;
				continue;
			}

			if (pCaster->DistNoRoot(*d) > 22250) { //only pick targets within 150 range
				++it;
				continue;
			}

			if (!pCaster->CheckLosFN(d)) {	//this is wasteful but can't really think of another way to do it
				++it;						//that wont have us trying to los the same target every time
				continue;					//it's only in combat so it's impact should be minimal.. but stil.
			}
			return d;
		}
		++it;
	}
	return nullptr;
}

void EntityList::sendZoneAppearance(Client* pClient) {
	if (!pClient) return;
	for (auto i = mMOBs.begin(); i != mMOBs.end(); i++) {
		Mob *cur = i->second;
		if (cur) {
			if (cur == pClient) continue;
			if (cur->GetAppearance() != eaStanding) {
				cur->SendAppearancePacket(AT_Anim, cur->GetAppearanceValue(cur->GetAppearance()), false, true, pClient);
			}
			if (cur->GetSize() != cur->GetBaseSize()) {
				cur->SendAppearancePacket(AT_Size, (uint32)cur->GetSize(), false, true, pClient);
			}
		}
	}
}

void EntityList::sendNimbusEffects(Client* pClient) {
	if (!pClient) return;
	for (auto i = mMOBs.begin(); i != mMOBs.end(); i++) {
		Mob *cur = i->second;
		if (cur) {
			if (cur == pClient) continue;
			if (cur->GetNimbusEffect1() != 0) {
				cur->SendSpellEffect(cur->GetNimbusEffect1(), 1000, 0, 1, 3000, false, pClient);
			}
			if (cur->GetNimbusEffect2() != 0) {
				cur->SendSpellEffect(cur->GetNimbusEffect2(), 2000, 0, 1, 3000, false, pClient);
			}
			if (cur->GetNimbusEffect3() != 0) {
				cur->SendSpellEffect(cur->GetNimbusEffect3(), 3000, 0, 1, 3000, false, pClient);
			}
		}
	}
}

void EntityList::sendUntargetable(Client* pClient) {
	if (!pClient) return;
	for (auto i = mMOBs.begin(); i != mMOBs.end(); i++) {
		Mob* cur = i->second;
		if (cur) {
			if (cur == pClient) continue;
			if (!cur->IsTargetable())
				cur->SendTargetable(false, pClient);
		}
	}
}

void EntityList::zoneWho(Client* pClient, Who_All_Struct* pWho) {
	// This is only called for SoF clients, as regular /who is now handled server-side for that client.
	uint32 PacketLength = 0;
	uint32 Entries = 0;
	uint8 WhomLength = strlen(pWho->whom);

	std::list<Client* > client_sub_list;
	auto it = mClients.begin();
	while (it != mClients.end()) {
		Client* ClientEntry = it->second;
		++it;

		if (ClientEntry) {
			if (ClientEntry->GMHideMe(pClient))
				continue;
			if ((pWho->wrace != 0xFFFFFFFF) && (ClientEntry->GetRace() != pWho->wrace))
				continue;
			if ((pWho->wclass != 0xFFFFFFFF) && (ClientEntry->GetClass() != pWho->wclass))
				continue;
			if ((pWho->lvllow != 0xFFFFFFFF) && (ClientEntry->GetLevel() < pWho->lvllow))
				continue;
			if ((pWho->lvlhigh != 0xFFFFFFFF) && (ClientEntry->GetLevel() > pWho->lvlhigh))
				continue;
			if (pWho->guildid != 0xFFFFFFFF) {
				if ((pWho->guildid == 0xFFFFFFFC) && !ClientEntry->IsTrader())
					continue;
				if ((pWho->guildid == 0xFFFFFFFB) && !ClientEntry->IsBuyer())
					continue;
				if (pWho->guildid != ClientEntry->GuildID())
					continue;
			}
			if (WhomLength && strncasecmp(pWho->whom, ClientEntry->getName(), WhomLength) &&
				strncasecmp(guild_mgr.getGuildName(ClientEntry->GuildID()), pWho->whom, WhomLength))
				continue;

			Entries++;
			client_sub_list.push_back(ClientEntry);

			PacketLength = PacketLength + strlen(ClientEntry->getName());

			if (strlen(guild_mgr.getGuildName(ClientEntry->GuildID())) > 0)
				PacketLength = PacketLength + strlen(guild_mgr.getGuildName(ClientEntry->GuildID())) + 2;
		}
	}

	PacketLength = PacketLength + sizeof(WhoAllReturnStruct)+(47 * Entries);
	EQApplicationPacket *outapp = new EQApplicationPacket(OP_WhoAllResponse, PacketLength);
	char *Buffer = (char *)outapp->pBuffer;
	WhoAllReturnStruct *WARS = (WhoAllReturnStruct *)Buffer;
	WARS->id = 0;
	WARS->playerineqstring = 5001;
	strncpy(WARS->line, "---------------------------", sizeof(WARS->line));
	WARS->unknown35 = 0x0a;
	WARS->unknown36 = 0;

	switch (Entries) {
	case 0:
		WARS->playersinzonestring = 5029;
		break;
	case 1:
		WARS->playersinzonestring = 5028; // 5028 There is %1 player in EverQuest.
		break;
	default:
		WARS->playersinzonestring = 5036; // 5036 There are %1 players in EverQuest.
	}

	WARS->unknown44[0] = 0;
	WARS->unknown44[1] = 0;
	WARS->unknown52 = Entries;
	WARS->unknown56 = Entries;
	WARS->playercount = Entries;
	Buffer += sizeof(WhoAllReturnStruct);

	auto sit = client_sub_list.begin();
	while (sit != client_sub_list.end()) {
		Client* ClientEntry = *sit;
		++sit;

		if (ClientEntry) {
			if (ClientEntry->GMHideMe(pClient))
				continue;
			if ((pWho->wrace != 0xFFFFFFFF) && (ClientEntry->GetRace() != pWho->wrace))
				continue;
			if ((pWho->wclass != 0xFFFFFFFF) && (ClientEntry->GetClass() != pWho->wclass))
				continue;
			if ((pWho->lvllow != 0xFFFFFFFF) && (ClientEntry->GetLevel() < pWho->lvllow))
				continue;
			if ((pWho->lvlhigh != 0xFFFFFFFF) && (ClientEntry->GetLevel() > pWho->lvlhigh))
				continue;
			if (pWho->guildid != 0xFFFFFFFF) {
				if ((pWho->guildid == 0xFFFFFFFC) && !ClientEntry->IsTrader())
					continue;
				if ((pWho->guildid == 0xFFFFFFFB) && !ClientEntry->IsBuyer())
					continue;
				if (pWho->guildid != ClientEntry->GuildID())
					continue;
			}
			if (WhomLength && strncasecmp(pWho->whom, ClientEntry->getName(), WhomLength) &&
				strncasecmp(guild_mgr.getGuildName(ClientEntry->GuildID()), pWho->whom, WhomLength))
				continue;
			std::string GuildName;
			if ((ClientEntry->GuildID() != GUILD_NONE) && (ClientEntry->GuildID() > 0)) {
				GuildName = "<";
				GuildName += guild_mgr.getGuildName(ClientEntry->GuildID());
				GuildName += ">";
			}
			uint32 FormatMSGID = 5025; // 5025 %T1[%2 %3] %4 (%5) %6 %7 %8 %9
			if (ClientEntry->GetAnon() == 1)
				FormatMSGID = 5024; // 5024 %T1[ANONYMOUS] %2 %3
			else if (ClientEntry->GetAnon() == 2)
				FormatMSGID = 5023; // 5023 %T1[ANONYMOUS] %2 %3 %4
			uint32 PlayerClass = 0;
			uint32 PlayerLevel = 0;
			uint32 PlayerRace = 0;
			uint32 ZoneMSGID = 0xFFFFFFFF;

			if (ClientEntry->GetAnon() == 0) {
				PlayerClass = ClientEntry->GetClass();
				PlayerLevel = ClientEntry->GetLevel();
				PlayerRace = ClientEntry->GetRace();
			}

			WhoAllPlayerPart1* WAPP1 = (WhoAllPlayerPart1*)Buffer;
			WAPP1->FormatMSGID = FormatMSGID;
			WAPP1->PIDMSGID = 0xFFFFFFFF;
			strcpy(WAPP1->Name, ClientEntry->getName());
			Buffer += sizeof(WhoAllPlayerPart1)+strlen(WAPP1->Name);
			WhoAllPlayerPart2* WAPP2 = (WhoAllPlayerPart2*)Buffer;

			if (ClientEntry->IsTrader())
				WAPP2->RankMSGID = 12315;
			else if (ClientEntry->IsBuyer())
				WAPP2->RankMSGID = 6056;
			else if (ClientEntry->Admin() >= 10)
				WAPP2->RankMSGID = 12312;
			else
				WAPP2->RankMSGID = 0xFFFFFFFF;

			strcpy(WAPP2->Guild, GuildName.c_str());
			Buffer += sizeof(WhoAllPlayerPart2)+strlen(WAPP2->Guild);
			WhoAllPlayerPart3* WAPP3 = (WhoAllPlayerPart3*)Buffer;
			WAPP3->Unknown80[0] = 0xFFFFFFFF;

			if (ClientEntry->IsLD())
				WAPP3->Unknown80[1] = 12313; // LinkDead
			else
				WAPP3->Unknown80[1] = 0xFFFFFFFF;

			WAPP3->ZoneMSGID = ZoneMSGID;
			WAPP3->Zone = 0;
			WAPP3->Class_ = PlayerClass;
			WAPP3->Level = PlayerLevel;
			WAPP3->Race = PlayerRace;
			WAPP3->Account[0] = 0;
			Buffer += sizeof(WhoAllPlayerPart3);
			WhoAllPlayerPart4* WAPP4 = (WhoAllPlayerPart4*)Buffer;
			WAPP4->Unknown100 = 0;
			Buffer += sizeof(WhoAllPlayerPart4);
		}

	}

	pClient->QueuePacket(outapp);

	safe_delete(outapp);
}

void EntityList::unMarkNPC(uint16 pID) {
	// Designed to be called from the Mob destructor, this method calls Group::UnMarkNPC for
	// each group to remove the dead mobs entity ID from the groups list of NPCs marked via the
	// Group Leadership AA Mark NPC ability.
	//
	for (auto& i : mClients) {
		Group* group = i.second->GetGroup();
		if (group) 
			group->unMarkNPC(pID);
	}
}

void EntityList::gateAllClients() {
	for (auto& i : mClients)
		i.second->GoToBind();
}

void EntityList::signalAllClients(uint32 pData) {
	for (auto& i : mClients)
		i.second->Signal(pData);
}

void EntityList::updateQGlobal(uint32 pID, QGlobal pNewGlobal) {
	for (auto& i : mMOBs) {
		Mob* mob = i.second;

		if (mob->isClient()) {
			QGlobalCache *qgc = mob->castToClient()->GetQGlobals();
			if (qgc) {
				uint32 char_id = mob->castToClient()->CharacterID();
				if (pNewGlobal.char_id == char_id && pNewGlobal.npc_id == 0)
					qgc->AddGlobal(pID, pNewGlobal);
			}
		}
		else if (mob->isNPC()) {
			QGlobalCache *qgc = mob->castToNPC()->GetQGlobals();
			if (qgc) {
				uint32 npc_id = mob->GetNPCTypeID();
				if (pNewGlobal.npc_id == npc_id)
					qgc->AddGlobal(pID, pNewGlobal);
			}
		}
	}
}

void EntityList::deleteQGlobal(std::string pName, uint32 pNPCID, uint32 pCharacterID, uint32 pZoneID) {
	for (auto& i : mMOBs) {
		Mob* mob = i.second;

		if (mob->isClient()) {
			QGlobalCache *qgc = mob->castToClient()->GetQGlobals();
			if (qgc)
				qgc->RemoveGlobal(pName, pNPCID, pCharacterID, pZoneID);
		}
		else if (mob->isNPC()) {
			QGlobalCache *qgc = mob->castToNPC()->GetQGlobals();
			if (qgc)
				qgc->RemoveGlobal(pName, pNPCID, pCharacterID, pZoneID);
		}
	}
}

void EntityList::sendFindableNPCList(Client* pClient) {
	if (!pClient) return;

	EQApplicationPacket *outapp = new EQApplicationPacket(OP_SendFindableNPCs, sizeof(FindableNPC_Struct));

	FindableNPC_Struct *fnpcs = (FindableNPC_Struct *)outapp->pBuffer;

	fnpcs->Unknown109 = 0x16;
	fnpcs->Unknown110 = 0x06;
	fnpcs->Unknown111 = 0x24;

	fnpcs->Action = 0;

	for (auto& i : mNPCs) {
		NPC* npc = i.second;
		if (npc->IsFindable()) {
			fnpcs->EntityID = npc->getID();
			strn0cpy(fnpcs->Name, npc->GetCleanName(), sizeof(fnpcs->Name));
			strn0cpy(fnpcs->LastName, npc->GetLastName(), sizeof(fnpcs->LastName));
			fnpcs->Race = npc->GetRace();
			fnpcs->Class = npc->GetClass();

			pClient->QueuePacket(outapp);
		}
	}
	safe_delete(outapp);
}

void EntityList::updateFindableNPCState(NPC *pNPC, bool pRemove) {
	if (!pNPC || !pNPC->IsFindable()) return;

	EQApplicationPacket *outapp = new EQApplicationPacket(OP_SendFindableNPCs, sizeof(FindableNPC_Struct));

	FindableNPC_Struct *fnpcs = (FindableNPC_Struct *)outapp->pBuffer;

	fnpcs->Unknown109 = 0x16;
	fnpcs->Unknown110 = 0x06;
	fnpcs->Unknown111 = 0x24;

	fnpcs->Action = pRemove ? 1 : 0;
	fnpcs->EntityID = pNPC->getID();
	strn0cpy(fnpcs->Name, pNPC->GetCleanName(), sizeof(fnpcs->Name));
	strn0cpy(fnpcs->LastName, pNPC->GetLastName(), sizeof(fnpcs->LastName));
	fnpcs->Race = pNPC->GetRace();
	fnpcs->Class = pNPC->GetClass();

	for (auto& i : mClients) {
		Client* client = i.second;
		if (client && (client->GetClientVersion() >= EQClientSoD))
			client->QueuePacket(outapp);
	}

	safe_delete(outapp);
}

void EntityList::hideCorpses(Client* pClient, uint8 pCurrentMode, uint8 pNewMode) {
	if (!pClient) return;

	if (pNewMode == HideCorpseNone) {
		sendZoneCorpses(pClient);
		return;
	}

	Group* group = nullptr;

	if (pNewMode == HideCorpseAllButGroup) {
		group = pClient->GetGroup();

		if (!group)
			pNewMode = HideCorpseAll;
	}

	for (auto& i : mCorpses) {
		Corpse* corpse = i.second;
		if (corpse && (corpse->getCharacterID() != pClient->CharacterID())) {
			if ((pNewMode == HideCorpseAll) || ((pNewMode == HideCorpseNPC) && (corpse->isNPCCorpse()))) {
				EQApplicationPacket outapp;
				corpse->CreateDespawnPacket(&outapp, false);
				pClient->QueuePacket(&outapp);
			}
			else if (pNewMode == HideCorpseAllButGroup) {
				if (!group->isGroupMember(corpse->GetOwnerName())) {
					EQApplicationPacket outapp;
					corpse->CreateDespawnPacket(&outapp, false);
					pClient->QueuePacket(&outapp);
				}
				else if ((pCurrentMode == HideCorpseAll)) {
					EQApplicationPacket outapp;
					corpse->CreateSpawnPacket(&outapp);
					pClient->QueuePacket(&outapp);
				}
			}
		}
	}
}

// TODO: Fix this amazing code.
void EntityList::addLootToNPCs(uint32 pItemID, uint32 pCount) {
	if (pCount == 0)
		return;

	int npc_count = 0;
	for (auto i = mNPCs.begin(); i != mNPCs.end(); i++) {
		if (!i->second->IsPet()
			&& i->second->GetClass() != LDON_TREASURE
			&& i->second->GetBodyType() != BT_NoTarget
			&& i->second->GetBodyType() != BT_NoTarget2
			&& i->second->GetBodyType() != BT_Special)
			npc_count++;
	}

	if (npc_count == 0)
		return;

	NPC **npcs = new NPC*[npc_count];
	int *counts = new int[npc_count];
	bool *marked = new bool[npc_count];
	memset(counts, 0, sizeof(int)* npc_count);
	memset(marked, 0, sizeof(bool)* npc_count);

	int npcIndex = 0;
	for (auto i = mNPCs.begin(); i != mNPCs.end(); i++) {
		if (!i->second->IsPet()
			&& i->second->GetClass() != LDON_TREASURE
			&& i->second->GetBodyType() != BT_NoTarget
			&& i->second->GetBodyType() != BT_NoTarget2
			&& i->second->GetBodyType() != BT_Special)
			npcs[npcIndex++] = i->second;
	}

	while (pCount > 0) {
		std::vector<int> selection;
		selection.reserve(npc_count);
		for (int j = 0; j < npc_count; ++j)
			selection.push_back(j);

		while (selection.size() > 0 && pCount > 0) {
			int k = MakeRandomInt(0, selection.size() - 1);
			counts[selection[k]]++;
			pCount--;
			selection.erase(selection.begin() + k);
		}
	}

	for (int j = 0; j < npc_count; ++j)
	if (counts[j] > 0)
	for (int k = 0; k < counts[j]; ++k)
		npcs[j]->AddItem(pItemID, 1);

	safe_delete_array(npcs);
	safe_delete_array(counts);
	safe_delete_array(marked);
}

void EntityList::cameraEffect(uint32 pDuration, uint32 pIntensity)
{
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_CameraEffect, sizeof(Camera_Struct));
	memset(outapp->pBuffer, 0, sizeof(outapp->pBuffer));
	Camera_Struct* cs = (Camera_Struct*)outapp->pBuffer;
	cs->duration = pDuration;	// Duration in milliseconds
	cs->intensity = ((pIntensity * 6710886) + 1023410176);	// Intensity ranges from 1023410176 to 1090519040, so simplify it from 0 to 10.
	entity_list.queueClients(0, outapp);
	safe_delete(outapp);
}


NPC* EntityList::getClosestBanker(Mob* pSender, uint32& pDistance) {
	if (!pSender) return nullptr;

	pDistance = 4294967295u;
	NPC* npc = nullptr;

	for (auto& i : mNPCs) {
		if (i.second->GetClass() == BANKER) {
			uint32 nd = ((i.second->GetY() - pSender->GetY()) * (i.second->GetY() - pSender->GetY())) +
				((i.second->GetX() - pSender->GetX()) * (i.second->GetX() - pSender->GetX()));
			if (nd < pDistance) {
				pDistance = nd;
				npc = i.second;
			}
		}
	}
	return npc;
}

void EntityList::expeditionWarning(uint32 pMinutesRemaining) {
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_DzExpeditionEndsWarning, sizeof(ExpeditionExpireWarning));
	ExpeditionExpireWarning *ew = (ExpeditionExpireWarning*)outapp->pBuffer;
	ew->minutes_remaining = pMinutesRemaining;

	auto it = mClients.begin();
	while (it != mClients.end()) {
		it->second->Message_StringID(15, EXPEDITION_MIN_REMAIN, itoa((int)pMinutesRemaining));
		it->second->QueuePacket(outapp);
		++it;
	}
}

Mob* EntityList::getClosestMobByBodyType(Mob* pSender, BodyType pBodyType) {
	if (!pSender) return nullptr;

	uint32 closestDistance = 4294967295u;
	Mob* closestMob = nullptr;

	for (auto& i : mMOBs) {
		Mob* currentMOB = i.second;
		if (currentMOB->GetBodyType() != pBodyType)
			continue;

		uint32 currentDistance = ((currentMOB->GetY() - pSender->GetY()) * (currentMOB->GetY() - pSender->GetY())) +
			((currentMOB->GetX() - pSender->GetX()) * (currentMOB->GetX() - pSender->GetX()));

		if (currentDistance < closestDistance) {
			closestDistance = currentDistance;
			closestMob = currentMOB;
		}
	}
	return closestMob;
}

void EntityList::getTargetsForConeArea(Mob *pStart, uint32 pRadius, uint32 pHeight, std::list<Mob*>& pMOBs)
{
	for (auto& i : mMOBs) {
		if (i.second == pStart) continue;

		Mob* mob = i.second;
		int32 x_diff = mob->GetX() - pStart->GetX();
		int32 y_diff = mob->GetY() - pStart->GetY();
		int32 z_diff = mob->GetZ() - pStart->GetZ();

		x_diff *= x_diff;
		y_diff *= y_diff;
		z_diff *= z_diff;

		if ((x_diff + y_diff) <= (pRadius * pRadius) && z_diff <= (pHeight * pHeight))
			pMOBs.push_back(mob);
	}
}

Client* EntityList::findCorpseDragger(uint16 pCorpseID) {
	for (auto i = mClients.begin(); i != mClients.end(); i++) {
		if (i->second->IsDraggingCorpse(pCorpseID))
			return i->second;
	}
	return nullptr;
}

Mob *EntityList::getTargetForVirus(Mob* pSpreader) {
	int max_spread_range = RuleI(Spells, VirusSpreadDistance);

	std::vector<Mob *> TargetsInRange;

	auto it = mMOBs.begin();
	while (it != mMOBs.end()) {
		// Make sure the target is in range, has los and is not the mob doing the spreading
		if ((it->second->getID() != pSpreader->getID()) &&
			(it->second->CalculateDistance(pSpreader->GetX(), pSpreader->GetY(),
			pSpreader->GetZ()) <= max_spread_range) &&
			(pSpreader->CheckLosFN(it->second))) {
			// If the spreader is an npc it can only spread to other npc controlled mobs
			if (pSpreader->isNPC() && !pSpreader->IsPet() && it->second->isNPC()) {
				TargetsInRange.push_back(it->second);
			}
			// If the spreader is an npc controlled pet it can spread to any other npc or an npc controlled pet
			else if (pSpreader->isNPC() && pSpreader->IsPet() && pSpreader->GetOwner()->isNPC()) {
				if (it->second->isNPC() && !it->second->IsPet()) {
					TargetsInRange.push_back(it->second);
				}
				else if (it->second->isNPC() && it->second->IsPet() && it->second->GetOwner()->isNPC()) {
					TargetsInRange.push_back(it->second);
				}
			}
			// if the spreader is anything else(bot, pet, etc) then it should spread to everything but non client controlled npcs
			else if (!pSpreader->isNPC() && !it->second->isNPC()) {
				TargetsInRange.push_back(it->second);
			}
			// if its a pet we need to determine appropriate targets(pet to client, pet to pet, pet to bot, etc)
			else if (pSpreader->isNPC() && pSpreader->IsPet() && !pSpreader->GetOwner()->isNPC()) {
				if (!it->second->isNPC()) {
					TargetsInRange.push_back(it->second);
				}
				else if (it->second->isNPC() && it->second->IsPet() && !it->second->GetOwner()->isNPC()) {
					TargetsInRange.push_back(it->second);
				}
			}
		}
		++it;
	}

	if (TargetsInRange.size() == 0)
		return nullptr;

	return TargetsInRange[MakeRandomInt(0, TargetsInRange.size() - 1)];
}

