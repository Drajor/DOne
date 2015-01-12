#pragma once

#include "Constants.h"
#include "ClientAuthentication.h"
#include "../common/timer.h"
#include "Vector3.h"

#include <memory>

class EQApplicationPacket;
class IDataStore;
class ILog;
class ILogFactory;
class ZoneConnectionManager;
class World;
class ZoneManager;
class Character;
class Group;
class GroupManager;
class Guild;
class Raid;
class RaidManager;
class Actor;
class NPC;
class Scene;
class SpawnPointManager;
class GuildManager;
struct ZonePoint;
class LootAllocator;
class Item;
class Object;
class Door;
class CommandHandler;
class ItemFactory;
class Transmutation;
class AccountManager;

namespace Experience {
	class Calculator;
	struct CalculationResult;
	struct GainResult;
	struct Context;
	struct Modifier;
}

namespace Data {
	struct Zone;
	struct SpawnGroup;
	struct SpawnPoint;
	struct ZonePoint;
	struct Object;
	struct Door;
	typedef std::list<Data::Object*>& ObjectList;
	typedef std::list<Data::Door*>& DoorList;
	typedef std::list<Data::SpawnGroup*>& SpawnGroupList;
	typedef std::list<Data::SpawnPoint*>& SpawnPointList;
	typedef std::list<Data::ZonePoint*>& ZonePointList;
}

typedef std::list<ZonePoint*> ZonePointList;
typedef std::list<Object*> ObjectList;
typedef std::list<Door*> DoorList;

struct ZonePoint {
	uint16 mID = 0;
	Vector3 mPosition;

	Vector3 mDestinationPosition;
	float mDestinationHeading = 0.0f;

	uint16 mDestinationZoneID = 0;
	uint16 mDestinationInstanceID = 0;
};

class Zone {
public:
	Zone(const u16 pPort, const u16 pZoneID, const u16 pInstanceID);
	~Zone();

	const bool initialise(ZoneManager* pZoneManager, ILogFactory* pLogFactory, Data::Zone* pZoneData, Experience::Calculator* pExperienceCalculator, GroupManager* pGroupManager, RaidManager* pRaidManager, GuildManager* pGuildManager, CommandHandler* pCommandHandler, ItemFactory* pItemFactory);

	void onEnterZone(Character* pCharacter);
	void onLeaveZone(Character* pCharacter);
	void onCampComplete(Character* pCharacter);
	void onLinkdeadBegin(Character* pCharacter);
	void onLinkdeadEnd(Character* pCharacter);

	const bool canShutdown() const;
	const bool shutdown();
	
	// Returns whether this Zone is currently shutting down.
	inline const bool isShuttingDown() const { return mShuttingDown; }

	const bool update();

	const bool populate();
	const bool depopulate();

	const bool checkAuthentication(Character* pCharacter);

	inline const String& getLongName() const { return mLongName; }
	inline const String& getShortName() const { return mShortName; }
	inline const Vector3& getSafePoint() const { return mSafePoint; }
	inline const uint32 getLongNameStringID() const { return mLongNameStringID; }
	inline const float getGravity() const { return 0.4f; }
	inline const float getMinimumZ() const { return -5000.0f; } // NOTE: The lowest point in the zone a Character should be able to reach.
	inline const u8 getZoneType() const { return mZoneType; }
	inline const u8 getTimeType() const { return mTimeType; }
	inline const u8 getSkyType() const { return mSkyType; }
	inline const float getFogDensity() const { return mFogDensity; }
	inline const float getMinimumClip() const { return mMinimumClip; }
	inline const float getMaximumClip() const { return mMaximumClip; }
	
	inline const u32 getNumCharacters() const { return mCharacters.size(); }
	inline const u16 getID() const { return mID; }
	inline const u16 getInstanceID() const { return mInstanceID; }
	inline const u16 getPort() const { return mPort; }

	Character* findCharacter(const String pCharacterName) const;

	// Search for an Actor by Spawn ID.
	Actor* findActor(const uint32 pSpawnID) const;

	void addActor(Actor* pActor);
	void removeActor(Actor* pActor);

	void handleActorPositionChange(Actor* pActor);
	void handleLinkDead(Character* pCharacter);
	void handleAFK(Character* pCharacter);
	void handleShowHelm(Actor* pActor);
	void handleAnonymous(Character* pCharacter);
	void handleStanding(Character* pCharacter);
	void handleSitting(Character* pCharacter);
	void handleCrouching(Character* pCharacter);

	void handleChannelMessage(Character* pCharacter, const u32 pChannelID, const String& pSenderName, const String& pTargetName, const String& pMessage);

	void handleSay(Character* pCharacter, const String pMessage);
	void handleShout(Character* pCharacter, const String pMessage);
	void handleOOC(Character* pCharacter, const String pMessage);
	void handleEmote(Character* pCharacter, const String pMessage);
	void handleAuction(Character* pCharacter, const String pMessage);
	void handleTell(Character* pCharacter, const String& pTargetName, const String& pMessage);
	void handleAnimation(Actor* pActor, const uint8 pAnimation, const uint8 pSpeed = 10, const bool pIncludeSender = false);
	void handleLevelIncrease(Character* pCharacter);
	void handleLevelDecrease(Character* pCharacter);
	void notifyCharacterGM(Character* pCharacter);

	void handleTarget(Character* pCharacter, const uint16 pSpawnID);
	void handleFaceChange(Character* pCharacter);

	void handleDeath(Actor* pActor, Actor* pKiller, const uint32 pDamage, const uint32 pSkill);

	// Handles specifics of NPC death.
	void _handleDeath(NPC* pNPC, Actor* pKiller);

	// Handles specifics of Character death.
	void _handleDeath(Character* pCharacter, Actor* pKiller);

	// Allocate experience for an NPC kill (Solo).
	void allocateSoloExperience(Character* pCharacter, NPC* pNPC);

	// Allocate experience for an NPC kill (Group).
	void allocateGroupExperience(Group* pGroup, NPC* pNPC);

	// Allocate experience for an NPC kill (Raid).
	void allocateRaidExperience(Raid* pRaid, NPC* pNPC);

	void processExperienceResult(Character* pCharacter, Experience::CalculationResult& pCalculationResult, Experience::Context& pContent);

	void handleDamage(Actor* pAttacker, Actor* pDefender, const int32 pAmount, const uint8 pType, const uint16 pSpellID);

	// Guild
	void onGuildsChanged();
	void notifyCharacterGuildChange(Character* pCharacter);

	void handleZoneChange(Character* pCharacter, const uint16 pZoneID, const uint16 pInstanceID, const Vector3& pPosition);

	void moveCharacter(Character* pCharacter, float pX, float pY, float pZ);
	inline void moveCharacter(Character* pCharacter, Vector3& pPosition) { moveCharacter(pCharacter, pPosition.x, pPosition.y, pPosition.z); }
	uint16 getNextSpawnID() { return mNextSpawnID++; }

	bool trySendTell(const String& pSenderName, const String& pTargetName, const String& pMessage);
	void processCharacterQueuedTells(Character* );
	void getWhoMatches(std::list<Character*>& pResults, const WhoFilter& pFilter);
	void requestSave(Character* pCharacter);

	
	void handleVisibilityAdd(Character* pCharacter, Actor* pAddActor);
	void handleVisibilityRemove(Character* pCharacter, Actor* pRemoveActor);

	void handleSurnameChange(Actor* pActor);
	void handleTitleChanged(Character* pCharacter, const uint32 pOption);

	void handleCastingBegin(Character* pCharacter, const uint16 pSlot, const uint32 pSpellID);
	void handleCastingFinished(Actor* pActor);

	void sendToVisible(Actor* pActor, EQApplicationPacket* pPacket);
	void sendToVisible(Character* pCharacter, EQApplicationPacket* pPacket, bool pIncludeSender);
	void sendToTargeters(Actor* pActor, EQApplicationPacket* pPacket);

	void handleBeginLootRequest(Character* pCharacter, const uint32 pCorpseSpawnID);
	void handleEndLootRequest(Character* pCharacter);
	void handleLootItem(Character* pCharacter, Actor* pCorpse, const uint32 pSlotID);

	void handleConsider(Character* pCharacter, const uint32 pSpawnID);
	void handleConsiderCorpse(Character* pCharacter, const uint32 pSpawnID);
	
	void handleTradeRequest(Character* pCharacter, const uint32 pToSpawnID);
	void handleTradeAccept(Character* pCharacter, const uint32 pSpawnID);
	void handleTradeCancel(Character* pCharacter, const uint32 pSpawnID);

	void handleShopRequest(Character* pCharacter, const uint32 pSpawnID);
	void handleShopEnd(Character* pCharacter, const uint32 pSpawnID);
	void handleShopSell(Character* pCharacter, const uint32 pSpawnID, const uint32 pSlotID, const uint32 pStacks);
	void handleShopBuy(Character* pCharacter, const uint32 pSpawnID, const uint32 pItemInstanceID, const uint32 pStacks);
	const bool _handleShopBuy(Character* pCharacter, NPC* pNPC, Item* pItem, const uint32 pStacks);

	const ZonePointList& getZonePoints() { return mZonePoints; }
	const ObjectList& getObjects() { return mObjects; }

	const bool canBank(Character* pCharacter);
	const bool canShop(Character* pCharacter, NPC* pMerchant);

	void handleCriticalHit(Actor* pActor, const int32 pDamage);
	void handleHPChange(Actor* pActor);

	void handleNimbusAdded(Actor * pActor, const uint32 pNimbusID);
	void handleNimbusRemoved(Actor * pActor, const uint32 pNimbusID);

	void handleRandomRequest(Character* pCharacter, const uint32 pLow, const uint32 pHigh);
	void handleDropItem(Character* pCharacter);
	void handleAppearanceChange(Actor* pActor);
	void handleRespawnSelection(Character* pCharacter, const u32 pSelection);

	void handleWhoRequest(Character* pCharacter, WhoFilter& pFilter);

	void handleSetLevel(Actor* pActor, const u8 pLevel);
	void handleSetLevel(Character* pCharacter, const u8 pLevel);
	void handleSetLevel(NPC* pNPC, const u8 pLevel);

	void _handleLevelChange(Character* pCharacter, const u8 pPreviousLevel, const u8 pCurrentLevel);
	
	// Gives normal experience to a Character.
	void giveExperience(Character* pCharacter, const u32 pExperience);

	// Gives Alternate Advanced experience to a Character.
	void giveAAExperience(Character* pCharacter, const u32 pExperience);

	// Gives Group Leadership experience to a Character.
	void giveGroupLeadershipExperience(Character* pCharacter, const u32 pExperience);

	// Gives Raid Leadership experience to a Character.
	void giveRaidLeadershipExperience(Character* pCharacter, const u32 pExperience);

	// Returns the experience modifier on this Zone.
	inline Experience::Modifier* getExperienceModifier() { return mExperienceModifer; }
	
	void onCombine(Character* pCharacter, const u32 pSlot);

private:

	bool mInitialised = false;
	ItemFactory* mItemFactory = nullptr;
	Transmutation* mTransmutation = nullptr;

	const bool loadZonePoints(Data::ZonePointList pZonePoints);
	const bool loadObjects(Data::ObjectList pObjects);
	const bool loadDoors(Data::DoorList pDoors);

	// Performs a global Character search.
	Character* _findCharacter(const String& pCharacterName, bool pIncludeZoning = false);

	void _updateCharacters();
	void _updateNPCs();

	void _sendChat(Character* pCharacter, const u32 pChannel, const String pMessage);
	void _sendSpawnAppearance(Actor* pActor, const u16 pType, const uint32 pParameter, const bool pIncludeSender = false);
	void _sendAppearanceUpdate(Actor* pActor);
	void _sendLevelAppearance(Character* pCharacter);
	void _sendActorLevel(Actor* pActor);

	u8 mZoneType = 0;
	u8 mTimeType = 0;
	u8 mSkyType = 0;
	float mFogDensity = 0.0f;
	float mMinimumClip = 0.0f;
	float mMaximumClip = 0.0f;

	uint32 mLongNameStringID = 0;
	String mLongName = "";
	String mShortName = "";
	Vector3 mSafePoint;
	uint16 mNextSpawnID = 1;
	const u16 mID;
	const u16 mInstanceID;
	const u16 mPort;

	bool mShuttingDown = false;
	bool mPopulated = false;

	ZonePoint* _getClosestZonePoint(const Vector3& pPosition);
	ZonePointList mZonePoints;

	ZoneConnectionManager* mZoneConnectionManager = nullptr;
	ZoneManager* mZoneManager = nullptr;
	ILogFactory* mLogFactory = nullptr;
	ILog* mLog = nullptr;
	AccountManager* mAccountManager = nullptr;
	SpawnPointManager* mSpawnPointManager = nullptr;
	GuildManager* mGuildManager = nullptr;
	GroupManager* mGroupManager = nullptr;
	RaidManager* mRaidManager = nullptr;
	CommandHandler* mCommandHandler = nullptr;
	Experience::Calculator* mExperienceCalculator = nullptr;
	Experience::Modifier* mExperienceModifer = nullptr;
	LootAllocator* mLootAllocator = nullptr;
	Scene* mScene = nullptr;

	std::list<Character*> mCharacters;
	std::list<NPC*> mNPCs;
	std::list<Actor*> mActors;
	ObjectList mObjects;
	DoorList mDoors;
};