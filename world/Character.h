#pragma once

#include "Actor.h"
#include "Utility.h"
#include "ClientAuthentication.h"
#include "../common/timer.h"

class Zone;
class Group;
class Guild;
class Raid;
class EQStreamInterface;
class ZoneClientConnection;

struct CharacterData;

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

	void setZone(Zone* pZone) { mZone = pZone; }

	Zone* getZone() const { return mZone; }
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

	inline const int32 getCopper() const { return mCopper; }
	inline const int32 getSilver() const { return mSilver; }
	inline const int32 getGold() const { return mGold; }
	inline const int32 getPlatinum() const { return mPlatinum; }

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
	uint32 getExperience() { return mExperience; }
	uint32 getExperienceRatio();
	void addExperience(uint32 pExperience);
	void removeExperience(uint32 pExperience);
	uint32 getExperienceForNextLevel() { return Character::getExperienceForLevel(getLevel() + 1); }
	static uint32 getExperienceForLevel(uint8 pLevel);
	static uint8 getMaxCharacterLevel() { return 20; }

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

	inline const bool getAutoConsentGroup() const { return mAutoConsentGroup; }
	inline const bool getAutoConsentRaid() const { return mAutoConsentRaid; }
	inline const bool getAutoConsentGuild() const { return mAutoConsentGuild; }

	inline const float getVisibleRange() const { return mVisibleRange; }
	inline void setVisibleRange(const float pVisibleRange) { mVisibleRange = pVisibleRange; /* Notify scene? */ }
	inline std::list<Character*>& getVisibleTo() { return mVisibleTo; }

	void addVisibleTo(Character* pCharacter);
	void removeVisibleTo(Character* pCharacter);

	const CharacterData* getData() const { return mData; }
private:

	const uint32 mAccountID;
	bool mInitialised = false;
	ClientAuthentication mAuthentication;
	bool mIsZoning = false;
	bool mIsLinkDead = false;

	uint32 mExperience = 0;
	void _checkForLevelIncrease();
	void _updateForSave();

	int32 mCurrentHP = 100;
	int32 mMaximumHP = 100;
	int32 mCurrentMana = 100;
	int32 mMaximumMana = 100;
	int32 mCurrentEndurance = 100;
	int32 mMaximumEndurance = 100;

	int32 mCopper = 0;
	int32 mSilver = 0;
	int32 mGold = 0;
	int32 mPlatinum = 0;

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

	Zone* mZone = nullptr;
	ZoneClientConnection* mConnection = nullptr;
	CharacterData* mData = nullptr;

	struct QueuedChannelMessage {
		const ChannelID mChannelID;
		const String mSenderName;
		const String mMessage;
	};
	std::list<QueuedChannelMessage> mMessageQueue;
	void _processMessageQueue();
	
	float mVisibleRange = 30.0f;
	std::list<Character*> mVisibleTo; // List of Characters who can see this Character.
};