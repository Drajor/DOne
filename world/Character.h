#pragma once

#include "Actor.h"
#include "Utility.h"
#include "ClientAuthentication.h"
#include "../common/timer.h"
#include "BindLocation.h"

#include <memory>

class Account;
class NPC;
class Group;
class Guild;
class Raid;
class EQApplicationPacket;
class ZoneConnection;
class ExtendedTargetController;
class Inventoryy;
class Item;
class RespawnOptions;

namespace Experience {
	class Controller;
}

namespace Data {
	struct Character;
	struct Spell;
}

struct GroupInvitation {
	u32 mTimeInvited = 0;
	String mInviterName;

	inline void clear() {
		mTimeInvited = 0;
		mInviterName.clear();
	}
};

struct RaidInvitation {
	u32 mTimeInvited = 0;
	String mInviterName;

	inline void clear() {
		mTimeInvited = 0;
		mInviterName.clear();
	}
};

struct GuildInvitation {
	u32 mTimeInvited = 0;
	u32 mGuildID = 0;
	String mInviterName;

	inline void clear() {
		mTimeInvited = 0;
		mGuildID = 0;
		mInviterName.clear();
	}
};

struct TradeRequest {
	u32 mTimeRequested = 0;
	u16 mSpawnID = 0;

	inline void clear() {
		mTimeRequested = 0;
		mSpawnID = 0;
	}
};

class Character : public Actor {
	friend ZoneConnection;
public:
	struct ZoneChange {
		uint16 mZoneID = 0;
		uint16 mInstanceID = 0;
	};
public:
	Character(Data::Character* pCharacterData);
	~Character();

	const bool initialise(Account* pAccount, Inventoryy* pInventory, Experience::Controller* pExperienceController);
	
	// Returns the Account this Character belongs to.
	inline Account* getAccount() const { return mAccount; }

	inline const bool isCharacter() const { return true; }
	inline const bool isInitialised() const { return mInitialised; }
	inline const bool isZoning() const { return mIsZoning; }
	inline const bool isCamping() const { return mIsCamping; }
	inline void setZoning(const bool pZoning) { mIsZoning = pZoning; }
	inline const bool isLinkDead() const { return mIsLinkDead; }
	inline void setLinkDead() { mIsLinkDead = true; }

	inline const bool isHidden() const { return mHidden; }
	inline void setHidden(const bool pValue) { mHidden = pValue; }

	inline const bool isMuted() const { return mIsMuted; }
	inline void setMuted(const bool pValue) { mIsMuted = pValue; }

	inline const bool isNew() const { return mIsNew; }
	inline void setNew(const bool pValue) { mIsNew = pValue; }

	const u8 getLevel() const;
	
	void setLevel(const u8 pLevel);

	// Returns a reference to a specified BindLocation.
	inline const BindLocation& getBindLocation(const i32 pIndex) const { return mBindLocations[pIndex]; }

	/////////////////////////////////////////////////////////////////////
	// Trade

	// Returns the Actor this Character is trading with.
	inline Actor* getTradingWith() const { return mTradingWith; }

	// Sets the Actor this Character is trading with.
	inline void setTradingWith(Actor* pActor) { mTradingWith = pActor; }

	// Returns whether this Character is trading.
	inline const bool isTrading() const { return getTradingWith() != nullptr; }

	// Returns whether this Character is trading with another Character.
	inline const bool isTradingWithCharacter() const { return isTrading() && getTradingWith()->isCharacter(); }

	// Returns whether this Character is trading with an NPC.
	inline const bool isTradingWithNPC() const { return isTrading() && getTradingWith()->isNPC(); }

	// Returns whether this Character accepts the current trade.
	inline const bool isTradeAccepted() const { return mTradeAccepted; }

	// Sets whether this Character accepts the current trade.
	inline void setTradeAccepted(const bool pValue) { mTradeAccepted = pValue; }

	/////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////
	// Shopping

	// Returns the NPC this Character is shopping with.
	inline NPC* getShoppingWith() const { return mShoppingWith; }

	// Sets the NPC this Character is shopping with.
	inline void setShoppingWith(NPC* pNPC) { mShoppingWith = pNPC; }

	// Returns whether this Character is shopping.
	inline const bool isShopping() const { return getShoppingWith() != nullptr; }

	/////////////////////////////////////////////////////////////////////

	// Zone Authentication
	uint16 mAuthenticatedZoneID = 0;
	uint16 mAuthicatedInstanceID = 0;
	inline const uint16 getAuthenticatedZoneID() const { return mAuthenticatedZoneID; }
	inline const uint16 getAuthenticatedInstanceID() const { return mAuthicatedInstanceID; }
	inline void setZoneAuthentication(const uint16 pZoneID, const uint16 pInstanceID) { mAuthenticatedZoneID = pZoneID; mAuthicatedInstanceID = pInstanceID; }
	inline const bool checkZoneAuthentication(const uint16 pZoneID, const uint16 pInstanceID) const { return mAuthenticatedZoneID == pZoneID && mAuthicatedInstanceID == pInstanceID; }

	inline Inventoryy* getInventory() const { return mInventory; }
	inline ExtendedTargetController* getXTargetController() { return mXTargetController; }
	inline RespawnOptions* getRespawnOptions() const { return mRespawnOptions; }
	inline Experience::Controller* getExperienceController() const { return mExperienceController; }
	
	bool onEnterZone();
	void onLeaveZone();
	const bool onDeath();

	void addQueuedMessage(const u32 pChannel, const String& pSenderName, const String& pMessage);

	// Returns whether or not this Character has a pending trade request.
	inline const bool hasTradeRequest() const { return mTradeRequest.mTimeRequested != 0; }

	// Returns a reference to the trade request.
	inline TradeRequest& getTradeRequest() { return mTradeRequest; }

	inline void clearTradeRequest() { mTradeRequest.clear(); }

	// Returns whether this Character has neither a Group nor a Raid.
	const bool isSolo() const { return !hasGroup() && !hasRaid(); }

	/////////////////////////////////////////////////////////////////////
	// Group
	
	// Returns whether or not this Character has a Group.
	inline bool hasGroup() const { return mGroup != nullptr; }

	// Returns the Group this Character belongs to.
	inline Group* getGroup() { return mGroup; }

	// Sets the Group this Character is a member of.
	void setGroup(Group* pGroup);

	// Clears this Character's Group.
	inline void clearGroup() { setGroup(nullptr); }

	// Returns whether or not this Character is the Group leader.
	const bool isGroupLeader();

	inline void setIsGroupMainTank(const bool pValue) { mGroupMainTank = pValue; }
	inline const bool isGroupMainTank() const { return mGroupMainTank; }

	inline void setIsGroupMainAssist(const bool pValue) { mGroupMainAssist = pValue; }
	inline const bool isGroupMainAssist() const { return mGroupMainAssist; }

	inline void setIsGroupPuller(const bool pValue) { mGroupPuller = pValue; }
	inline const bool isGroupPuller() const { return mGroupPuller; }

	// Returns whether or not this Character has a pending group invitation.
	inline const bool hasGroupInvitation() const { return mGroupInvitation.mTimeInvited != 0; }

	// Returns a reference to the group invitation.
	inline GroupInvitation& getGroupInvitation() { return mGroupInvitation; }

	// Clears the group invitation.
	inline void clearGroupInvitation() { mGroupInvitation.clear(); }

	/////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////
	// Raid
	
	// Returns whether or not this Character has a Raid.
	inline bool hasRaid() const { return mRaid != nullptr; }

	// Returns the Raid this Character belongs to.
	inline Raid* getRaid() { return mRaid; }

	// Sets the Raid this Character is a member of.
	inline void setRaid(Raid* pRaid) { mRaid = pRaid; }

	// Clears this Character's Raid.
	inline void clearRaid() { setRaid(nullptr); }

	// Returns whether or not this Character is the Raid leader.
	const bool isRaidLeader() const;

	// Returns whether or not this Character has a pending raid invitation.
	inline const bool hasRaidInvitation() const { return mRaidInvitation.mTimeInvited != 0; }

	// Returns a reference to the raid invitation.
	RaidInvitation& getRaidInvitation() { return mRaidInvitation; }

	// Clears the raid invitation.
	inline void clearRaidInvitation() { mRaidInvitation.clear(); }

	/////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////
	// Guild

	// Returns whether or not this Character has a Guild.
	inline bool hasGuild() const { return mGuild != nullptr; }

	// Returns the Guild this Character belongs to.
	inline Guild* getGuild() const { return mGuild; }

	// Sets the Guild this Character is a member of.
	void setGuild(Guild* pGuild);

	// Clears this Character's Guild.
	void clearGuild() { setGuild(nullptr); }

	// Returns whether or not this Character has a pending guild invitation.
	inline const bool hasGuildInvitation() const { return mGuildInvitation.mTimeInvited != 0; }

	// Returns a reference to the guild invitation.
	GuildInvitation& getGuildInvitation() { return mGuildInvitation; }

	// Clears the guild invitation.
	inline void clearGuildInvitation() { mGuildInvitation.clear(); }

	/////////////////////////////////////////////////////////////////////

	void setConnection(ZoneConnection* pConnection) { mConnection = pConnection; }
	ZoneConnection* getConnection() { return mConnection; }
	const bool send(EQApplicationPacket* pPacket);
	
	void update();

	void setStanding(bool pStanding);
	inline const u32 getStatus() const { return mStatus; }
	inline void setStatus(const u32 pStatus) { mStatus = pStatus; }
	
	void startCamp();
	inline void setCampComplete(const bool pCampComplete) { mCampComplete = pCampComplete; }
	inline const bool isCampComplete() const { return mCampComplete; }
	inline void setZoningOut() { mIsZoningOut = true; }
	inline const bool isZoningOut() const { return mIsZoningOut; }

	void message(const u32 pType, String pMessage);

	// Target Group Buff
	inline void setTGB(bool pTGB) { mTGB = pTGB; }
	inline const bool getTGB() const { return mTGB; }

	// Healing
	void healPercentage(int pPercent);

	void doAnimation(uint8 pAnimationID);

	static float getDefaultSize(uint32 pRace);

	// Base Stats
	uint32 getBaseStatistic(Statistic pStatistic);
	void setBaseStatistic(Statistic pStatistic, uint32 pValue);

	inline const i32 getBasePoisonResist() const { return 0; }
	inline const i32 getBaseMagicResist() const { return 0; }
	inline const i32 getBaseDiseaseResist() const { return 0; }
	inline const i32 getBaseFireResist() const { return 0; }
	inline const i32 getBaseColdResist() const { return 0; }
	inline const i32 getBaseCorruptionResist() const { return 0; }

	inline const u32 getBaseStrength() const { return mBaseStrength; };
	inline const u32 getBaseStamina() const { return mBaseStamina; };
	inline const u32 getBaseCharisma() const { return mBaseCharisma; };
	inline const u32 getBaseDexterity() const { return mBaseDexterity; };
	inline const u32 getBaseIntelligence() const { return mBaseIntelligence; };
	inline const u32 getBaseAgility() const { return mBaseAgility; };
	inline const u32 getBaseWisdom() const { return mBaseWisdom; };

	// 
	inline const u32 getStrength() const { return 0; };
	inline const u32 getStamina() const { return 0; };
	inline const u32 getCharisma() const { return 0; };
	inline const u32 getDexterity() const { return 0; };
	inline const u32 getIntelligence() const { return 0; };
	inline const u32 getAgility() const { return 0; };
	inline const u32 getWisdom() const { return 0; };

	// Consent
	inline const bool getAutoConsentGroup() const { return mAutoConsentGroup; }
	inline void setAutoConsentGroup(const bool pConsent) { mAutoConsentGroup = pConsent; }
	inline const bool getAutoConsentRaid() const { return mAutoConsentRaid; }
	inline void setAutoConsentRaid(const bool pConsent) { mAutoConsentRaid = pConsent; }
	inline const bool getAutoConsentGuild() const { return mAutoConsentGuild; }
	inline void setAutoConsentGuild(const bool pConsent) { mAutoConsentGuild = pConsent; }

	inline void addVisibleNPC(NPC* pNPC) { mVisibleNPCs.push_back(pNPC); }
	inline void removeVisibleNPC(NPC* pNPC) { mVisibleNPCs.remove(pNPC); }
	inline std::list<NPC*>& getVisibleNPCs() { return mVisibleNPCs; }
	inline void clearVisibleNPCs() { mVisibleNPCs.clear(); }

	inline const bool isCasting() const { return mIsCasting; }

	// Returns whether this Character has a spell in their SpellBook
	const bool hasSpell(const u32 pSpellID) const;

	// Returns whether this Character has a specific spell in a specific SpellBar slot.
	const bool hasSpell(const uint16 pSlot, const uint32 pSpellID) const;

	// Returns the first free spell book slot.
	const u16 getFreeSpellBookSlot() const;

	// Returns the slot that contains a specified spell.
	const u16 getSpellBookSlot(const u32 pSpellID) const;

	const bool canCast(const uint32 pSpellID) const;
	const bool canRequestTrade() const;
	//const bool canAcceptTradeRequest() const;

	const bool preCastingChecks(const Data::Spell* pSpell);
	const bool postCastingChecks(const Data::Spell* pSpell);
	const bool beginCasting(const uint16 pSlot, const uint32 pSpellID);
	const bool finishCasting();

	const Data::Character* getData() const { return mData; }

	inline const bool isCaster() const { return Utility::isCaster(getClass()); }
	inline const bool isHybrid() const { return Utility::isHybrid(getClass()); }
	inline const bool isMelee() const { return Utility::isMelee(getClass()); }
	inline const bool canTaunt() const { return Utility::canClassTaunt(getClass()); }

	// Deletes a spell from the Spell Book.
	const bool handleDeleteSpell(const int16 pSlot);

	// Swaps two spells in the Spell Book.
	const bool handleSwapSpells(const uint16 pFrom, const uint16 pTo);

	// Adds a spell to the Spell Bar.
	const bool handleMemoriseSpell(const uint16 pSlot, const uint32 pSpellID);

	// Removes a spell from the Spell Bar.
	const bool handleUnmemoriseSpell(const uint16 pSlot);

	// Adds a spell to the Spell Book.
	const bool handleScribeSpell(const u16 pSlot, const u32 pSpellID);

	const uint32 getSkill(const uint32 pSkillID) const;
	const bool setSkill(const uint32 pSkillID, const uint32 pValue);
	const uint32 getAdjustedSkill(const uint32 pSkillID) const;

	const uint32 getLanguage(const uint32 pLanguageID) const;
	const bool setLanguage(const uint32 pLanguageID, const uint32 pValue);

	void notify(const String& pMessage);
	const bool _updateForSave();

	inline const bool isLooting() const { return mLootingCorpse != nullptr; }
	inline void setLootingCorpse(Actor* pCorpse) { mLootingCorpse = pCorpse; }
	inline Actor* getLootingCorpse() { return mLootingCorpse; }

	Actor* findVisible(const uint32 pSpawnID);

	void setZoneChange(const uint16 pZoneID, const uint16 pInstanceID);
	const bool checkZoneChange(const uint16 pZoneID, const uint16 pInstanceID) const;
	void clearZoneChange();
	inline const ZoneChange& getZoneChange() const { return mZoneChange; }
	inline void setFilters(Filters pFilters) { mFilters = pFilters; }

	void setAutoAttack(const bool pAttacking);

	// Returns whether this Character can equip PItem in pSlotID
	const bool canEquip(const Item* pItem, const uint32 pSlotID) const;
	const bool onWornSlotChange(const uint32 pSlot, Item* pOldItem, Item* pNewItem);
	const bool onPrimarySlotChange(Item* pItem);
	const bool onSecondarySlotChange(Item* pItem);
	const bool onRangeItemChange(Item* pItem);

	const bool canCombine() const;
	const bool canShop() const;

	inline const u32 getIntoxication() const { return 0; }

private:

	Account* mAccount = nullptr;

	ZoneChange mZoneChange;
	BindLocation mBindLocations[5];

	bool mInitialised = false;
	ClientAuthentication mAuthentication;
	bool mIsZoning = false;
	bool mIsLinkDead = false;
	Actor* mLootingCorpse = nullptr;
	Actor* mTradingWith = nullptr;
	bool mTradeAccepted = false;
	NPC* mShoppingWith = nullptr;

	Filters mFilters;

	u32 mBaseStrength = 0;
	u32 mBaseStamina = 0;
	u32 mBaseCharisma = 0;
	u32 mBaseDexterity = 0;
	u32 mBaseIntelligence = 0;
	u32 mBaseAgility = 0;
	u32 mBaseWisdom = 0;

	bool mIsCasting = false;
	uint32 mCastingSpellID = 0;
	uint16 mCastingSlot = 0; // The Spell Bar slot ID used to cast a spell
	Timer mCastingTimer;

	std::array<uint32, Limits::Skills::MAX_ID> mSkills;
	std::array<uint32, Limits::Languages::MAX_ID> mLanguages;

	bool mIsMuted = false;
	bool mIsNew = false;
	bool mHidden = false;
	bool mAutoAttacking = false;
	bool mStanding = true;
	bool mIsCamping = false;
	bool mIsZoningOut = false;
	bool mCampComplete = false; // Flag indicating whether this character logged out via /camp
	bool mTGB = false;
	bool mAutoConsentGroup = false;
	bool mAutoConsentRaid = false;
	bool mAutoConsentGuild = false;
	u32 mStatus = 0;
	Timer mCampTimer; // 30 seconds.

	Timer mAutoSave;

	Group* mGroup = nullptr;
	GroupInvitation mGroupInvitation;
	bool mGroupMainTank = false;
	bool mGroupMainAssist = false;
	bool mGroupPuller = false;

	Raid* mRaid = nullptr;
	RaidInvitation mRaidInvitation;

	Guild* mGuild = nullptr;
	GuildInvitation mGuildInvitation;

	ZoneConnection* mConnection = nullptr;
	Data::Character* mData = nullptr;
	
	TradeRequest mTradeRequest;

	std::list<NPC*> mVisibleNPCs; // NPCs that are visible to this Character

	struct QueuedChannelMessage {
		const u32 mChannelID;
		const String mSenderName;
		const String mMessage;
	};
	std::list<QueuedChannelMessage> mMessageQueue;
	void _processMessageQueue();

	// Spell Book
	class SpellBook {
	public:
		SpellBook() {
			mSpellIDs.resize(Limits::SpellBook::MAX_SLOTS);
			for (auto i = 0; i < Limits::SpellBook::MAX_SLOTS; i++)
				mSpellIDs[i] = 0;
		}
		const std::vector<uint32>& getData() const { return mSpellIDs; }
		void setSpell(const uint16 pSlot, const uint32 pSpellID);
		const bool deleteSpell(const uint16 pSlot);
		const bool swapSpells(const uint16 pFrom, const uint16 pTo);
		const bool hasSpell(const uint32 pSpellID);
		const u16 getFreeSlot() const;

		// Returns whether a specific spell slot is free.
		const bool isSlotFree(const u16 pSlot) const;

		// Returns the slot containing a specific spell.
		const u16 getSlot(const u32 pSpellID) const;
	private:
		std::vector<uint32> mSpellIDs;
	};
	SpellBook* mSpellBook = nullptr;
	const std::vector<uint32> getSpellBookData() const;

	// Spell Bar
	class SpellBar {
	public:
		SpellBar() { 
			mSpellIDs.resize(Limits::SpellBar::MAX_SLOTS);
			for (auto& i : mSpellIDs) i = 0;
		}
		const std::vector<uint32>& getData() const { return mSpellIDs; }
		void setSpell(const uint16 pSlot, const uint32 pSpellID);
		const uint32 getSpellID(const uint16 pSlot) const;
	private:
		std::vector<uint32> mSpellIDs;
	};
	SpellBar* mSpellBar = nullptr;
	const std::vector<uint32> getSpellBarData() const;
	// Inventory
	Inventoryy* mInventory = nullptr;

	ExtendedTargetController* mXTargetController = nullptr;
	RespawnOptions* mRespawnOptions = nullptr;
	Experience::Controller* mExperienceController = nullptr;
};