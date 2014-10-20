#pragma once

#include "Constants.h"
#include "Payload.h"
#include "Vector3.h"
#include "../common/timer.h"
#include "Timer.h"

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

	inline std::list<Actor*>& getTargeters() { return mTargeters; }

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

	//inline const unsigned char* getPositionData() { return reinterpret_cast<unsigned char*>(&mActorData.mPosition); }
	inline const Payload::ActorPosition& getPositionData() { return mActorData.mPosition; }

	inline const float getVisibleRange() const { return mVisibleRange; }
	inline void setVisibleRange(const float pVisibleRange) { mVisibleRange = pVisibleRange; /* Notify scene? */ }
	inline std::list<Character*>& getVisibleTo() { return mVisibleTo; }
	inline const bool isSeen() const { return mVisibleTo.empty() == false; }

	inline void addVisibleTo(Character* pCharacter) { mVisibleTo.push_back(pCharacter); }
	inline void removeVisibleTo(Character* pCharacter) { mVisibleTo.remove(pCharacter); }

	inline const uint32 getSpawnID() const { return mActorData.mSpawnID; }
	inline void setSpawnID(const uint32 pSpawnID) { mActorData.mSpawnID = pSpawnID; mActorData.mPosition.mSpawnID = pSpawnID; }

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
		mActorData.mPosition.mX = FUCK::xFloatToEQ19(mPosition.x);
		mActorData.mPosition.mY = FUCK::xFloatToEQ19(mPosition.y);
		mActorData.mPosition.mZ = FUCK::xFloatToEQ19(mPosition.z);
		mActorData.mPosition.mHeading = FUCK::xFloatToEQ19(mHeading);
		// Delta
		mActorData.mPosition.mDeltaX = FUCK::xNewFloatToEQ13(mPositionDelta.x);
		mActorData.mPosition.mDeltaY = FUCK::xNewFloatToEQ13(mPositionDelta.y);
		mActorData.mPosition.mDeltaZ = FUCK::xNewFloatToEQ13(mPositionDelta.z);
		mActorData.mPosition.mDeltaHeading = FUCK::xNewFloatToEQ13(mHeadingDelta);
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

	inline const uint8 getAnonymous() const { return mActorData.mFlags.mAnonymous; }
	inline const bool isAnonymous() const { return getAnonymous() == AnonType::Anonymous; }
	inline const bool isRoleplaying() const { return getAnonymous() == AnonType::Roleplay; }

	inline void setAnonymous(const uint8 pAnonType) {
		mActorData.mFlags.mAnonymous = 0;
		switch (pAnonType) {
		case AnonType::None:
			mActorData.mFlags.mAnonymous = 0;
			return;
		case AnonType::Roleplay:
			mActorData.mFlags.mAnonymous = 1;
			return;
		case AnonType::Anonymous:
			mActorData.mFlags.mAnonymous = 0;
			return;
		default:
			Log::error("[Actor] Invalid anontype value in setAnonymous");
			break;
		}
	};

	inline const uint8 getGender() const { return mActorData.mFlags.mGender; }
	inline const bool isMale() const { return getGender() == Gender::Male; }
	inline const bool isFemale() const { return getGender() == Gender::Female; }
	inline const bool isMonster() const { return getGender() == Gender::Monster; }

	inline void setGender(const uint8 pGender) {
		mActorData.mFlags.mGender = 0;
		switch (pGender) {
		case Gender::Male:
			mActorData.mFlags.mGender = Gender::Male;
			return;
		case Gender::Female:
			mActorData.mFlags.mGender = Gender::Female;
			return;
		case Gender::Monster:
			mActorData.mFlags.mGender = Gender::Monster;
			return;
		default:
			Log::error("[Actor] Invalid gender value in setGender");
			break;
		}
	};

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
	
	inline const uint8 getClass() const { return mActorData.mClass; }
	inline void setClass(const uint8 pClassID) { mActorData.mClass = pClassID; }
	
	//inline const AATitle getAATitle() const { return mSpawnData.mAATitle; }
	//inline void setAATitle(const AATitle pAATitle) { mSpawnData.mAATitle = pAATitle; }
	
	inline const uint32 getDeityID() const { return mActorData.mDeity; }
	inline void setDeityID(const uint32 pDeityID) { mActorData.mDeity = pDeityID; }

	inline const float getSize() const { return mActorData.mSize; }
	inline void setSize(const float pSize) { mActorData.mSize = pSize; }

	inline const uint8 getActorType() const { return mActorData.mActorType; }
	inline void setActorType(const uint8 pActorType) { mActorData.mActorType = pActorType; }

	inline const uint8 getHPPercent() const { return mActorData.mHPPercent; }
	inline void setHPPercent(const uint8 pValue) { mActorData.mHPPercent = pValue; }

	inline const uint8 getStandState() const { return mActorData.mStandState; }
	inline void setStandState(const uint8 pStandState) { mActorData.mStandState = pStandState; }

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

	inline const bool isPVP() const { return mActorData.mPVP == 1; }
	inline void setIsPVP(const bool pValue) { mActorData.mPVP = pValue ? 1 : 0; }

	inline const float getWalkSpeed() const { return mActorData.mWalkSpeed; }
	inline void setWalkSpeed(const float pWalkSpeed) { mActorData.mWalkSpeed = pWalkSpeed; }

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

	inline const uint8 getPrimaryDamageType() const { return mPrimaryDamageType; }
	inline const uint8 getSecondaryDamageType() const { return mSecondaryDamageType; }
	inline const uint8 getRangeDamageType() const { return mRangeDamageType; }

	inline void setPrimaryDamageType(const uint8 pValue) { mPrimaryDamageType = pValue; }
	inline void setSecondaryDamageType(const uint8 pValue) { mSecondaryDamageType = pValue; }
	inline void setRangeDamageType(const uint8 pValue) { mRangeDamageType = pValue; }

	inline const uint8 getPrimaryAttackAnimation() const { return mPrimaryAttackAnimation; }
	inline const uint8 getSecondaryAttackAnimation() const { return mSecondaryAttackAnimation; }
	inline const uint8 getRangeAttackAnimation() const { return mRangeAttackAnimation; }

	inline void setPrimaryAttackAnimation(const uint8 pValue) { mPrimaryAttackAnimation = pValue; }
	inline void setSecondaryAttackAnimation(const uint8 pValue) { mSecondaryAttackAnimation = pValue; }
	inline void setRangeAttackAnimation(const uint8 pValue) { mRangeAttackAnimation = pValue; }

	inline const bool isStunImmune() const { return mIsStunImmune; }
	inline void setStunImmune(const bool pValue) { mIsStunImmune = pValue; }

	inline const bool isStunned() const { return mIsStunned; }
	inline void setIsStunned(const bool pValue) { mIsStunned = pValue; }

	inline const bool isInvulnerable() const { return mIsInvulnerable; }
	inline void setInvulnerable(const bool pValue) { mIsInvulnerable = pValue; }

	inline const int32 getCurrentHP() const { return mCurrentHP; }
	inline void setCurrentHP(const int32 pValue) { mCurrentHP = pValue; }
	inline const int32 getMaximumHP() const { return mMaximumHP; }
	inline void setMaximumHP(const int32 pValue) { mMaximumHP = pValue; }
	inline const int32 getCurrentMana() const { return mCurrentMana; }
	inline void setCurrentMana(const int32 pValue) { mCurrentMana = pValue; }
	inline const int32 getMaximumMana() const { return mMaximumMana; }
	inline void setMaximumMana(const int32 pValue) { mMaximumMana = pValue; }
	inline const int32 getCurrentEndurance() const { return mCurrentEndurance; }
	inline void setCurrentEndurance(const int32 pValue) { mCurrentEndurance = pValue; }
	inline const int32 getMaximumEndurance() const { return mMaximumEndurance; }
	inline void setMaximumEndurance(const int32 pValue) { mMaximumEndurance = pValue; }

	inline const bool isDead() const { return mIsDead; }
	inline void setIsDead(const bool pValue) { mIsDead = pValue; }

	void damage(const int32 pAmount);

protected:

	// Returns a number of type T between 0-100 representing current HP percentage.
	template <typename T = uint8>
	inline const T _calcHPPercent() const {
		static_assert(std::is_arithmetic<T>::value, "T must be a number.");
		return static_cast<T>(std::floor((static_cast<float>(mCurrentHP) / static_cast<float>(mMaximumHP)) * 100));
	}

	Vector3 mPosition;
	float mHeading = 0.0f;
	Vector3 mPositionDelta;
	float mHeadingDelta = 0.0f;
	int32 mAnimation = 0;

	Character* mLooter = nullptr;
	Timer mDecayTimer;
	Payload::ActorData mActorData;
	Zone* mZone = nullptr;

	TTimer mPrimaryAttackTimer;
	TTimer mSecondaryAttackTimer;
	TTimer mRangeAttackTimer;
private:

	void _onCopy();

	bool mDestroy = false;

	String mName = "";
	String mLastName = "";
	String mTitle = "";
	String mSuffix = "";

	uint8 mPrimaryDamageType = DamageType::HandtoHand;
	uint8 mSecondaryDamageType = DamageType::HandtoHand;
	uint8 mRangeDamageType = DamageType::Archery;

	uint8 mPrimaryAttackAnimation = Animation::ANIM_HAND2HAND;
	uint8 mSecondaryAttackAnimation = Animation::ANIM_HAND2HAND;
	uint8 mRangeAttackAnimation = Animation::ANIM_SHOOTBOW;
	
	bool mIsInvulnerable = false;
	bool mIsStunImmune = false;
	bool mIsStunned = false;
	bool mIsDead = false;

	int32 mCurrentHP = 100;
	int32 mMaximumHP = 100;
	int32 mCurrentMana = 100;
	int32 mMaximumMana = 100;
	int32 mCurrentEndurance = 100;
	int32 mMaximumEndurance = 100;

	float mVisibleRange = 3000.0f;
	std::list<Character*> mVisibleTo; // Characters who can see this Actor.

	Actor* mTarget = nullptr; // Current target.
	std::list<Actor*> mTargeters; // Actors currently targeting this Actor.

	inline void _setName(const char* pName) { strncpy(mActorData.mName, pName, Limits::Character::MAX_NAME_LENGTH); }
	inline void _setLastName(const char* pLastName) { strncpy(mActorData.mLastName, pLastName, Limits::Character::MAX_LAST_NAME_LENGTH); }
	inline void _setTitle(const char* pTitle) { strncpy(mActorData.mTitle, pTitle, Limits::Character::MAX_TITLE_LENGTH); }
	inline void _setSuffix(const char* pSuffix) { strncpy(mActorData.mSuffix, pSuffix, Limits::Character::MAX_SUFFIX_LENGTH); }

	const bool sendsEquipment() const;
};