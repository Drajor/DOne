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
#ifndef ENTITY_H
#define ENTITY_H
#include <unordered_map>
#include <queue>

#include "../common/types.h"
#include "../common/linked_list.h"
#include "../common/servertalk.h"
#include "../common/bodytypes.h"
#include "../common/eq_constants.h"

#include "zonedb.h"
#include "zonedump.h"
#include "zonedbasync.h"
#include "QGlobals.h"

class EQApplicationPacket;

class Client;
class Mob;
class NPC;
class Merc;
class Corpse;
class Beacon;
class Petition;
class Object;
class Group;
class Raid;
class Doors;
class Trap;
class Entity;
class EntityList;

extern EntityList entity_list;

class Entity
{
public:
	Entity();
	virtual ~Entity();

	virtual bool isClient() const { return false; }
	virtual bool isNPC() const { return false; }
	virtual bool isMOB() const { return false; }
	virtual bool isMerc() const { return false; }
	virtual bool isCorpse() const { return false; }
	virtual bool isPlayerCorpse() const { return false; }
	virtual bool isNPCCorpse() const { return false; }
	virtual bool isObject() const { return false; }
	virtual bool isDoor() const { return false; }
	virtual bool isTrap() const { return false; }
	virtual bool isBeacon() const { return false; }

	virtual bool process() { return false; }
	virtual bool save() { return true; }
	virtual void depop(bool StartSpawnTimer = false) {}

	Client* castToClient();
	const Client* castToClient() const;
	NPC* castToNPC();
	const NPC* castToNPC() const;
	Mob* castToMOB();
	const Mob* castToMOB() const;
	Merc* castToMerc();
	const Merc* castToMerc() const;
	Corpse* castToCorpse();
	const Corpse* castToCorpse() const;
	Object* castToObject();
	const Object* castToObject() const;
	Doors* castToDoors();
	const Doors* castToDoors() const;
	Trap* castToTrap();
	const Trap* castToTrap() const;
	Beacon* castToBeacon();
	const Beacon* castToBeacon() const;

	inline const uint16& getID() const { return mID; }

	virtual const char* getName() { return ""; }
	// TODO: Look more closely at DBAWComplete and checkCoordLosNoZLeaps before modifying.
	virtual void DBAWComplete(uint8 workpt_b1, DBAsyncWork* dbaw) { mDBAsyncWorkID = 0; }
	bool checkCoordLosNoZLeaps(float cur_x, float cur_y, float cur_z, float trg_x, float trg_y, float trg_z, float perwalk=1);

protected:
	friend class EntityList;
	inline virtual void setID(uint16 pID) { mID = pID; }
	uint32 mDBAsyncWorkID;
private:
	uint16 mID;
};

class EntityList
{
public:
	struct Area {
		Area() : mID(0), mType(0), mMinX(0.0f), mMaxX(0.0f), mMinY(0.0f), mMaxY(0.0f), mMinZ(0.0f), mMaxZ(0.0f) {};
		int mID;
		int mType;
		float mMinX;
		float mMaxX;
		float mMinY;
		float mMaxY;
		float mMinZ;
		float mMaxZ;
	};

	EntityList();
	~EntityList();

	Entity* getID(uint16 pID);
	Mob* getMOB(uint16 pID);
	
	bool isMOBSpawnedByNpcTypeID(uint32 pID);

	// Search for MOB 
	Mob* getMOB(const char* pName);
	Mob* getMOBByNpcTypeID(uint32 pID);
	Mob* getTargetForVirus(Mob* pSpreader);
	inline Mob* getMOBByID(uint16 pID) { return getMOB(pID); }

	NPC* getNPCByNPCTypeID(uint32 pNPCTypeID);
	inline NPC* getNPCByID(uint16 pID) { return mNPCs.count(pID) ? mNPCs.at(pID) : nullptr; }
	inline Merc* getMercByID(uint16 pID) { return mMercs.count(pID) ? mMercs.at(pID) : nullptr; }

	// Search for Client
	Client* getClientByName(const char* pName);
	Client* getClientByAccountID(uint32 pAccountID);
	Client* getClientByCharacterID(uint32 pCharacterID);
	Client* getClientByWID(uint32 pWID);
	Client* getRandomClient(float pX, float pY, float pZ, float pDistance, Client* pExcludeClient = nullptr);
	inline Client* getClientByID(uint16 pID) { return mClients.count(pID) ? mClients.at(pID) : nullptr; }

	Group* getGroupByMOB(Mob* pMOB);
	Group* getGroupByClient(Client* pClient);
	// Returns the Group which has ID pGroupID, or null where it does not exist.
	Group* getGroupByID(uint32 pGroupID);
	// Returns the Group which has leader named pLeaderName, or null where it does not exist.
	Group* getGroupByLeaderName(const char* pLeaderName);

	Raid* getRaidByClient(Client* pClient);
	Raid* getRaidByID(uint32 pRaidID);

	Corpse* getCorpseByOwner(Client* pClient);
	Corpse* getCorpseByOwnerWithinRange(Client* pClient, Mob* pCenter, int pRange);
	Corpse* getCorpseByDatabaseID(uint32 pDatabaseID);
	Corpse* getCorpseByName(const char* pName);
	inline Corpse* getCorpseByID(uint16 pID) { return mCorpses.count(pID) ? mCorpses.at(pID) : nullptr; }

	Spawn2* getSpawnByID(uint32 pID);

	Client* findCorpseDragger(uint16 pCorpseID);

	inline Object* getObjectByID(uint16 pID) { return mObjects.count(pID) ? mObjects.at(pID) : nullptr; }
	Object* getObjectByDatabaseID(uint32 pID);
	inline Doors* getDoorsByID(uint16 pID) { return mDoors.count(pID) ? mDoors.at(pID) : nullptr; }
	Doors* getDoorsByDoorID(uint32 pID);
	Doors* getDoorsByDatabaseID(uint32 pID);
	void removeAllCorpsesByCharacterID(uint32 pCharacterID);
	void removeCorpseByDatabaseID(uint32 pDatabaseID);
	int resurrectAllCorpsesByCharacterID(uint32 pCharacterID);
	bool isMOBInZone(Mob* pMOB);
	void clearClientPetitionQueue();
	bool canAddHateForMOB(Mob* pMOB); // TODO: What is this?
	void sendGuildMOTD(uint32 pGuildID);
	void sendGuildSpawnAppearance(uint32 pGuildID);
	void sendGuildMembers(uint32 pGuildID);
	void refreshAllGuildInfo(uint32 pGuildID);
	void sendGuildList();
	void checkGroupList (const char* pFileName, const int pFileLine);
	void groupProcess();
	void raidProcess();
	void doorProcess();
	void objectProcess();
	void corpseProcess();
	void MOBProcess();
	void trapProcess();
	void beaconProcess();
	void processMove(Client* pClient, float pX, float pY, float pZ);
	void processMove(NPC* pNPC, float pX, float pY, float pZ);
	void addArea(int pID, int pType, float pMinX, float pMaxX, float pMinY, float pMaxY, float pMinZ, float pMaxZ);
	void removeArea(int pID);
	void clearAreas();
	void processProximitySay(const char* pMessage, Client* pClient, uint8 pLanguage = 0);
	Doors* findDoor(uint8 pID);
	Object* findObject(uint32 pID);
	Object*	findNearbyObject(float pX, float pY, float pZ, float pRadius);
	bool makeDoorSpawnPacket(EQApplicationPacket* pApp, Client* pClient);
	bool makeTrackPacket(Client* pClient);
	void sendTraders(Client* pClient);
	void addClient(Client* pClient);
	void addNPC(NPC* pNPC, bool pSendSpawnPacket = true, bool pDontQueue = false);
	void addMerc(Merc* pMerc, bool pSendSpawnPacket = true, bool pDontQueue = false);
	void addCorpse(Corpse* pCorpse, uint32 pID = 0xFFFFFFFF);
	void addObject(Object* pObject, bool pSendSpawnPacket = true);
	void addGroup(Group* pGroup);
	void addGroup(Group* pGroup, uint32 pID);
	void addRaid(Raid* pRaid);
	void addRaid(Raid* pRaid, uint32 pID);
	void addDoor(Doors* pDoor);
	void addTrap(Trap* pTrap);
	void addBeacon(Beacon* pBeacon);
	void addProximity(NPC* pProximityFor);
	void clear();
	bool removeMOB(uint16 pID);
	bool removeMOB(Mob* pMOB);
	bool removeClient(uint16 pID);
	bool removeClient(Client* pClient);
	bool removeNPC(uint16 pID);
	bool removeMerc(uint16 pID);
	bool removeGroup(uint32 pID);
	bool removeRaid(uint32 pID);
	bool removeCorpse(uint16 pID);
	bool removeDoor(uint16 pID);
	bool removeTrap(uint16 pID);
	bool removeObject(uint16 pID);
	bool removeProximity(uint16 pID);
	void removeAllMobs();
	void removeAllClients();
	void removeAllNPCs();
	void removeAllMercs();
	void removeAllGroups();
	void removeAllCorpses();
	void removeAllDoors();
	void despawnAllDoors();
	void respawnAllDoors();
	void removeAllTraps();
	void removeAllObjects();
	void removeAllLocalities();
	void removeAllRaids();
	void destroyTempPets(Mob* pOwner);
	Entity* getEntityMOB(uint16 pID);
	Entity* getEntityMerc(uint16 pID);
	Entity* getEntityDoor(uint16 pID);
	Entity* getEntityObject(uint16 pID);
	Entity* getEntityCorpse(uint16 pID);
	Entity* getEntityTrap(uint16 pID);
	Entity* getEntityBeacon(uint16 pID);
	Entity* getEntityMOB(const char* pName);
	Entity* getEntityCorpse(const char* pName);

	void describeAggro(Client* pClient, NPC* pNPC, float pDistance, bool pVerbose);

	void message(uint32 pGuildID, uint32 pType, const char* pMessage, ...);
	void messageStatus(uint32 pGuildID, int pMinStatus, uint32 pType, const char* pMessage, ...);
	void messageClose(Mob* pSender, bool pSkipSender, float pDistance, uint32 pType, const char* pMessage, ...);
	void messageStringID(Mob* pSender, bool pSkipSender, uint32 pType, uint32 pStringID, const char* pMessage1=0, const char* pMmessage2=0,const char* pMessage3=0, const char* pMessage4=0, const char* pMessage5=0, const char* pMessage6=0, const char* pMessage7=0, const char* pMessage8=0, const char* pMessage9=0);
	void filteredMessageStringID(Mob* pSender, bool pSkipSender, uint32 pType, eqFilterType pFilter, uint32 StringID, const char* pMessage1 = 0, const char* pMmessage2 = 0, const char* pMessage3 = 0, const char* pMessage4 = 0, const char* pMessage5 = 0, const char* pMessage6 = 0, const char* pMessage7 = 0, const char* pMessage8 = 0, const char* pMessage9 = 0);
	void messageCloseStringID(Mob* pSender, bool pSkipSender, float pDistance, uint32 pType, uint32 StringID, const char* pMessage1 = 0, const char* pMmessage2 = 0, const char* pMessage3 = 0, const char* pMessage4 = 0, const char* pMessage5 = 0, const char* pMessage6 = 0, const char* pMessage7 = 0, const char* pMessage8 = 0, const char* pMessage9 = 0);
	void filteredMessageCloseStringID(Mob* pSender, bool pSkipSender, float pDistance, uint32 pType, eqFilterType pFilter, uint32 pStringID, const char* pMessage1 = 0, const char* pMmessage2 = 0, const char* pMessage3 = 0, const char* pMessage4 = 0, const char* pMessage5 = 0, const char* pMessage6 = 0, const char* pMessage7 = 0, const char* pMessage8 = 0, const char* pMessage9 = 0);
	
	void channelMessageFromWorld(const char* pFrom, const char* pTo, uint8 pChannelNumber, uint32 pGuildDatabaseID, uint8 pLanguage, const char* pMessage);
	void channelMessage(Mob* pFrom, uint8 pChannelNumber, uint8 pLanguage, const char* pMessage, ...);
	void channelMessage(Mob* pFrom, uint8 pChannelNumber, uint8 pLanguage, uint8 pLanguageSkill, const char* pMessage, ...);
	void channelMessageSend(Mob* pTo, uint8 pChannelNumber, uint8 pLanguage, const char* pMessage, ...);

	void sendZoneSpawns(Client* pClient);
	void sendZonePVPUpdates(Client* pClient);
	void sendZoneSpawnsBulk(Client* pClient);
	void save();
	void sendZoneCorpses(Client* pClient);
	void sendZoneCorpsesBulk(Client* pClient);
	void sendZoneObjects(Client* pClient);
	void sendZoneAppearance(Client* pClient);
	void sendNimbusEffects(Client* pClient);
	void sendUntargetable(Client* pClient);

	void duelMessage(Mob* pWinner, Mob* pLoser, bool pFlee);
	void questJournalledSayClose(Mob *pSender, Client* pQuestIntiator, float pDistance, const char* pMobName, const char* pMessage);
	void groupMessage(uint32 pGroupID, const char* pFrom, const char* pMessage);
	void expeditionWarning(uint32 pMinutesRemaining);

	void removeFromTargets(Mob* pMOB, bool pRemoveFromXTargets = false);
	void removeFromXTargets(Mob* pMOB);
	void removeFromAutoXTargets(Mob* pMOB);
	void replaceWithTarget(Mob* pOldMOB, Mob* pNewTarget);
	void queueCloseClients(Mob* pSender, const EQApplicationPacket* pApp, bool pIgnoreSender = false, float pDistance = 200, Mob* pSkipThisMob = 0, bool pAckReq = true, eqFilterType pFilter = FilterNone);
	void queueClients(Mob* pSender, const EQApplicationPacket* pApp, bool pIgnoreSender = false, bool pAckReq = true);
	void queueClientsStatus(Mob* pSender, const EQApplicationPacket* pApp, bool pIgnoreSender = false, uint8 pMinStatus = 0, uint8 pMaxStatus = 0);
	void queueClientsGuild(Mob* pSender, const EQApplicationPacket* pApp, bool pIgnoreSender = false, uint32 pGuildID = 0);
	void queueClientsGuildBankItemUpdate(const GuildBankItemUpdate_Struct* pUpdate, uint32 pGuildID);
	void queueClientsByTarget(Mob* pSender, const EQApplicationPacket* pApp, bool pSendToSender = true, Mob* pSkipThisMOB = 0, bool pAckReq = true, bool pHoTT = true, uint32 pClientVersionBits = 0xFFFFFFFF);

	void queueClientsByXTarget(Mob* pSender, const EQApplicationPacket* pApp, bool pSendToSender = true);
	void queueToGroupsForNPCHealthAA(Mob* pSender, const EQApplicationPacket* pApp);
	void queueManaged(Mob* pSender, const EQApplicationPacket* pApp, bool pIgnoreSender = false, bool pAckReq = true);

	void AEAttack(Mob* pAttacker, float pDistance, int pHand = 13, int pCount = 0, bool pIsFromSpell = false);
	void AETaunt(Client* pCaster, float pRange = 100);
	void AESpell(Mob* pCaster, Mob* pCenter, uint16 pSpellID, bool pAffectCaster = true, int16 pResistAdjust = 0);
	void massGroupBuff(Mob* pCaster, Mob* pCenter, uint16 pSpellID, bool pAffectCaster = true);
	void AEBardPulse(Mob* pCaster, Mob* pCenter, uint16 pSpellID, bool pAffectCaster = true);

	void radialSetLogging(Mob* pAround, bool pEnabled, bool pClients, bool pNonClients, float pRange = 0);

	//trap stuff
	Mob* getTrapTrigger(Trap* pTrap);
	void sendAlarm(Trap* pTrap, Mob* pCurrentTarget, uint8 pKOS);
	Trap* findNearbyTrap(Mob* pSearcher, float pMaxDistance);

	void addHealAggro(Mob* pTarget, Mob* pCaster, uint16 pDamage);
	void openDoorsNear(NPC* pOpener);
	void updateWho(bool pSendFullUpdate = false);
	void sendPositionUpdates(Client* pClient, uint32 pLastUpdate = 0, float pRange = 0, Entity* pAlwaysSend = 0, bool pSendEvenIfNotChanged = false);
	char* MakeNameUnique(char* pName);
	static char* removeNumbers(char* pName);
	void signalMOBsByNPCID(uint32 pNPCTypeID, int pSignalID);
	void removeEntity(uint16 pID);
	void sendPetitionToAdmins(Petition* pPetition);
	void sendPetitionToAdmins();
	void addLootToNPCs(uint32 pItemID, uint32 pCount);

	void listNPCCorpses(Client* pClient);
	void listPlayerCorpses(Client* pClient);
	int32 deleteNPCCorpses();
	int32 deletePlayerCorpses();
	void updateHoTT(Mob* pTarget);

	void process();
	void clearAggro(Mob* pTarget);
	void clearFeignAggro(Mob* pTarget);
	void clearZoneFeignAggro(Client* pTarget);
	void aggroZone(Mob* pTarget, int pHate = 0);

	bool isFighting(Mob* pTarget);
	void removeFromHateLists(Mob* pMOB, bool pSetToOne = false); // TODO: Remove pSetToOne - this is a different function entirely.
	void removeDebuffs(Mob* pCaster);


	void messageGroup(Mob* pSender, bool pSkipClose, uint32 pType, const char* pMessage, ...);

	void limitAddNPC(NPC* pNPC);
	void limitRemoveNPC(NPC *pNPC);
	bool limitCheckType(uint32 pNPCTypeID, int pCount);
	bool limitCheckGroup(uint32 pSpawnGroupID, int pCount);
	bool limitCheckName(const char* pName);

	void checkClientAggro(Client* pClient);
	Mob* AICheckCloseAggro(Mob* pSender);
	int getHatedCount(Mob* pAttacker, Mob* pExclude);
	void AIYellForHelp(Mob* pSender, Mob* pAttacker);
	bool AICheckCloseBeneficialSpells(NPC* pCaster, uint8 pChance, float pRange, uint16 pSpellTypes);
	bool Merc_AICheckCloseBeneficialSpells(Merc* pCaster, uint8 pChance, float pRange, uint32 pSpellTypes);
	Mob* getTargetForMez(Mob* pCaster);
	uint32 checkNPCsClose(Mob *pCenter);

	Corpse* getClosestCorpse(Mob* pSender, const char* pName);
	NPC* getClosestBanker(Mob* pSender, uint32& pDistance);
	void cameraEffect(uint32 pDuration, uint32 pIntensity);
	Mob* getClosestMobByBodyType(Mob* pSender, BodyType pBodyType);
	void forceGroupUpdate(uint32 pGroupID);
	void sendGroupLeave(uint32 pGroupID, const char* pName);
	void sendGroupJoin(uint32 pGroupID, const char* pName);

	void saveAllClientsTaskState();
	void reloadAllClientsTaskState(int pTaskID = 0);

	uint16 createGroundObject(uint32 pItemID, float pX, float pY, float pZ, float pHeading, uint32 pDecayTime = 300000);
	uint16 createGroundObjectFromModel(const char* pModel, float pX, float pY, float pZ, float pHeading, uint8 pType = 0x00, uint32 pDecayTime = 0);
	uint16 createDoor(const char* pModel, float pX, float pY, float pZ, float pHeading, uint8 pType = 0, uint16 pSize = 100);
	void zoneWho(Client* pClient, Who_All_Struct* pWho);
	void unMarkNPC(uint16 pID);

	void gateAllClients();
	void signalAllClients(uint32 pData);
	void updateQGlobal(uint32 pID, QGlobal pNewGlobal);
	void deleteQGlobal(std::string pName, uint32 pNPCID, uint32 pCharacterID, uint32 pZoneID);
	void sendFindableNPCList(Client* pClient);
	void updateFindableNPCState(NPC *pNPC, bool pRemove);
	void hideCorpses(Client* pClient, uint8 pCurrentMode, uint8 pNewMode);
	
	void getTargetsForConeArea(Mob *pStart, uint32 pRadius, uint32 pHeight, std::list<Mob*>&pMOBs);

	void depopAll(int pNPCTypeID, bool pStartSpawnTimer = true);

	uint16 getFreeID();
	void refreshAutoXTargets(Client* pClient);
	void refreshClientXTargets(Client* pClient);

protected:
	friend class Zone;
	void depop(bool pStartSpawnTimer = false);

private:
	void addToSpawnQueue(uint16 pEntityID, NewSpawn_Struct** pApp);
	void checkSpawnQueue();

	//used for limiting spawns
	class SpawnLimitRecord { public: uint32 mSpawnGroupID; uint32 mNPCTypeID; };
	std::map<uint16, SpawnLimitRecord> mNPCLimits;		//entity id -> npc type

	uint32	tsFirstSpawnOnQueue; // timestamp that the top spawn on the spawnqueue was added, should be 0xFFFFFFFF if queue is empty
	uint32	NumSpawnsOnQueue;
	LinkedList<NewSpawn_Struct*> SpawnQueue;

	std::unordered_map<uint16, Client*> mClients;
	std::unordered_map<uint16, Mob*> mMOBs;
	std::unordered_map<uint16, NPC*> mNPCs;
	std::unordered_map<uint16, Merc*> mMercs;
	std::unordered_map<uint16, Corpse*> mCorpses;
	std::unordered_map<uint16, Object*> mObjects;
	std::unordered_map<uint16, Doors*> mDoors;
	std::unordered_map<uint16, Trap*> mTraps;
	std::unordered_map<uint16, Beacon*> mBeacons;
	std::list<NPC*> mProximities;
	std::list<Group*> mGroups;
	std::list<Raid*> mRaids;
	std::list<Area> mAreas;
	std::queue<uint16> mFreeIDs;

};

class BulkZoneSpawnPacket {
public:
	BulkZoneSpawnPacket(Client* iSendTo, uint32 iMaxSpawnsPerPacket);	// 0 = send zonewide
	virtual ~BulkZoneSpawnPacket();

	bool	AddSpawn(NewSpawn_Struct* ns);
	void	SendBuffer();	// Sends the buffer and cleans up everything - can safely re-use the object after this function call (no need to free and do another new)
private:
	uint32	pMaxSpawnsPerPacket;
	uint32	index;
	NewSpawn_Struct* data;
	Client* pSendTo;
};

#endif

