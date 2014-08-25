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

	Zone* getZone() const { return mZone; }
	void setConnection(ZoneClientConnection* pConnection) { mConnection = pConnection; }
	ZoneClientConnection* getConnection() { return mConnection; }
	
	void update();

	void setStanding(bool pStanding);

	inline const String& getName() const { return mName; }
	inline const String& getLastName() const { return mLastName; }
	inline const String& getTitle() const { return mTitle; }
	inline const String& getSuffix() const { return mSuffix; }
	inline const uint32 getStatus() const { return mStatus; }
	
	void startCamp();
	inline void setCampComplete(bool pCampComplete) { mCampComplete = pCampComplete; }
	inline bool getCampComplete() { return mCampComplete; }
	inline void setZoningOut() { mIsZoningOut = true; }
	inline bool isZoningOut() { return mIsZoningOut; }

	inline void setAFK(bool pAFK) { mAFK = pAFK; };
	inline const bool isAFK() const { return mAFK; };
	inline void setLFG(bool pLFG) { mLFG = pLFG; }
	inline const bool isLFG() const { return mLFG; }
	inline void setShowHelm(bool pShowHelm) { mShowHelm = pShowHelm; };
	inline const bool getShowHelm() { return mShowHelm; };

	void message(MessageType pType, String pMessage);
	


	// Position and Heading
	void setPositionDeltas(float pDeltaX, float pDeltaY, float pDeltaZ, int32 pDeltaHeading);
	Vector3 getPosition3() { return mPosition; };
	float getX() { return mPosition.x; }
	float getY() { return mPosition.y; }
	float getZ() { return mPosition.z; }
	int32 getDeltaX() { return mDeltaX; }
	int32 getDeltaY() { return mDeltaY; }
	int32 getDeltaZ() { return mDeltaZ; }

	void setPosition(float pX, float pY, float pZ, float pHeading);
	void setPosition(Vector3& pPosition);

	const float getHeading() const { return mHeading; }
	const int32 getDeltaHeading() const { return mDeltaHeading; }
	void setHeading(float pHeading);
	const uint32 getRace() const { return mRace; }
	const uint32 getOriginalRace() const { return mOriginalRace; }
	const float getRunSpeed() const { return mRunSpeed; }
	const float getWalkSpeed() const { return mWalkSpeed; }
	const uint8 getClass() const { return mClass; }
	const uint8 getGender() const { return mGender; }
	const uint8 getLevel() const { return mLevel; }
	const uint16 getDeity() const { return mDeity; }
	const float getSize() const { return mSize; }
	const int32 getAnimation() const { return mAnimation; }
	const SpawnAppearanceAnimation getAppearance() const { return mAppearance; }
	void setAnimation(const int32 pAnimation) { mAnimation = pAnimation; }
	inline void setAnonymous(const AnonType pAnonymous) { mAnonymous = pAnonymous; }
	inline const AnonType getAnonymous() const { return mAnonymous; };
	inline const bool isGM() const { return mGM; };
	inline void setGM(const bool pGM) { mGM = pGM; }

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

	inline const bool getAutoConsentGroup() const { return mAutoConsentGroup; }
	inline const bool getAutoConsentRaid() const { return mAutoConsentRaid; }
	inline const bool getAutoConsentGuild() const { return mAutoConsentGuild; }

	inline const uint8 getLeftEyeColour() const { return mLeftEyeColour; }
	inline const uint8 getRightEyeColour() const { return mRightEyeColour; }
	inline const uint8 getBeardStyle() const { return mBeardStyle; }
	inline const uint8 getBeardColour() const { return mBeardColour; }
	inline const uint8 getHairStyle() const { return mHairStyle; }
	inline const uint8 getHairColour() const { return mHairColour; }
	inline const uint8 getFaceStyle() const { return mFaceStyle; }
	inline const uint32 getDrakkinHeritage() const { return mDrakkinHeritage; }
	inline const uint32 getDrakkinTattoo() const { return mDrakkinTattoo; }
	inline const uint32 getDrakkinDetails() const { return mDrakkinDetails; }

	inline void setLeftEyeColour(const uint8 pLeftEyeColour) { mLeftEyeColour = pLeftEyeColour; }
	inline void setRightEyeColour(const uint8 pRightEyeColour) { mRightEyeColour = pRightEyeColour; }
	inline void setBeardStyle(const uint8 pBeardStyle) { mBeardStyle = pBeardStyle; }
	inline void setBeardColour(const uint8 pBeardColour) { mBeardColour = pBeardColour; }
	inline void setHairStyle(const uint8 pHairStyle) { mHairStyle = pHairStyle; }
	inline void setHairColour(const uint8 pHairColour) { mHairColour = pHairColour; }
	inline void setFaceStyle(const uint8 pFaceStyle) { mFaceStyle = pFaceStyle; }
	inline void setDrakkinHeritage(const uint32 pDrakkinHeritage) { mDrakkinHeritage = pDrakkinHeritage; }
	inline void setDrakkinTattoo(const uint32 pDrakkinTattoo) { mDrakkinTattoo = pDrakkinTattoo; }
	inline void setDrakkinDetails(const uint32 pDrakkinDetails) { mDrakkinDetails = pDrakkinDetails; }

	inline const float getVisibleRange() const { return mVisibleRange; }
	inline void setVisibleRange(const float pVisibleRange) { mVisibleRange = pVisibleRange; /* Notify scene? */ }
	//inline std::list<Character*>& getVisible() { return mVisible; }
	inline std::list<Character*>& getVisibleTo() { return mVisibleTo; }

	inline void addVisibleTo(Character* pCharacter) { EXPECTED(pCharacter); EXPECTED(pCharacter != this); mVisibleTo.push_back(pCharacter); }
	inline void removeVisibleTo(Character* pCharacter) { EXPECTED(pCharacter); EXPECTED(mVisibleTo.empty() == false); std::size_t oSize = mVisibleTo.size(); mVisibleTo.remove(pCharacter); EXPECTED(mVisibleTo.size() == oSize - 1); }

	const CharacterData* getData() const { return mData; }
private:

	void _initialiseProfile();

	void _setAppearance(SpawnAppearanceAnimation pAppearance) { mAppearance = pAppearance; }

	const uint32 mAccountID;
	bool mInitialised = false;
	ClientAuthentication mAuthentication;
	bool mIsZoning = false;
	bool mIsLinkDead = false;
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
	AnonType mAnonymous = AnonType::AT_None;

	uint8 mRightEyeColour = 0;
	uint8 mLeftEyeColour = 0;
	uint8 mFaceStyle = 0;
	uint8 mBeardStyle = 0;
	uint8 mBeardColour = 0;
	uint8 mHairStyle = 0;
	uint8 mHairColour = 0;
	uint32 mDrakkinHeritage = 0;
	uint32 mDrakkinTattoo = 0;
	uint32 mDrakkinDetails = 0;

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
	bool mLFG = false;
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
	
	float mVisibleRange = 30.0f;
	//std::list<Character*> mVisible; // List of Characters who this Character can see.
	std::list<Character*> mVisibleTo; // List of Characters who can see this Character.
};