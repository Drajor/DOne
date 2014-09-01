#pragma once

#include "Constants.h"
#include "Payload.h"
#include "Vector3.h"

namespace FUCK {
	static inline const float xEQ19toFloat(const int d)
	{
		return (float(d) / float(1 << 3));
	}

	static inline const int xFloatToEQ19(float d)
	{
		return int(d*float(1 << 3));
	}

	static inline const float xEQ13toFloat(int d)
	{
		return (float(d) / float(1 << 2));
	}

	static inline const float xNewEQ13toFloat(int d)
	{
		return (float(d) / float(1 << 6));
	}

	static inline const int xNewFloatToEQ13(float d)
	{
		return int(d*float(1 << 6));
	}
}

class Character;
class Zone;
class Actor {
public:
	Actor();;
	virtual ~Actor() {};

	void setZone(Zone* pZone) { mZone = pZone; }
	Zone* getZone() const { return mZone; }

	inline virtual const bool isCharacter() const { return false; }
	inline virtual const bool isCharacterCorpse() const { return false; }
	inline virtual const bool isNPC() const { return false; }
	inline virtual const bool isNPCCorpse() const { return false; }
	inline const bool isCorpse() const { return (isCharacterCorpse() || isNPCCorpse()); }
	inline void setTarget(Actor* pActor) { mTarget = pActor; }
	inline Actor* getTarget() const { return mTarget; }
	inline const bool hasTarget() const { return mTarget != nullptr; }
	template <typename T>
	inline static T cast(Actor* pActor) {
		return static_cast<T>(pActor);
	}
	inline const float distanceTo(const Actor* pActor) {
		return mPosition.distance(pActor->mPosition);
	}
	inline const float squareDistanceTo(const Actor* pActor) {
		return mPosition.squareDistance(pActor->mPosition);
	}

	inline const unsigned char* getActorData() { return reinterpret_cast<unsigned char*>(&mSpawnData); }
	inline const unsigned char* getPositionData() { return reinterpret_cast<unsigned char*>(&mSpawnData.mSpawnID2); }

	inline const float getVisibleRange() const { return mVisibleRange; }
	inline void setVisibleRange(const float pVisibleRange) { mVisibleRange = pVisibleRange; /* Notify scene? */ }
	inline std::list<Character*>& getVisibleTo() { return mVisibleTo; }

	inline void addVisibleTo(Character* pCharacter) { mVisibleTo.push_back(pCharacter); }
	inline void removeVisibleTo(Character* pCharacter) { mVisibleTo.remove(pCharacter); }

	inline const SpawnID getSpawnID() const { return mSpawnData.mSpawnID; }
	inline void setSpawnID(const SpawnID pSpawnID) { mSpawnData.mSpawnID = pSpawnID; mSpawnData.mSpawnID2 = pSpawnID; }

	inline const String& getName() const { return mName; }
	inline void setName(const String& pName) { mName = pName; _setName(pName.c_str()); }
	inline const String& getLastName() const { return mLastName; }
	inline void setLastName(const String& pLastName) { mLastName = pLastName; _setLastName(pLastName.c_str()); }
	inline const String& getTitle() const { return mTitle; }
	inline void setTitle(const String& pTitle) { mTitle = pTitle; _setTitle(pTitle.c_str()); }
	inline const String& getSuffix() const { return mSuffix; }
	inline void setSuffix(const String& pSuffix) { mSuffix = pSuffix; _setSuffix(pSuffix.c_str()); }

	inline const int32 getCopper() const { return mCopper; }
	inline void setCopper(const int32 pCopper) { mCopper = pCopper; }
	inline const int32 getSilver() const { return mSilver; }
	inline void setSilver(const int32 pSilver) { mSilver = pSilver; }
	inline const int32 getGold() const { return mGold; }
	inline void setGold(const int32 pGold) { mGold = pGold; }
	inline const int32 getPlatinum() const { return mPlatinum; }
	inline void setPlatinum(const int32 pPlatinum) { mPlatinum = pPlatinum; }

	inline void _syncPosition() {
		// Current
		mSpawnData.x = FUCK::xFloatToEQ19(mPosition.x);
		mSpawnData.y = FUCK::xFloatToEQ19(mPosition.y);
		mSpawnData.z = FUCK::xFloatToEQ19(mPosition.z);
		mSpawnData.heading = FUCK::xFloatToEQ19(mHeading);
		// Delta
		mSpawnData.deltaX = FUCK::xNewFloatToEQ13(mPositionDelta.x);
		mSpawnData.deltaY = FUCK::xNewFloatToEQ13(mPositionDelta.y);
		mSpawnData.deltaZ = FUCK::xNewFloatToEQ13(mPositionDelta.z);
		mSpawnData.deltaHeading = FUCK::xNewFloatToEQ13(mHeadingDelta);
	}

	inline const Vector3& getPosition() const { return mPosition; }
	inline void setPosition(const Vector3& pPosition) { mPosition = pPosition; }
	inline const Vector3& getPositionDelta() const { return mPositionDelta; }
	inline void setPositionDelta(const Vector3& pPositionDelta) { mPositionDelta = pPositionDelta; }
	inline const float getX() const { return mPosition.x; }
	inline const float getY() const { return mPosition.y; }
	inline const float getZ() const { return mPosition.z; }
	inline const float getXDelta() const { return mPositionDelta.x; }
	inline const float getYDelta() const { return mPositionDelta.y; }
	inline const float getZDelta() const { return mPositionDelta.z; }
	inline const float getHeading() const { return mHeading; }
	inline void setHeading(const float pHeading) { mHeading = pHeading; }
	inline const float getHeadingDelta() const { return mHeadingDelta; }
	inline void setHeadingDelta(const float pHeadingDelta) { mHeadingDelta = pHeadingDelta; }

	inline const int32 getAnimation() const { return mAnimation; }
	inline void setAnimation(const int32 pAnimation) { mAnimation = pAnimation; }

	inline const uint8 getStandingState() const { return mSpawnData.mStandState; }
	inline void setStandingState(const uint8 pStandingState) { mSpawnData.mStandState = pStandingState; }

	// Visual
	inline const uint8 getFaceStyle() const { return mSpawnData.mFaceStyle; }
	inline const uint8 getLeftEyeColour() const { return mSpawnData.mLeftEyeColour; }
	inline const uint8 getRightEyeColour() const { return mSpawnData.mRightEyeColour; }
	inline const uint8 getHairStyle() const { return mSpawnData.mHairStyle; }
	inline const uint8 getBeardStyle() const { return mSpawnData.mBeardStyle; }
	inline const uint8 getHairColour() const { return mSpawnData.mHairColor; }
	inline const uint8 getBeardColour() const { return mSpawnData.mBeardColour; }
	inline const uint32 getDrakkinHeritage() const { return mSpawnData.mDrakkinHeritage; }
	inline const uint32 getDrakkinTattoo() const { return mSpawnData.mDrakkinTattoo; }
	inline const uint32 getDrakkinDetails() const { return mSpawnData.mDrakkinDetails; }

	inline void setFaceStyle(const uint8 pFaceStyle) { mSpawnData.mFaceStyle = pFaceStyle; }
	inline void setLeftEyeColour(const uint8 pLeftEyeColour) { mSpawnData.mLeftEyeColour = pLeftEyeColour; }
	inline void setRightEyeColour(const uint8 pRightEyeColour) { mSpawnData.mRightEyeColour = pRightEyeColour; }
	inline void setHairStyle(const uint8 pHairStyle) { mSpawnData.mHairStyle = pHairStyle; }
	inline void setBeardStyle(const uint8 pBeardStyle) { mSpawnData.mBeardStyle = pBeardStyle; }
	inline void setHairColour(const uint8 pHairColour) { mSpawnData.mHairColor = pHairColour; }
	inline void setBeardColour(const uint8 pBeardColour) { mSpawnData.mBeardColour = pBeardColour; }
	inline void setDrakkinHeritage(const uint32 pDrakkinHeritage) { mSpawnData.mDrakkinHeritage = pDrakkinHeritage; }
	inline void setDrakkinTattoo(const uint32 pDrakkinTattoo) { mSpawnData.mDrakkinTattoo = pDrakkinTattoo; }
	inline void setDrakkinDetails(const uint32 pDrakkinDetails) { mSpawnData.mDrakkinDetails = pDrakkinDetails; }

	// Other
	inline const bool getIsGM() const { return mSpawnData.mGM == 1; }
	inline void setIsGM(const bool pIsGM) { mSpawnData.mGM = pIsGM ? GM_ON : GM_OFF; }

	inline const GenderID getGender() const { return mSpawnData.mGender; }
	inline void setGender(const GenderID pGender) { mSpawnData.mGender = pGender; }
	
	inline const ClassID getClass() const { return mSpawnData.mClass; }
	inline void setClass(const ClassID pClassID) { mSpawnData.mClass = pClassID; }
	
	inline const AATitle getAATitle() const { return mSpawnData.mAATitle; }
	inline void setAATitle(const AATitle pAATitle) { mSpawnData.mAATitle = pAATitle; }
	
	inline const AnonType getAnonymous() const { return mSpawnData.mAnonymous; }
	inline void setAnonymous(const AnonType pAnonType) { mSpawnData.mAnonymous = pAnonType; }
	
	inline const DeityID getDeityID() const { return mSpawnData.mDeity; }
	inline void setDeityID(const DeityID pDeityID) { mSpawnData.mDeity = pDeityID; }

	inline const float getSize() const { return mSpawnData.mSize; }
	inline void setSize(const float pSize) { mSpawnData.mSize = pSize; }

	inline const ActorType getActorType() const { return mSpawnData.mActorType; }
	inline void setActorType(const ActorType pActorType) { mSpawnData.mActorType = pActorType; }

	inline const bool isInvisible() const { return mSpawnData.mIsInvisible == 1; }
	inline void setInvisible(const bool pInvisible) { mSpawnData.mIsInvisible = pInvisible ? 1 : 0; }

	// TODO: Is this a ratio?
	inline const uint8 getCurrentHP() const { return mSpawnData.mCurrentHP; }
	inline void setCurrentHP(const uint8 pCurrentHP) { mSpawnData.mCurrentHP = pCurrentHP; }
	// max_hp TODO

	inline const bool isFindable() const { return mSpawnData.mIsFindable == 1; }
	inline void setIsFindable(const bool pFindable) { mSpawnData.mIsFindable = pFindable ? 1 : 0; }

	inline const uint8 getStandState() const { return mSpawnData.mStandState; }
	inline void setStandState(const uint8 pStandState) { mSpawnData.mStandState = pStandState; }

	inline const bool getShowHelm() const { return mSpawnData.mShowHelm == 1; }
	inline void setShowHelm(const bool pShowHelm) { mSpawnData.mShowHelm = pShowHelm ? 1 : 0; }

	inline const bool getIsNPC() const { return mSpawnData.mIsNPC == 1; }
	inline void setIsNPC(const bool pIsNPC) { mSpawnData.mIsNPC = pIsNPC ? 1 : 0; }

	inline const uint8 getLevel() const { return mSpawnData.mLevel; }
	inline void setLevel(const uint8 pLevel) { mSpawnData.mLevel = pLevel; }

	inline const SpawnID getOwnerSpawnID() const { return mSpawnData.mOwnerSpawnID; }
	inline void setOwnerSpawnID(const SpawnID pSpawnID) { mSpawnData.mOwnerSpawnID = pSpawnID; }

	inline const GuildRank getGuildRank() const { return mSpawnData.mGuildRank; }
	inline void setGuildRank(const GuildRank pGuildRank) { mSpawnData.mGuildRank = pGuildRank; }

	inline const uint32 getMaterial(const MaterialSlot pSlotID) const {
		// TODO: Guard this.
		return mSpawnData.mEquipmentMaterials[pSlotID];
	}
	inline const uint32 getMaterial(const int pSlotID) const {
		// TODO: Guard this.
		return mSpawnData.mEquipmentMaterials[pSlotID];
	}
	inline void setMaterial(const MaterialSlot pSlotID, const uint32 pMaterial) {
		// TODO: Guard this.
		mSpawnData.mEquipmentMaterials[pSlotID] = pMaterial;
	}

	inline const float getRunSpeed() const { return mSpawnData.mRunSpeed; }
	inline void setRunSpeed(const float pRunSpeed) { mSpawnData.mRunSpeed = pRunSpeed; }

	inline const bool isAFK() const { return mSpawnData.mIsAFK == 1; }
	inline void setIsAFK(const bool pAFK) { mSpawnData.mIsAFK = pAFK ? 1 : 0; }

	inline const GuildID getGuildID() const { return mSpawnData.mGuildID; }
	inline void setGuildID(const GuildID pGuildID) { mSpawnData.mGuildID = pGuildID; }

	inline const RaceID getRaceID() const { return mSpawnData.mRace; }
	inline void setRaceID(const RaceID pRaceID) { mSpawnData.mRace = pRaceID; }

	inline const float getWalkSpeed() const { return mSpawnData.mWalkSpeed; }
	inline void setWalkSpeed(const float pWalkSpeed) { mSpawnData.mWalkSpeed = pWalkSpeed; }

	inline const bool isPet() const { return mSpawnData.mIsPet == 1; }
	inline void setIsPet(const bool pIsPet) { mSpawnData.mIsPet = pIsPet ? 1 : 0; }

	inline const uint8 getLight() const { return mSpawnData.mLight; }
	inline void setLight(const uint8 pLight) { mSpawnData.mLight = pLight; }

	inline const FlyMode getFlyMode() const { return mSpawnData.mFlyMode; }
	inline void getFlyMode(const FlyMode pFlyMode) { mSpawnData.mFlyMode = pFlyMode; }

	inline const BodyType getBodyType() const { return mSpawnData.mBodyType; }
	inline void setBodyType(const BodyType pBodyType) { mSpawnData.mBodyType = pBodyType; }

	inline const Colour getColour(const int pSlotID) const {
		// TODO: Guard this.
		return mSpawnData.mEquipmentColours[pSlotID];
	}
	inline void setColour(const int pSlotID, const Colour pColour) {
		// TODO: Guard this.
		mSpawnData.mEquipmentColours[pSlotID] = pColour;
	}
	inline void setColour(const int pSlotID, const uint32 pColour) {
		// TODO: Guard this.
		mSpawnData.mEquipmentColours[pSlotID].mColour = pColour;
	}

	inline const bool isLFG() const { return mSpawnData.mIsLFG == 1; }
	inline void setIsLFG(const bool pIsLFG) { mSpawnData.mIsLFG = pIsLFG ? 1 : 0; }

protected:
	Vector3 mPosition;
	float mHeading = 0.0f;
	Vector3 mPositionDelta;
	float mHeadingDelta = 0.0f;
	int32 mAnimation = 0;

	Actor* mTarget = nullptr;
	Payload::SpawnData mSpawnData;
	Zone* mZone = nullptr;
private:

	int32 mCopper = 0;
	int32 mSilver = 0;
	int32 mGold = 0;
	int32 mPlatinum = 0;

	String mName = "";
	String mLastName = "";
	String mTitle = "";
	String mSuffix = "";

	float mVisibleRange = 30.0f;
	std::list<Character*> mVisibleTo; // Characters who can see this Actor.

	inline void _setName(const char* pName) { strncpy(mSpawnData.mName, pName, Limits::Character::MAX_NAME_LENGTH); }
	inline void _setLastName(const char* pLastName) { strncpy(mSpawnData.mLastName, pLastName, Limits::Character::MAX_LAST_NAME_LENGTH); }
	inline void _setTitle(const char* pTitle) { strncpy(mSpawnData.mTitle, pTitle, Limits::Character::MAX_TITLE_LENGTH); }
	inline void _setSuffix(const char* pSuffix) { strncpy(mSpawnData.mSuffix, pSuffix, Limits::Character::MAX_SUFFIX_LENGTH); }
};