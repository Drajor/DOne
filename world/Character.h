#pragma once

#include "Actor.h"
#include "ClientAuthentication.h"
#include "../common/timer.h"

class Zone;
class Group;
class Guild;
class Raid;
class EQStreamInterface;
class ZoneClientConnection;

struct CharacterData;

struct Vector3 {
	Vector3() : x(0.0f), y(0.0f), z(0.0f) {};
	Vector3(float pX, float pY, float pZ) : x(pX), y(pY), z(pZ) {};
	float x;
	float y;
	float z;
};
struct Vector4 {
	Vector4() : x(0.0f), y(0.0f), z(0.0f), h(0.0f) {};
	Vector4(float pX, float pY, float pZ, float pH) : x(pX), y(pY), z(pZ), h(pH) {};
	float x;
	float y;
	float z;
	float h;
};


class Character : public Actor {
	friend ZoneClientConnection;
public:
	Character(CharacterData* pCharacterData);
	~Character();
	inline bool isCharacter() { return true; }
	inline bool isInitialised() { return mInitialised; }
	inline bool isZoning() { return mIsZoning; }
	inline void setZoning(bool pZoning) { mIsZoning = pZoning; }
	inline bool isLinkDead() { return mIsLinkDead; }
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
	inline void setGuildID(GuildID pGuildID) { mGuildID = pGuildID; };
	inline void setGuildRank(GuildRank pGuildRank) { mGuildRank = pGuildRank; };
	inline const GuildRank getGuildRank() { return mGuildRank; };
	inline const GuildID getGuildID() { return mGuildID; };
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

	Zone* getZone() { return mZone; }
	void setConnection(ZoneClientConnection* pConnection) { mConnection = pConnection; }
	ZoneClientConnection* getConnection() { return mConnection; }
	
	void update();

	void setStanding(bool pStanding);

	inline const String& getName() { return mName; }
	inline const String& getLastName() { return mLastName; }
	inline const String& getTitle() { return mTitle; }
	inline const String& getSuffix() { return mSuffix; }
	inline const uint32 getStatus() { return mStatus; }
	
	void startCamp();
	inline void setCampComplete(bool pCampComplete) { mCampComplete = pCampComplete; }
	inline bool getCampComplete() { return mCampComplete; }
	inline void setZoningOut() { mIsZoningOut = true; }
	inline bool isZoningOut() { return mIsZoningOut; }

	inline void setAFK(bool pAFK) { mAFK = pAFK; };
	inline bool getAFK() { return mAFK; };
	inline void setShowHelm(bool pShowHelm) { mShowHelm = pShowHelm; };
	inline bool getShowHelm() { return mShowHelm; };

	void message(MessageType pType, String pMessage);
	


	// Position and Heading
	void setPositionDeltas(float pDeltaX, float pDeltaY, float pDeltaZ, int32 pDeltaHeading);
	Vector3 getPosition3() { return Vector3(mX, mY, mZ); };
	Vector4 getPosition4() { return Vector4(mX, mY, mZ, mHeading); };
	float getX() { return mX; }
	float getY() { return mY; }
	float getZ() { return mZ; }
	int32 getDeltaX() { return mDeltaX; }
	int32 getDeltaY() { return mDeltaY; }
	int32 getDeltaZ() { return mDeltaZ; }

	void setPosition(float pX, float pY, float pZ, float pHeading);
	void setPosition(Vector3& pPosition);
	void setPosition(Vector4& pPosition);

	float getHeading() { return mHeading; }
	int32 getDeltaHeading() { return mDeltaHeading; }
	void setHeading(float pHeading);
	uint32 getRace() { return mRace; }
	uint32 getOriginalRace() { return mOriginalRace; }
	float getRunSpeed() { return mRunSpeed; }
	float getWalkSpeed() { return mWalkSpeed; }
	uint8 getClass() { return mClass; }
	uint8 getGender() { return mGender; }
	uint8 getLevel() { return mLevel; }
	uint16 getDeity() { return mDeity; }
	float getSize() { return mSize; }
	int32 getAnimation() { return mAnimation; }
	SpawnAppearanceAnimation getAppearance() { return mAppearance; }
	void setAnimation(int32 pAnimation) { mAnimation = pAnimation; }
	inline void setAnonymous(uint8 pAnonymous) { mAnonymous = pAnonymous; }
	inline uint8 getAnonymous() { return mAnonymous; };
	uint8 getGM();
	void setGM(bool pGM);

	inline const int32 getCopper() { return mCopper; }
	inline const int32 getSilver() { return mSilver; }
	inline const int32 getGold() { return mGold; }
	inline const int32 getPlatinum() { return mPlatinum; }

	// Target Group Buff
	inline void setTGB(bool pTGB) { mTGB = pTGB; }
	inline bool getTGB() { return mTGB; }

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
	uint32 getExperienceForNextLevel() { return Character::getExperienceForLevel(mLevel + 1); }
	static uint32 getExperienceForLevel(uint8 pLevel);
	static uint8 getMaxCharacterLevel() { return 20; }
	void setLevel(uint8 pLevel);

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

	inline const bool getAutoConsentGroup() { return mAutoConsentGroup; }
	inline const bool getAutoConsentRaid() { return mAutoConsentRaid; }
	inline const bool getAutoConsentGuild() { return mAutoConsentGuild; }

	uint8 getLeftEyeColour() { return mLeftEyeColour; }
	uint8 getRightEyeColour() { return mRightEyeColour; }
	uint8 getBeardStyle() { return mBeardStyle; }
	uint8 getBeardColour() { return mBeardColour; }
	uint8 getHairStyle() { return mHairStyle; }
	uint8 getHairColour() { return mHairColour; }
private:

	void _initialiseProfile();

	void _setAppearance(SpawnAppearanceAnimation pAppearance) { mAppearance = pAppearance; }

	bool mInitialised = false;
	ClientAuthentication mAuthentication;
	bool mIsZoning = false;
	bool mIsLinkDead = false;
	float mX = 0.0f;
	float mY = 0.0f;
	float mZ = 0.0f;
	float mHeading = 0.0f;
	int32 mDeltaX = 0;
	int32 mDeltaY = 0;
	int32 mDeltaZ = 0;
	int32 mDeltaHeading = 0;
	int32 mAnimation = 0;
	SpawnAppearanceAnimation mAppearance = Standing;
	

	uint32 mExperience;
	void _checkForLevelIncrease();
	void _updateForSave();
	
	float mSize = 5.0f;
	uint16 mDeity = 394;
	uint8 mLevel = 1;
	ClassID mClass = ClassIDs::Warrior;
	uint32 mRace = 1;
	uint32 mOriginalRace = 1;
	GenderID mGender = 1;
	float mRunSpeed = 0.7f;
	float mWalkSpeed = 0.35f;
	uint8 mAnonymous = 0;

	uint8 mRightEyeColour = 0;
	uint8 mLeftEyeColour = 0;

	uint8 mBeardStyle = 0;
	uint8 mBeardColour = 0;
	uint8 mHairStyle = 0;
	uint8 mHairColour = 0;

	GuildRank mGuildRank = GR_None;
	GuildID mGuildID = NO_GUILD;

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

	SpawnID mSpawnID = 0;
	String mName = "";
	String mLastName = "";
	String mTitle = "";
	String mSuffix = "";
	bool mGM = false;
	bool mStanding = true;
	bool mAFK = false;
	bool mIsZoningOut = false;
	bool mCampComplete = false; // Flag indicating whether this character logged out via /camp
	bool mTGB = false;
	bool mShowHelm = false;
	bool mAutoConsentGroup = false;
	bool mAutoConsentRaid = false;
	bool mAutoConsentGuild = false;
	uint32 mStatus = 0;
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
	

};