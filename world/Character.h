#pragma once

#include "Actor.h"
#include "Utility.h"
#include "ClientAuthentication.h"
#include "../common/timer.h"
#include "BindLocation.h"

class NPC;
class Group;
class Guild;
class Raid;
class EQApplicationPacket;
class ZoneClientConnection;
class ExtendedTargetController;
class Inventoryy;
class Item;
class RespawnOptions;

namespace Data {
	struct Character;
	struct Spell;
}

class Character : public Actor {
	friend ZoneClientConnection;
public:
	struct ZoneChange {
		uint16 mZoneID = 0;
		uint16 mInstanceID = 0;
	};
public:
	Character(const uint32 pAccountID, Data::Character* pCharacterData);
	~Character();
	inline const uint32 getAccountID() const { return mAccountID; }
	inline const bool isCharacter() const { return true; }
	inline const bool isInitialised() const { return mInitialised; }
	inline const bool isZoning() const { return mIsZoning; }
	inline void setZoning(const bool pZoning) { mIsZoning = pZoning; }
	inline const bool isLinkDead() const { return mIsLinkDead; }
	inline void setLinkDead() { mIsLinkDead = true; }

	inline const bool isHidden() const { return mHidden; }
	inline void setHidden(const bool pValue) { mHidden = pValue; }

	// Returns a reference to a specified BindLocation.
	inline const BindLocation& getBindLocation(const i32 pIndex) const { return mBindLocations[pIndex]; }

	// Returns the Actor this Character is trading with.
	inline Actor* getTradingWith() const { return mTradingWith; }

	// Sets the Actor this Character is trading with.
	inline void setTradingWith(Actor* pActor) { mTradingWith = pActor; }

	// Returns whether this Character is trading.
	inline const bool isTrading() const { return getTradingWith() != nullptr; }

	// Returns the NPC this Character is shopping with.
	inline NPC* getShoppingWith() const { return mShoppingWith; }

	// Sets the NPC this Character is shopping with.
	inline void setShoppingWith(NPC* pNPC) { mShoppingWith = pNPC; }

	// Returns whether this Character is shopping.
	inline const bool isShopping() const { return getShoppingWith() != nullptr; }

	// Zone Authentication
	uint16 mAuthenticatedZoneID = 0;
	uint16 mAuthicatedInstanceID = 0;
	inline const uint16 getAuthenticatedZoneID() const { return mAuthenticatedZoneID; }
	inline const uint16 getAuthenticatedInstanceID() const { return mAuthicatedInstanceID; }
	inline void setZoneAuthentication(const uint16 pZoneID, const uint16 pInstanceID) { mAuthenticatedZoneID = pZoneID; mAuthicatedInstanceID = pInstanceID; }
	inline const bool checkZoneAuthentication(const uint16 pZoneID, const uint16 pInstanceID) const { return mAuthenticatedZoneID == pZoneID && mAuthicatedInstanceID == pInstanceID; }

	bool initialise();

	inline Inventoryy* getInventory() const { return mInventory; }
	inline ExtendedTargetController* getXTargetController() { return mXTargetController; }
	inline RespawnOptions* getRespawnOptions() const { return mRespawnOptions; }
	
	bool onEnterZone();
	bool onZoneOut();
	const bool onDeath();

	void addQueuedMessage(ChannelID pChannel, const String& pSenderName, const String& pMessage);

	// Group
	inline bool hasGroup() { return mGroup != nullptr; }
	inline Group* getGroup() { return mGroup; }
	inline void setGroup(Group* pGroup) { mGroup = pGroup; }

	// Guild
	inline bool hasGuild() { return mGuild != nullptr; }
	inline Guild* getGuild() { return mGuild; }
	inline void setGuild(Guild* pGuild) { mGuild = pGuild; }
	inline void setGuild(Guild* pGuild, GuildID pGuildID, GuildRank pGuildRank) { setGuild(pGuild); setGuildID(pGuildID); setGuildRank(pGuildRank); }
	inline void clearGuild() { setGuild(nullptr, NO_GUILD, GuildRanks::GR_None); }

	// Pending Guild Invite
	bool hasPendingGuildInvite() { return mPendingGuildInviteID != NO_GUILD; }
	GuildID getPendingGuildInviteID() { return mPendingGuildInviteID; }
	void setPendingGuildInviteID(GuildID pGuildID) { mPendingGuildInviteID = pGuildID; }
	String getPendingGuildInviteName() { return mPendingGuildInviteName; }
	void setPendingGuildInviteName(String pName) { mPendingGuildInviteName = pName; }
	void clearPendingGuildInvite() { mPendingGuildInviteID = NO_GUILD; mPendingGuildInviteName = ""; }

	// Raid
	inline bool hasRaid() { return mRaid != nullptr; }
	inline Raid* getRaid() { return mRaid; }
	inline void setRaid(Raid* pRaid) { mRaid = pRaid; }

	void setConnection(ZoneClientConnection* pConnection) { mConnection = pConnection; }
	ZoneClientConnection* getConnection() { return mConnection; }
	const bool send(EQApplicationPacket* pPacket);
	
	void update();

	void setStanding(bool pStanding);
	inline const uint32 getStatus() const { return mStatus; }
	inline void setStatus(const uint32 pStatus) { mStatus = pStatus; }
	
	void startCamp();
	inline void setCampComplete(bool pCampComplete) { mCampComplete = pCampComplete; }
	inline bool getCampComplete() { return mCampComplete; }
	inline void setZoningOut() { mIsZoningOut = true; }
	inline bool isZoningOut() { return mIsZoningOut; }

	void message(const u32 pType, String pMessage);

	// Target Group Buff
	inline void setTGB(bool pTGB) { mTGB = pTGB; }
	inline const bool getTGB() const { return mTGB; }

	// Healing
	void healPercentage(int pPercent);

	//// Damage
	//void damage(uint32 pAmount);

	void doAnimation(uint8 pAnimationID);

	// Experience
	inline const uint32 getExperience() const { return mExperience; }
	inline void setExperience(const uint32 pExperience) { mExperience = pExperience; }
	const uint32 getExperienceRatio() const;
	void addExperience(uint32 pExperience);
	void removeExperience(uint32 pExperience);
	inline const uint32 getExperienceForNextLevel() const { return Character::getExperienceForLevel(getLevel() + 1); }
	
	void setCharacterLevel(uint8 pLevel);

	static const uint32 getExperienceForLevel(const uint8 pLevel);
	static inline const uint8 getMaxCharacterLevel() { return 60; }

	static float getDefaultSize(uint32 pRace);

	// Base Stats
	uint32 getBaseStatistic(Statistic pStatistic);
	void setBaseStatistic(Statistic pStatistic, uint32 pValue);

	// 
	uint32 getStrength() { return 0; };
	uint32 getStamina() { return 0; };
	uint32 getCharisma() { return 0; };
	uint32 getDexterity() { return 0; };
	uint32 getIntelligence() { return 0; };
	uint32 getAgility() { return 0; };
	uint32 getWisdom() { return 0; };
	uint32 getStatistic(Statistic pStatistic);

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
private:
	ZoneChange mZoneChange;
	BindLocation mBindLocations[5];

	const uint32 mAccountID;
	bool mInitialised = false;
	ClientAuthentication mAuthentication;
	bool mIsZoning = false;
	bool mIsLinkDead = false;
	Actor* mLootingCorpse = nullptr;
	Actor* mTradingWith = nullptr;
	NPC* mShoppingWith = nullptr;

	Filters mFilters;

	uint32 mExperience = 0;
	void _checkForLevelIncrease();

	uint32 mBaseStrength = 0;
	uint32 mBaseStamina = 0;
	uint32 mBaseCharisma = 0;
	uint32 mBaseDexterity = 0;
	uint32 mBaseIntelligence = 0;
	uint32 mBaseAgility = 0;
	uint32 mBaseWisdom = 0;

	bool mIsCasting = false;
	uint32 mCastingSpellID = 0;
	uint16 mCastingSlot = 0; // The Spell Bar slot ID used to cast a spell
	Timer mCastingTimer;

	std::array<uint32, Limits::Skills::MAX_ID> mSkills;
	std::array<uint32, Limits::Languages::MAX_ID> mLanguages;

	bool mHidden = false;
	bool mAutoAttacking = false;
	bool mStanding = true;
	bool mIsZoningOut = false;
	bool mCampComplete = false; // Flag indicating whether this character logged out via /camp
	bool mTGB = false;
	bool mAutoConsentGroup = false;
	bool mAutoConsentRaid = false;
	bool mAutoConsentGuild = false;
	uint8 mStatus = 0;
	Timer mCampTimer; // 30 seconds.

	Timer mSuperGMPower;
	Timer mAutoSave;

	Group* mGroup = nullptr;
	Raid* mRaid = nullptr;

	Guild* mGuild = nullptr;
	GuildID mPendingGuildInviteID = NO_GUILD;
	String mPendingGuildInviteName = "";

	ZoneClientConnection* mConnection = nullptr;
	Data::Character* mData = nullptr;
	

	std::list<NPC*> mVisibleNPCs; // NPCs that are visible to this Character

	struct QueuedChannelMessage {
		const ChannelID mChannelID;
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
};