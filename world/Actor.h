#pragma once

#include "Constants.h"
#include "Payload.h"
#include "Vector3.h"
#include "../common/timer.h"

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
	Actor();
	virtual ~Actor() {};

	void setZone(Zone* pZone) { mZone = pZone; }
	Zone* getZone() const { return mZone; }

	inline virtual const bool isCharacter() const { return false; }
	inline virtual const bool isCharacterCorpse() const { return false; }
	inline virtual const bool isNPC() const { return false; }
	inline virtual const bool isNPCCorpse() const { return false; }
	inline const bool isCorpse() const { return (isCharacterCorpse() || isNPCCorpse()); }

	// Target
	void setTarget(Actor* pActor);
	inline Actor* getTarget() const { return mTarget; }
	inline const bool hasTarget() const { return mTarget != nullptr; }
	inline const bool targetIsCharacter() const { return mTarget ? mTarget->isCharacter() : false; }
	inline void addTargeter(Actor* pActor) { mTargeters.push_back(pActor); }
	inline void removeTargeter(Actor* pActor) { mTargeters.remove(pActor); }
	inline void clearTarget(const bool pRemoveFromTargeter = true) {
		if (!mTarget) return;

		if (pRemoveFromTargeter)
			mTarget->removeTargeter(this);
		mTarget = nullptr;
	}
	
	// Removes this Actor from any other Actors that have this Actor targeted.
	inline void clearTargeters() {
		for (auto i : mTargeters) {
			i->clearTarget(false);
		}
		mTargeters.clear();
	}

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

	virtual const bool onDeath() = 0;

	//inline const unsigned char* getActorData() { return reinterpret_cast<unsigned char*>(&mSpawnData); }
	inline const unsigned char* getPositionData() { return reinterpret_cast<unsigned char*>(&mActorData.mPosition); }

	inline const float getVisibleRange() const { return mVisibleRange; }
	inline void setVisibleRange(const float pVisibleRange) { mVisibleRange = pVisibleRange; /* Notify scene? */ }
	inline std::list<Character*>& getVisibleTo() { return mVisibleTo; }
	inline const bool isSeen() const { return mVisibleTo.empty() == false; }

	inline void addVisibleTo(Character* pCharacter) { mVisibleTo.push_back(pCharacter); }
	inline void removeVisibleTo(Character* pCharacter) { mVisibleTo.remove(pCharacter); }

	inline const SpawnID getSpawnID() const { return mActorData.mSpawnID; }
	inline void setSpawnID(const SpawnID pSpawnID) { mActorData.mSpawnID = pSpawnID; }

	inline const String& getName() const { return mName; }
	inline void setName(const String& pName) { mName = pName; _setName(pName.c_str()); }
	inline const String& getLastName() const { return mLastName; }
	inline void setLastName(const String& pLastName) { mLastName = pLastName; _setLastName(pLastName.c_str()); }
	
	inline const bool hasTitle() const { return mTitle.length() > 0; }
	inline const String& getTitle() const { return mTitle; }
	inline void setTitle(const String& pValue) { mTitle = pValue; _setTitle(pValue.c_str()); }
	
	inline const bool hasSuffix() const { return mSuffix.length() > 0; }
	inline const String& getSuffix() const { return mSuffix; }
	inline void setSuffix(const String& pValue) { mSuffix = pValue; _setSuffix(pValue.c_str()); }

	inline const bool isDestructible() const { return false; }

	inline void _syncPosition() {
		// Current
		mActorData.mPosition.x = FUCK::xFloatToEQ19(mPosition.x);
		mActorData.mPosition.y = FUCK::xFloatToEQ19(mPosition.y);
		mActorData.mPosition.z = FUCK::xFloatToEQ19(mPosition.z);
		mActorData.mPosition.heading = FUCK::xFloatToEQ19(mHeading);
		// Delta
		mActorData.mPosition.deltaX = FUCK::xNewFloatToEQ13(mPositionDelta.x);
		mActorData.mPosition.deltaY = FUCK::xNewFloatToEQ13(mPositionDelta.y);
		mActorData.mPosition.deltaZ = FUCK::xNewFloatToEQ13(mPositionDelta.z);
		mActorData.mPosition.deltaHeading = FUCK::xNewFloatToEQ13(mHeadingDelta);
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

	inline const uint8 getStandingState() const { return mActorData.mStandState; }
	inline void setStandingState(const uint8 pStandingState) { mActorData.mStandState = pStandingState; }

	// Visual
	inline const uint8 getFaceStyle() const { return mActorData.mFaceStyle; }
	inline const uint8 getLeftEyeColour() const { return mActorData.mLeftEyeColour; }
	inline const uint8 getRightEyeColour() const { return mActorData.mRightEyeColour; }
	inline const uint8 getHairStyle() const { return mActorData.mHairStyle; }
	inline const uint8 getBeardStyle() const { return mActorData.mBeardStyle; }
	inline const uint8 getHairColour() const { return mActorData.mHairColour; }
	inline const uint8 getBeardColour() const { return mActorData.mBeardColour; }
	inline const uint32 getDrakkinHeritage() const { return mActorData.mDrakkinHeritage; }
	inline const uint32 getDrakkinTattoo() const { return mActorData.mDrakkinTattoo; }
	inline const uint32 getDrakkinDetails() const { return mActorData.mDrakkinDetails; }

	inline void setFaceStyle(const uint8 pFaceStyle) { mActorData.mFaceStyle = pFaceStyle; }
	inline void setLeftEyeColour(const uint8 pLeftEyeColour) { mActorData.mLeftEyeColour = pLeftEyeColour; }
	inline void setRightEyeColour(const uint8 pRightEyeColour) { mActorData.mRightEyeColour = pRightEyeColour; }
	inline void setHairStyle(const uint8 pHairStyle) { mActorData.mHairStyle = pHairStyle; }
	inline void setBeardStyle(const uint8 pBeardStyle) { mActorData.mBeardStyle = pBeardStyle; }
	inline void setHairColour(const uint8 pHairColour) { mActorData.mHairColour = pHairColour; }
	inline void setBeardColour(const uint8 pBeardColour) { mActorData.mBeardColour = pBeardColour; }
	inline void setDrakkinHeritage(const uint32 pDrakkinHeritage) { mActorData.mDrakkinHeritage = pDrakkinHeritage; }
	inline void setDrakkinTattoo(const uint32 pDrakkinTattoo) { mActorData.mDrakkinTattoo = pDrakkinTattoo; }
	inline void setDrakkinDetails(const uint32 pDrakkinDetails) { mActorData.mDrakkinDetails = pDrakkinDetails; }

	// Flags
	inline const bool isPet() const { return mActorData.mFlags.mIsPet == 1; }
	inline void setIsPet(const bool pValue) { mActorData.mFlags.mIsPet = pValue ? 1 : 0; }

	inline const bool isAFK() const { return mActorData.mFlags.mIsAFK == 1; }
	inline void setIsAFK(const bool pValue) { mActorData.mFlags.mIsAFK = pValue ? 1 : 0; }

	inline const bool isSneaking() const { return mActorData.mFlags.mIsSneaking == 1; }
	inline void setIsSneaking(const bool pValue) { mActorData.mFlags.mIsSneaking = pValue ? 1 : 0; }

	inline const bool isLFG() const { return mActorData.mFlags.mLFG == 1; }
	inline void setIsLFG(const bool pValue) { mActorData.mFlags.mLFG = pValue ? 1 : 0; }

	inline const bool isInvisible() { return mActorData.mFlags.mIsInvisible == 1; }
	inline const bool setIsInvisible(const bool pValue) { mActorData.mFlags.mIsInvisible = pValue ? 1 : 0; }

	inline const bool isGM() const { return mActorData.mFlags.mIsGM == 1; }
	inline void setIsGM(const bool pIsGM) { mActorData.mFlags.mIsGM = pIsGM ? 1 : 0; }

	// Anonymous

	// Gender

	inline const bool isLD() { return mActorData.mFlags.mIsLD == 1; }
	inline void setIsLD(const bool pValue) { mActorData.mFlags.mIsLD = pValue ? 1 : 0; }

	inline const bool getShowHelm() const { return mActorData.mFlags.mShowHelm & 1; }
	inline void setShowHelm(const bool pValue) { mActorData.mFlags.mShowHelm = pValue ? 1 : 0; }

	inline const bool getShowName() const { return mActorData.mFlags.mShowName == 1; }
	inline void setShowName(const bool pValue) { mActorData.mFlags.mShowName = pValue ? 1 : 0; }

	inline const bool isTrader() const { return mActorData.mFlags.mIsTrader == 1 ; }
	inline const bool setIsTrader(const bool pValue) { mActorData.mFlags.mIsTrader = pValue ? 1 : 0; }

	inline const bool isBuyer() const { return mActorData.mFlags.mIsBuyer == 1; }
	inline const bool setIsBuyer(const bool pValue) { mActorData.mFlags.mIsBuyer = pValue ? 1 : 0; }
	 
	//inline const bool 

	//inline const GenderID getGender() const { return mActorData.mGender; }
	//inline void setGender(const GenderID pGender) { mSpawnData.mGender = pGender; }
	// TODO!
	inline const GenderID getGender() const { return 0; }
	inline void setGender(const GenderID pGender) { };
	
	inline const uint8 getClass() const { return mActorData.mClass; }
	inline void setClass(const uint8 pClassID) { mActorData.mClass = pClassID; }
	
	//inline const AATitle getAATitle() const { return mSpawnData.mAATitle; }
	//inline void setAATitle(const AATitle pAATitle) { mSpawnData.mAATitle = pAATitle; }
	
	//inline const AnonType getAnonymous() const { return mSpawnData.mAnonymous; }
	//inline void setAnonymous(const AnonType pAnonType) { mSpawnData.mAnonymous = pAnonType; }
	// TODO!
	inline const AnonType getAnonymous() const { return AnonType::AT_None; }
	inline void setAnonymous(const AnonType pAnonType) { };
	
	inline const uint32 getDeityID() const { return mActorData.mDeity; }
	inline void setDeityID(const uint32 pDeityID) { mActorData.mDeity = pDeityID; }

	inline const float getSize() const { return mActorData.mSize; }
	inline void setSize(const float pSize) { mActorData.mSize = pSize; }

	inline const uint8 getActorType() const { return mActorData.mActorType; }
	inline void setActorType(const uint8 pActorType) { mActorData.mActorType = pActorType; }

	//inline const bool isInvisible() const { return mActorData.mFlags.mIsInvisible == 1; }
	//inline void setInvisible(const bool pInvisible) { mActorData.mFlags.mIsInvisible ? 1 : 0; }

	inline const uint8 getHPPercent() const { return mActorData.mHPPercent; }
	inline void setHPPercent(const uint8 pValue) { mActorData.mHPPercent = pValue; }

	//inline const bool isFindable() const { return mSpawnData.mIsFindable == 1; }
	//inline void setIsFindable(const bool pFindable) { mSpawnData.mIsFindable = pFindable ? 1 : 0; }

	inline const uint8 getStandState() const { return mActorData.mStandState; }
	inline void setStandState(const uint8 pStandState) { mActorData.mStandState = pStandState; }

	//inline const bool getIsNPC() const { return mSpawnData.mIsNPC == 1; }
	//inline void setIsNPC(const bool pIsNPC) { mSpawnData.mIsNPC = pIsNPC ? 1 : 0; }

	inline const uint8 getLevel() const { return mActorData.mLevel; }
	inline void setLevel(const uint8 pLevel) { mActorData.mLevel = pLevel; }

	inline const uint32 getOwnerSpawnID() const { return mActorData.mOwnerSpawnID; }
	inline void setOwnerSpawnID(const uint32 pSpawnID) { mActorData.mOwnerSpawnID = pSpawnID; }

	inline const uint32 getGuildRank() const { return mActorData.mGuildRank; }
	inline void setGuildRank(const uint32 pGuildRank) { mActorData.mGuildRank = pGuildRank; }

	inline const uint32 getMaterial(const MaterialSlot pSlotID) const {
		// TODO: Guard this.
		//return mSpawnData.mEquipmentMaterials[pSlotID];
		// TODO!
		return 0;
	}
	inline const uint32 getMaterial(const int pSlotID) const {
		// TODO: Guard this.
		//return mSpawnData.mEquipmentMaterials[pSlotID];
		// TODO!
		return 0;
	}
	inline void setMaterial(const MaterialSlot pSlotID, const uint32 pMaterial) {
		// TODO: Guard this.
		//mActorData.mEquipment[pSlotID] = pMaterial;
		// TODO!
	}

	inline const float getRunSpeed() const { return mActorData.mRunSpeed; }
	inline void setRunSpeed(const float pRunSpeed) { mActorData.mRunSpeed = pRunSpeed; }

	inline const uint32 getGuildID() const { return mActorData.mGuildID; }
	inline void setGuildID(const uint32 pGuildID) { mActorData.mGuildID = pGuildID; }

	inline const uint32 getRace() const { return mActorData.mRace; }
	inline void setRace(const uint32 pRaceID) { mActorData.mRace = pRaceID; }

	inline const float getWalkSpeed() const { return mActorData.mWalkSpeed; }
	inline void setWalkSpeed(const float pWalkSpeed) { mActorData.mWalkSpeed = pWalkSpeed; }

	//inline const bool isPet() const { return mActorData.mFlags.mIsPet == 1; }
	//inline void setIsPet(const bool pIsPet) { mActorData.mFlags.mIsPet = pIsPet ? 1 : 0; }

	inline const uint8 getLight() const { return mActorData.mLight; }
	inline void setLight(const uint8 pLight) { mActorData.mLight = pLight; }

	inline const uint8 getFlyMode() const { return mActorData.mFlyMode; }
	inline void getFlyMode(const uint8 pFlyMode) { mActorData.mFlyMode = pFlyMode; }

	inline const uint32 getBodyType() const { return mActorData.mBodyType; }
	inline void setBodyType(const uint32 pBodyType) { mActorData.mBodyType = pBodyType; }

	//inline const Colour getColour(const int pSlotID) const {
	//	// TODO: Guard this.
	//	return mActorData.mColours[pSlotID];
	//}
	//inline void setColour(const int pSlotID, const Colour pColour) {
	//	// TODO: Guard this.
	//	//mSpawnData.mEquipmentColours[pSlotID] = pColour;
	//	mActorData.mColours[pSlotID] = pColour.mColour;
	//}
	inline void setColour(const int pSlotID, const uint32 pColour) {
		// TODO: Guard this.
		//mSpawnData.mEquipmentColours[pSlotID].mColour = pColour;
		mActorData.mColours[pSlotID] = pColour;
	}

	// Looting
	inline const bool hasLooter() const { return mLooter != nullptr; }
	inline Character* getLooter() const { return mLooter; }
	inline void setLooter(Character* pCharacter) { mLooter = pCharacter; }

	const bool hasItems() const { return false; } // TODO: Items

	inline void destroy() { mDestroy = true; }
	inline const bool getDestroy() const { return mDestroy; }

	const uint32 getDataSize() const;
	const bool copyData(Utility::DynamicStructure& pStructure);

protected:
	Vector3 mPosition;
	float mHeading = 0.0f;
	Vector3 mPositionDelta;
	float mHeadingDelta = 0.0f;
	int32 mAnimation = 0;

	Character* mLooter = nullptr;
	Timer mDecayTimer;
	Payload::ActorData mActorData;
	Zone* mZone = nullptr;
private:

	void _onCopy();

	bool mDestroy = false;

	String mName = "";
	String mLastName = "";
	String mTitle = "";
	String mSuffix = "";

	float mVisibleRange = 3000.0f;
	std::list<Character*> mVisibleTo; // Characters who can see this Actor.

	Actor* mTarget = nullptr; // Current target.
	std::list<Actor*> mTargeters; // Actors currently targeting this Actor.

	inline void _setName(const char* pName) { strncpy(mActorData.mName, pName, Limits::Character::MAX_NAME_LENGTH); }
	inline void _setLastName(const char* pLastName) { strncpy(mActorData.mLastName, pLastName, Limits::Character::MAX_LAST_NAME_LENGTH); }
	inline void _setTitle(const char* pTitle) { strncpy(mActorData.mTitle, pTitle, Limits::Character::MAX_TITLE_LENGTH); }
	inline void _setSuffix(const char* pSuffix) { strncpy(mActorData.mSuffix, pSuffix, Limits::Character::MAX_SUFFIX_LENGTH); }
};