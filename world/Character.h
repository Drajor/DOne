#pragma once

#include "Actor.h"
#include "Utility.h"
#include "ClientAuthentication.h"
#include "../common/timer.h"

class Zone;
class NPC;
class Group;
class Guild;
class Raid;
class EQStreamInterface;
class ZoneClientConnection;

struct CharacterData;
class SpellBook;

class Character : public Actor {
	friend ZoneClientConnection;
public:
	Character(const uint32 pAccountID, CharacterData* pCharacterData);
	~Character();
	inline const uint32 getAccountID() const { return mAccountID; }
	inline const bool isCharacter() const { return true; }
	inline const bool isInitialised() const { return mInitialised; }
	inline const bool isZoning() const { return mIsZoning; }
	inline void setZoning(const bool pZoning) { mIsZoning = pZoning; }
	inline const bool isLinkDead() const { return mIsLinkDead; }
	inline void setLinkDead() { mIsLinkDead = true; }


	ClientAuthentication getAuthentication() { return mAuthentication; }

	bool initialise();
	
	bool onZoneIn();
	bool onZoneOut();
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
	
	void update();

	void setStanding(bool pStanding);
	inline const uint32 getStatus() const { return mStatus; }
	inline void setStatus(const uint32 pStatus) { mStatus = pStatus; }
	
	void startCamp();
	inline void setCampComplete(bool pCampComplete) { mCampComplete = pCampComplete; }
	inline bool getCampComplete() { return mCampComplete; }
	inline void setZoningOut() { mIsZoningOut = true; }
	inline bool isZoningOut() { return mIsZoningOut; }

	void message(MessageType pType, String pMessage);

	// Target Group Buff
	inline void setTGB(bool pTGB) { mTGB = pTGB; }
	inline const bool getTGB() const { return mTGB; }

	// Healing
	void healPercentage(int pPercent);

	// Damage
	void damage(uint32 pAmount);

	int32 getCurrentHP() { return mCurrentHP; }
	int32 getMaximumHP() { return mMaximumHP; }
	int32 getCurrentMana() { return mCurrentMana; }
	int32 getMaximumMana() { return mMaximumMana; }
	int32 getCurrentEndurance() { return mCurrentEndurance; }
	int32 getMaximumEndurance() { return mMaximumEndurance; }

	void doAnimation(uint8 pAnimationID);

	// Experience
	inline const uint32 getExperience() const { return mExperience; }
	inline void setExperience(const uint32 pExperience) { mExperience = pExperience; }
	const uint32 getExperienceRatio() const;
	void addExperience(uint32 pExperience);
	void removeExperience(uint32 pExperience);
	inline const uint32 getExperienceForNextLevel() const { return Character::getExperienceForLevel(getLevel() + 1); }
	
	static const uint32 getExperienceForLevel(const uint8 pLevel);
	static inline const uint8 getMaxCharacterLevel() { return 20; }

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

	// Radiant Crystals.
	inline const uint32 getRadiantCrystals() const { return mRadiantCrystals; }
	inline const uint32 getTotalRadiantCrystals() const { return mTotalRadiantCrystals; }
	inline void addRadiantCrystals(const uint32 pCrystals) { mRadiantCrystals += pCrystals; mTotalRadiantCrystals += pCrystals; }
	inline void removeRadiantCrystals(const uint32 pCrystals) { mRadiantCrystals -= pCrystals; } // TODO: This needs to be guarded.

	// Ebon Crystals.
	inline const uint32 getEbonCrystals() const { return mEbonCrystals; }
	inline const uint32 getTotalEbonCrystals() const { return mTotalEbonCrystals; }
	inline void addEbonCrystals(const uint32 pCrystals) { mEbonCrystals += pCrystals; mTotalEbonCrystals += pCrystals; }
	inline void removeEbonCrystals(const uint32 pCrystals) { mEbonCrystals -= pCrystals; } // TODO: This needs to be guarded.

	inline void addVisibleNPC(NPC* pNPC) { mVisibleNPCs.push_back(pNPC); }
	inline void removeVisibleNPC(NPC* pNPC) { mVisibleNPCs.remove(pNPC); }
	inline std::list<NPC*>& getVisibleNPCs() { return mVisibleNPCs; }

	const CharacterData* getData() const { return mData; }

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
	const bool handleScribeSpell(const uint16 pSlot, const uint32 pSpellID);

	void notify(const String& pMessage);
private:

	const uint32 mAccountID;
	bool mInitialised = false;
	ClientAuthentication mAuthentication;
	bool mIsZoning = false;
	bool mIsLinkDead = false;

	uint32 mExperience = 0;
	void _checkForLevelIncrease();
	void _updateForSave();

	uint32 mRadiantCrystals = 0;
	uint32 mTotalRadiantCrystals = 0;
	inline void _setRadiantCrystals(const uint32 pCurrent, const uint32 pTotal) { mRadiantCrystals = pCurrent; mTotalRadiantCrystals = pTotal; }

	uint32 mEbonCrystals = 0;
	uint32 mTotalEbonCrystals = 0;
	inline void _setEbonCrystals(const uint32 pCurrent, const uint32 pTotal) { mEbonCrystals = pCurrent; mTotalEbonCrystals = pTotal; }

	int32 mCurrentHP = 100;
	int32 mMaximumHP = 100;
	int32 mCurrentMana = 100;
	int32 mMaximumMana = 100;
	int32 mCurrentEndurance = 100;
	int32 mMaximumEndurance = 100;

	uint32 mBaseStrength = 0;
	uint32 mBaseStamina = 0;
	uint32 mBaseCharisma = 0;
	uint32 mBaseDexterity = 0;
	uint32 mBaseIntelligence = 0;
	uint32 mBaseAgility = 0;
	uint32 mBaseWisdom = 0;

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
	CharacterData* mData = nullptr;
	

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
			for (auto i = 0; i < Limits::SpellBar::MAX_SLOTS; i++)
				mSpellIDs[i] = 0;
		}
		const std::vector<uint32>& getData() const { return mSpellIDs; }
		void setSpell(const uint16 pSlot, const uint32 pSpellID);
	private:
		std::vector<uint32> mSpellIDs;
	};
	SpellBar* mSpellBar = nullptr;
	const std::vector<uint32> getSpellBarData() const;
};