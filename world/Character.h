#pragma once

#include "Constants.h"
#include "ClientAuthentication.h"
#include "../common/timer.h"

class Zone;
class Group;
class Guild;
class Raid;
class EQStreamInterface;
class ZoneClientConnection;
struct PlayerProfile_Struct;
struct ExtendedProfile_Struct;

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


class Character {
	friend ZoneClientConnection;
public:
	Character(uint32 pCharacterID, ClientAuthentication& pAuthentication);
	~Character();
	
	ClientAuthentication getAuthentication() { return mAuthentication; }

	bool initialise(PlayerProfile_Struct* pProfile, ExtendedProfile_Struct* pExtendedProfile);
	bool isZoning() { return mIsZoning; }
	bool isLinkDead() { return mIsLinkDead; }
	void setLinkDead();
	bool onZoneIn();
	bool onZoneOut();
	void addQueuedMessage(ChannelID pChannel, const String& pSenderName, const String& pMessage);

	// Group
	bool hasGroup() { return mGroup != nullptr; }
	Group* getGroup() { return mGroup; }
	void setGroup(Group* pGroup) { mGroup = pGroup; }

	// Guild
	bool hasGuild() { return mGuild != nullptr; }
	Guild* getGuild() { return mGuild; }
	void setGuild(Guild* pGuild) { mGuild = pGuild; }
	void setGuildID(GuildID pGuildID);
	void setGuildRank(GuildRank pGuildRank);

	// Raid
	bool hasRaid() { return mRaid != nullptr; }
	Raid* getRaid() { return mRaid; }
	void setRaid(Raid* pRaid) { mRaid = pRaid; }

	void setZone(Zone* pZone) { mZone = pZone; }
	void setSpawnID(uint16 pSpawnID) { mSpawnID = pSpawnID; }

	Zone* getZone() { return mZone; }
	void setConnection(ZoneClientConnection* pConnection) { mConnection = pConnection; }
	ZoneClientConnection* getConnection() { return mConnection; }
	
	void update();

	void setStanding(bool pStanding);

	String getName() { return mName; }
	String getLastName() { return mLastName; }
	String getTitle() { return mTitle; }
	String getSuffix() { return mSuffix; }
	uint32 getID() { return mCharacterID; };
	SpawnID getSpawnID() { return mSpawnID; }
	// Returns the account status that this Character belongs to.
	uint32 getStatus() { return mStatus; }
	PlayerProfile_Struct* getProfile() { return mProfile; }
	ExtendedProfile_Struct* getExtendedProfile() { return mExtendedProfile; }
	
	void startCamp();
	void setCampComplete(bool pCampComplete) { mCampComplete = pCampComplete; }
	bool getCampComplete() { return mCampComplete; }
	void setZoningOut() { mIsZoningOut = true; }
	bool isZoningOut() { return mIsZoningOut; }

	void setAFK(bool pAFK);
	bool getAFK();
	void setShowHelm(bool pShowHelm);
	bool getShowHelm();

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
	void setAnonymous(uint8 pAnonymous);
	uint8 getAnonymous();
	uint8 getGM();
	void setGM(bool pGM);
	uint8 getGuildRank() { return mGuildRank; }
	uint32 getGuildID() { return mGuildID; }

	int32 getCopper() { return mCopper; }
	int32 getSilver() { return mSilver; }
	int32 getGold() { return mGold; }
	int32 getPlatinum() { return mPlatinum; }

	// Target Group Buff
	void setTGB(bool pTGB) { mTGB = pTGB; }
	bool getTGB() { return mTGB; }

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
private:

	void _initialiseProfile();

	void _setAppearance(SpawnAppearanceAnimation pAppearance) { mAppearance = pAppearance; }

	ClientAuthentication mAuthentication;
	bool mIsZoning;
	bool mIsLinkDead;
	float mX;
	float mY;
	float mZ;
	float mHeading;
	int32 mDeltaX;
	int32 mDeltaY;
	int32 mDeltaZ;
	int32 mDeltaHeading;
	int32 mAnimation;
	SpawnAppearanceAnimation mAppearance;
	void _updateProfilePosition();
	

	uint32 mExperience;
	void _checkForLevelIncrease();
	void _updateForSave();
	

	
	float mSize;
	uint16 mDeity;
	uint8 mLevel;
	uint8 mClass;
	uint32 mRace;
	uint32 mOriginalRace; // Race that was loaded from Profile.
	uint8 mGender;
	float mRunSpeed;
	float mWalkSpeed;

	uint8 mGuildRank;
	uint32 mGuildID;

	int32 mCurrentHP;
	int32 mMaximumHP;
	int32 mCurrentMana;
	int32 mMaximumMana;
	int32 mCurrentEndurance;
	int32 mMaximumEndurance;

	int32 mCopper;
	int32 mSilver;
	int32 mGold;
	int32 mPlatinum;

	const uint32 mCharacterID;
	SpawnID mSpawnID;
	String mName;
	String mLastName;
	String mTitle;
	String mSuffix;
	bool mGM;
	bool mStanding;
	bool mAFK;
	bool mIsZoningOut;
	bool mCampComplete; // Flag indicating whether this character logged out via /camp
	bool mTGB;
	uint32 mStatus;
	Timer mCampTimer; // 30 seconds.

	Timer mSuperGMPower;
	Timer mAutoSave;

	Group* mGroup;
	Guild* mGuild;
	Raid* mRaid;

	Zone* mZone;
	ZoneClientConnection* mConnection;
	PlayerProfile_Struct* mProfile;
	ExtendedProfile_Struct* mExtendedProfile;

	struct QueuedChannelMessage {
		const ChannelID mChannelID;
		const String mSenderName;
		const String mMessage;
	};
	std::list<QueuedChannelMessage> mMessageQueue;
	void _processMessageQueue();
};