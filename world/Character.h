#pragma once

#include "Constants.h"
#include "../common/types.h"
#include "../common/timer.h"
#include <string>
#include <unordered_map>

class Zone;
class Group;
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
	Character(uint32 pCharacterID);
	~Character();

	bool initialise(PlayerProfile_Struct* pProfile, ExtendedProfile_Struct* pExtendedProfile);
	bool onZoneIn();
	bool onZoneOut();

	void addQueuedTell(std::string pSenderName, std::string pMessage);
	bool hasQueuedTells() { return !mQueuedTells.empty(); }

	// Group
	bool hasGroup() { return mGroup != nullptr; }
	Group* getGroup() { return mGroup; }
	void setGroup(Group* pGroup) { mGroup = pGroup; }

	void setZone(Zone* pZone) { mZone = pZone; }
	void setSpawnID(uint16 pSpawnID) { mSpawnID = pSpawnID; }

	Zone* getZone() { return mZone; }
	void setConnection(ZoneClientConnection* pConnection) { mConnection = pConnection; }
	ZoneClientConnection* getConnection() { return mConnection; }
	
	void update();

	void setStanding(bool pStanding);

	std::string getName() { return mName; }
	std::string getLastName() { return mLastName; }
	std::string getTitle() { return mTitle; }
	std::string getSuffix() { return mSuffix; }
	uint32 getID() { return mCharacterID; };
	uint16 getSpawnID() { return mSpawnID; }
	// Returns the account status that this Character belongs to.
	uint32 getStatus() { return mStatus; }
	PlayerProfile_Struct* getProfile() { return mProfile; }
	ExtendedProfile_Struct* getExtendedProfile() { return mExtendedProfile; }
	void startCamp();
	void setLoggedOut(bool pLoggedOut) { mLoggedOut = pLoggedOut; }
	bool getLoggedOut() { return mLoggedOut; }
	void setAFK(bool pAFK);
	bool getAFK();
	void setShowHelm(bool pShowHelm);
	bool getShowHelm();

	void message(MessageType pType, std::string pMessage);
	


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
private:

	void _initialiseProfile();

	void _setAppearance(SpawnAppearanceAnimation pAppearance) { mAppearance = pAppearance; }

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
	uint16 mSpawnID;
	std::string mName;
	std::string mLastName;
	std::string mTitle;
	std::string mSuffix;
	bool mGM;
	bool mStanding;
	bool mAFK;
	bool mLoggedOut; // Flag indicating whether this character logged out via /camp
	bool mTGB;
	uint32 mStatus;
	Timer mCampTimer; // 30 seconds.

	Timer mSuperGMPower;
	Timer mAutoSave;

	std::unordered_multimap<std::string, std::string> mQueuedTells;
	void _processQueuedTells();
	Group* mGroup;

	Zone* mZone;
	ZoneClientConnection* mConnection;
	PlayerProfile_Struct* mProfile;
	ExtendedProfile_Struct* mExtendedProfile;
};