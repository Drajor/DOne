#pragma once

#include "types.h"
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
class NPCFactory;
class TitleManager;
class LootHandler;
class TradeHandler;
class ShopHandler;
class TaskDataStore;

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
	u16 mID = 0;
	Vector3 mPosition;
	float mHeading = 0.0f;

	Vector3 mDestinationPosition;
	float mDestinationHeading = 0.0f;

	u16 mDestinationZoneID = 0;
	u16 mDestinationInstanceID = 0;
};

static const u16 MaxActorID = 65535;

class Zone {
public:
	Zone(const u16 pPort, const u16 pZoneID, const u16 pInstanceID);
	~Zone();

	const bool initialise(ZoneManager* pZoneManager, ILogFactory* pLogFactory, Data::Zone* pZoneData, Experience::Calculator* pExperienceCalculator, GroupManager* pGroupManager, RaidManager* pRaidManager, GuildManager* pGuildManager, TitleManager* pTitleManager, CommandHandler* pCommandHandler, ItemFactory* pItemFactory, NPCFactory* pNPCFactory, TaskDataStore* pTaskDataStore);

	// Mutes a Character.
	const bool mute(Character* pCharacter, const String& pCharacterName);

	// Unmutes a Character.
	const bool unmute(Character* pCharacter, const String& pCharacterName);

	void onEnterZone(Character* pCharacter);
	void onLeaveZone(Character* pCharacter);
	void onCampComplete(Character* pCharacter);
	void onLinkdeadBegin(Character* pCharacter);
	void onLinkdeadEnd(Character* pCharacter);

	const bool giveItem(Character* pCharacter, Item* pItem, const String& pReason);
	const bool giveItems(Character* pCharacter, std::list<Item*>& pItems, const String& pReason);
	const bool giveStackableItem(Character* pCharacter, Item* pItem, const String& pReason);

	// Chat Events.
	const bool onChannelMessage(Character* pCharacter, const u32 pChannelID, const u32 pType, const String& pSenderName, const String& pTargetName, const String& pMessage);
	const bool onHail(Character* pCharacter);
	const bool onGuildMessage(Character* pCharacter, const String& pMessage);
	const bool onGroupMessage(Character* pCharacter, const String& pMessage);
	const bool onShoutMessage(Character* pCharacter, const String& pMessage);
	const bool onAuctionMessage(Character* pCharacter, const String& pMessage);
	const bool onOOCMessage(Character* pCharacter, const String& pMessage);
	const bool onBroadcastMessage(Character* pCharacter, const String& pMessage);
	const bool onTellMessage(Character* pCharacter, const String& pTargetName, const String& pMessage);
	const bool onSayMessage(Character* pCharacter, const String& pMessage);
	const bool onGMSayMessage(Character* pCharacter, const String& pMessage);
	const bool onRaidMessage(Character* pCharacter, const String& pMessage);
	const bool onUCSMessage(Character* pCharacter, const String& pMessage);
	const bool onEmoteMessage(Character* pCharacter, const String& pMessage);

	// Misc.
	const bool onPetCommand(Character* pCharacter, const u32 pCommand);
	const bool onEmote(Character* pCharacter, const String& pMessage); // This comes from using the /em command.

	// Group Events.
	void onGroupInvite(Character* pInviter, const String& pInviteeName);
	void onGroupInviteAccept(Character* pCharacter);
	void onGroupInviteDecline(Character* pCharacter);
	void onGroupLeave(Character* pCharacter);
	void onGroupRemove(Character* pCharacter, const String& pCharacterName);
	void onGroupMakeLeader(Character* pCharacter, const String& pTargetName);
	void onGroupRoleChange(Character* pCharacter, const String& pTargetName, const u32 pRoleID, const u8 pToggle);

	// Raid Events.
	const bool onRaidInviteDecline(Character* pCharacter);

	// Guild Events.
	const bool onGuildCreate(Character* pCharacter, const String& pGuildName);
	const bool onGuildDelete(Character* pCharacter);
	const bool onGuildInvite(Character* pInviter, const String& pInviteeName);
	const bool onGuildInviteAccept(Character* pCharacter);
	const bool onGuildInviteDecline(Character* pCharacter);
	const bool onGuildRemove(Character* pRemover, const String& pRemoveeName);
	const bool onGuildLeave(Character* pCharacter);
	const bool onGuildPromote(Character* pPromoter, const String& pPromoteeName);
	const bool onGuildDemote(Character* pDemoter, const String& pDemoteeName);
	const bool onGuildSetMOTD(Character* pCharacter, const String& pMOTD);
	const bool onGuildMOTDRequest(Character* pCharacter);
	const bool onGuildSetURL(Character* pSetter, const String& pURL);
	const bool onGuildSetChannel(Character* pSetter, const String& pChannel);
	const bool onGuildMakeLeader(Character* pCharacter, const String& pLeaderName);
	const bool onGuildSetFlags(Character* pSetter, const String& pCharacterName, const u32 pFlags);
	const bool onGuildSetPublicNote(Character* pSetter, const String& pCharacterName, const String& pPublicNote);

	// Loot Events.
	void onLootRequest(Character* pCharacter, const u32 pSpawnID);
	void onLootFinished(Character* pCharacter);
	void onLootItem(Character* pCharacter, const u32 pSlotID);

	// Shop Events.
	void onShopRequest(Character* pCharacter, const u32 pSpawnID);
	void onShopFinished(Character* pCharacter);
	void onShopSell(Character* pCharacter, const u32 pSlotID, const u32 pStacks);
	void onShopBuy(Character* pCharacter, const u32 pInstanceID, const u32 pStacks);

	// Trade Events.
	void onTradeRequest(Character* pCharacter, const u32 pSpawnID);
	void onTradeResponse(Character* pCharacter, const u32 pSpawnID);
	void onTradeBusy(Character* pCharacter, const u32 pSpawnID);
	void onTradeAccept(Character* pCharacter, const u32 pSpawnID);
	void onTradeCancel(Character* pCharacter, const u32 pSpawnID);

	void onChangeGuild(Character* pCharacter);
	void onGuildsChanged();

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
	inline const u32 getLongNameStringID() const { return mLongNameStringID; }
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

	// Find Actor by Spawn ID.
	inline Actor* getActor(const u16 pSpawnID) const { return mActors[pSpawnID]; }

	void addActor(Actor* pActor);
	void removeActor(Actor* pActor);

	// The Character is requesting the list of titles available to them.
	const bool onRequestTitles(Character* pCharacter);

	// The Character has set their title.
	const bool onSetTitle(Character* pCharacter, const u32 pTitleID);

	// The Character has set their suffix.
	const bool onSetSuffix(Character* pCharacter, const u32 pSuffixID);

	// The Character is requesting history for a task.
	void onTaskHistoryRequest(Character* pCharacter, const u32 pIndex);

	void onRemoveTask(Character* pCharacter, const u32 pIndex, const u32 pTaskType);

	const bool onSizeChange(Actor * pActor, float pSize);
	const bool onAnimationChange(Actor* pActor, const u8 pAnimation, const u8 pSpeed = 10, const bool pIncludeSender = false);
	const bool onTargetChange(Character* pCharacter, const u16 pSpawnID);
	const bool onWearChange(Character* pCharacter, const u32 pMaterialID, u32 pEliteMaterialID, u32 pColour, u8 pSlot);
	void onCampBegin(Character* pCharacter);
	void onDropItem(Character* pCharacter);
	void onInspectRequest(Character* pCharacter, const u32 pActorID);

	// The Character is requesting to remove a buff.
	void onRemoveBuffRequest(Character* pCharacter, const u32 pActorID, const u32 pSlotID);

	void handleActorPositionChange(Actor* pActor);

	void handleAFK(Character* pCharacter);
	void handleShowHelm(Actor* pActor);
	void handleAnonymous(Character* pCharacter);
	void handleStanding(Character* pCharacter);
	void handleSitting(Character* pCharacter);
	void handleCrouching(Character* pCharacter);

	void handleLevelIncrease(Character* pCharacter);
	void handleLevelDecrease(Character* pCharacter);
	void notifyCharacterGM(Character* pCharacter);

	void handleFaceChange(Character* pCharacter);

	void handleDeath(Actor* pActor, Actor* pKiller, const u32 pDamage, const u32 pSkill);

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

	void handleDamage(Actor* pAttacker, Actor* pDefender, const i32 pAmount, const u8 pType, const u16 pSpellID);

	void handleZoneChange(Character* pCharacter, const u16 pZoneID, const u16 pInstanceID, const Vector3& pPosition);

	void moveCharacter(Character* pCharacter, float pX, float pY, float pZ);
	inline void moveCharacter(Character* pCharacter, Vector3& pPosition) { moveCharacter(pCharacter, pPosition.x, pPosition.y, pPosition.z); }
	u16 getNextSpawnID() { return mNextSpawnID++; }

	bool trySendTell(const String& pSenderName, const String& pTargetName, const String& pMessage);
	void processCharacterQueuedTells(Character* );
	void getWhoMatches(std::list<Character*>& pResults, const WhoFilter& pFilter);
	void saveCharacter(Character* pCharacter);

	
	void handleVisibilityAdd(Character* pCharacter, Actor* pAddActor);
	void handleVisibilityRemove(Character* pCharacter, Actor* pRemoveActor);

	void onSurnameChange(Actor* pActor, const String& pSurname);

	void onCast(Character* pCharacter, const u16 pTargetID, const u16 pSlot, const u16 pSpellID);
	void onBeginCast(Actor* pActor, const u16 pTargetID, const u32 pSpellID, const u32 pCastTime);
	void onFinishCast(Actor* pActor);

	void sendToVisible(Actor* pActor, EQApplicationPacket* pPacket);
	void sendToVisible(Actor* pActor, EQApplicationPacket* pPacket, bool pIncludeSender);

	void sendToVisible(Character* pCharacter, EQApplicationPacket* pPacket, bool pIncludeSender);
	void sendToTargeters(Actor* pActor, EQApplicationPacket* pPacket);

	void handleConsider(Character* pCharacter, const u32 pSpawnID);
	void handleConsiderCorpse(Character* pCharacter, const u32 pSpawnID);

	// Character is moving an Item.
	const bool onMoveItem(Character* pCharacter, const u32 pFromSlot, const u32 pToSlot, const u32 pStacks);

	// Character is equipping an Item.
	const bool onEquipItem(Character* pCharacter, const u32 pFromSlot, const u32 pToSlot, const u32 pStacks);

	// Character is unequipping an Item.
	const bool onUnequipItem(Character* pCharacter, const u32 pFromSlot, const u32 pToSlot, const u32 pStacks);

	// Character is moving currency.
	const bool onMoveCurrency(Character* pCharacter, const u32 pFromSlot, const u32 pToSlot, const u32 pFromType, const u32 pToType, const i32 pAmount);

	const ZonePointList& getZonePoints() { return mZonePoints; }
	const ObjectList& getObjects() { return mObjects; }

	const bool canBank(Character* pCharacter);

	void handleCriticalHit(Actor* pActor, const i32 pDamage);
	void handleHPChange(Actor* pActor);

	void handleNimbusAdded(Actor * pActor, const u32 pNimbusID);
	void handleNimbusRemoved(Actor * pActor, const u32 pNimbusID);

	void handleRandomRequest(Character* pCharacter, const u32 pLow, const u32 pHigh);
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
	ZoneConnectionManager* mZoneConnectionManager = nullptr;
	ZoneManager* mZoneManager = nullptr;
	ILogFactory* mLogFactory = nullptr;
	ILog* mLog = nullptr;
	AccountManager* mAccountManager = nullptr;
	SpawnPointManager* mSpawnPointManager = nullptr;
	GuildManager* mGuildManager = nullptr;
	GroupManager* mGroupManager = nullptr;
	RaidManager* mRaidManager = nullptr;
	TitleManager* mTitleManager = nullptr;
	CommandHandler* mCommandHandler = nullptr;
	Experience::Calculator* mExperienceCalculator = nullptr;
	Experience::Modifier* mExperienceModifer = nullptr;
	LootAllocator* mLootAllocator = nullptr;
	Scene* mScene = nullptr;
	ItemFactory* mItemFactory = nullptr;
	Transmutation* mTransmutation = nullptr;
	NPCFactory* mNPCFactory = nullptr;
	LootHandler* mLootHandler = nullptr;
	TradeHandler* mTradeHandler = nullptr;
	ShopHandler* mShopHandler = nullptr;
	TaskDataStore* mTaskDataStore = nullptr;

	const bool loadZonePoints(Data::ZonePointList pZonePoints);
	const bool loadObjects(Data::ObjectList pObjects);
	const bool loadDoors(Data::DoorList pDoors);

	// Performs a global Character search.
	Character* _findCharacter(const String& pCharacterName, bool pIncludeZoning = false);

	void _updateCharacters();
	void _updateNPCs();

	void sendMessage(Character* pCharacter, const u32 pChannel, const String pMessage);
	void _sendSpawnAppearance(Actor* pActor, const u16 pType, const u32 pParameter, const bool pIncludeSender = false);
	void _sendAppearanceUpdate(Actor* pActor);
	void _sendLevelAppearance(Character* pCharacter);
	void _sendActorLevel(Actor* pActor);

	u8 mZoneType = 0;
	u8 mTimeType = 0;
	u8 mSkyType = 0;
	float mFogDensity = 0.0f;
	float mMinimumClip = 0.0f;
	float mMaximumClip = 0.0f;

	u32 mLongNameStringID = 0;
	String mLongName = "";
	String mShortName = "";
	Vector3 mSafePoint;
	u16 mNextSpawnID = 1;
	const u16 mID;
	const u16 mInstanceID;
	const u16 mPort;

	bool mShuttingDown = false;
	bool mPopulated = false;

	ZonePoint* _getClosestZonePoint(const Vector3& pPosition);
	ZonePointList mZonePoints;

	std::list<Character*> mCharacters;
	std::list<NPC*> mNPCs;
	Actor* mActors[MaxActorID]; // Lookup table by SpawnID. cost ~264kb per Zone.

	ObjectList mObjects;
	DoorList mDoors;
};