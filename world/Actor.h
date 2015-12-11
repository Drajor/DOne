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
class HateController;
class LootController;
class BuffController;
class ActorBonuses;
class Bonuses;

struct CastingState {
	u16 mTargetID = 0;
	u16 mSpellID = 0;
	u16 mCastTimeMS = 0;
	Vector3 mOrigin;

	void clear() {
		mTargetID = 0;
		mSpellID = 0;
		mCastTimeMS = 0;
		mOrigin.x = 0.0f;
		mOrigin.y = 0.0f;
		mOrigin.z = 0.0f;
	}
};

class Actor {
public:
	Actor();
	virtual ~Actor();

	// Sets the Zone this Actor is in.
	void setZone(Zone* pZone) { mZone = pZone; }

	// Returns the Zone this Actor is in.
	Zone* getZone() const { return mZone; }

	inline HateController* getHateController() const { return mHateController; }
	inline LootController* getLootController() const { return mLootController; }
	inline BuffController* getBuffController() const { return mBuffController; }
	inline ActorBonuses* getActorBonuses() const { return mActorBonuses; }
	inline Bonuses* getBaseBonuses() const { return mBaseBonuses; }
	inline CastingState& getCastingState() { return mCastingState; }
	
	// Returns a copy of the hate list.
	inline std::list<Actor*> getHaters() { return mHaters; }
	inline const bool hasHaters() const { return !mHaters.empty(); }
	inline void addHater(Actor* pHater) { mHaters.push_back(pHater); }
	inline void removeHater(Actor* pHater) { mHaters.remove(pHater); }
	inline void clearHaters() { mHaters.clear(); }


	// Returns whether this Actor is a Character or not.
	inline virtual const bool isCharacter() const { return false; }

	// Returns whether this Actor is a Character corpse or not.
	inline virtual const bool isCharacterCorpse() const { return false; }

	// Returns whether this Actor is an NPC or not.
	inline virtual const bool isNPC() const { return false; }

	// Returns whether this Actor is an NPC corpse or not.
	inline virtual const bool isNPCCorpse() const { return false; }

	// Returns whether this Actor is a corpse or not.
	inline const bool isCorpse() const { return (isCharacterCorpse() || isNPCCorpse()); }

	// Target
	void setTarget(Actor* pActor);
	inline Actor* getTarget() const { return mTarget; }
	inline const bool hasTarget() const { return mTarget != nullptr; }
	inline const bool targetIsCharacter() const { return mTarget ? mTarget->isCharacter() : false; }
	inline const bool targetIsNPC() const { return mTarget ? mTarget->isNPC() : false; }
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

	// Returns whether or not anything has this Actor targeted.
	inline const bool hasTargeters() const { return !mTargeters.empty(); }

	// Returns a list of Actors who have this Actor targeted.
	inline std::list<Actor*>& getTargeters() { return mTargeters; }

	template <typename T>
	inline static T cast(Actor* pActor) {
		return static_cast<T>(pActor);
	}
	inline const float distanceTo(const Actor* pActor) const {
		return mPosition.distance(pActor->mPosition);
	}
	inline const float squareDistanceTo(const Actor* pActor) const {
		return mPosition.squareDistance(pActor->mPosition);
	}

	virtual const bool onDeath() = 0;

	inline const Payload::ActorPosition& getPositionData() { return mActorData.mPosition; }

	inline const float getVisibleRange() const { return mVisibleRange; }
	inline void setVisibleRange(const float pVisibleRange) { mVisibleRange = pVisibleRange; /* Notify scene? */ }
	inline std::list<Character*>& getVisibleTo() { return mVisibleTo; }
	inline const bool isSeen() const { return mVisibleTo.empty() == false; }

	inline void addVisibleTo(Character* pCharacter) { mVisibleTo.push_back(pCharacter); }
	inline void removeVisibleTo(Character* pCharacter) { mVisibleTo.remove(pCharacter); }

	inline const u32 getSpawnID() const { return mActorData.mSpawnID; }
	inline void setSpawnID(const u32 pSpawnID) { mActorData.mSpawnID = pSpawnID; mActorData.mPosition.mSpawnID = pSpawnID; }

	inline const String& getName() const { return mName; }
	inline void setName(const String& pName) { mName = pName; _setName(pName.c_str()); }
	inline const String& getSurname() const { return mSurname; }
	inline void setSurname(const String& pSurname) { mSurname = pSurname; _setSurname(pSurname.c_str()); }
	
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

	inline const i32 getAnimation() const { return mAnimation; }
	inline void setAnimation(const i32 pAnimation) { mAnimation = pAnimation; }

	inline const u8 getStandingState() const { return mActorData.mStandState; }
	inline void setStandingState(const u8 pStandingState) { mActorData.mStandState = pStandingState; }

	// Visual
	inline const u8 getFaceStyle() const { return mActorData.mFaceStyle; }
	inline const u8 getLeftEyeColour() const { return mActorData.mLeftEyeColour; }
	inline const u8 getRightEyeColour() const { return mActorData.mRightEyeColour; }
	inline const u8 getHairStyle() const { return mActorData.mHairStyle; }
	inline const u8 getBeardStyle() const { return mActorData.mBeardStyle; }
	inline const u8 getHairColour() const { return mActorData.mHairColour; }
	inline const u8 getBeardColour() const { return mActorData.mBeardColour; }
	inline const u32 getDrakkinHeritage() const { return mActorData.mDrakkinHeritage; }
	inline const u32 getDrakkinTattoo() const { return mActorData.mDrakkinTattoo; }
	inline const u32 getDrakkinDetails() const { return mActorData.mDrakkinDetails; }
	inline const u8 getTexture() const { return mActorData.mTexture; }
	inline const u8 getHelmTexture() const { return mActorData.mShowHelm; }

	inline void setFaceStyle(const u8 pFaceStyle) { mActorData.mFaceStyle = pFaceStyle; }
	inline void setLeftEyeColour(const u8 pLeftEyeColour) { mActorData.mLeftEyeColour = pLeftEyeColour; }
	inline void setRightEyeColour(const u8 pRightEyeColour) { mActorData.mRightEyeColour = pRightEyeColour; }
	inline void setHairStyle(const u8 pHairStyle) { mActorData.mHairStyle = pHairStyle; }
	inline void setBeardStyle(const u8 pBeardStyle) { mActorData.mBeardStyle = pBeardStyle; }
	inline void setHairColour(const u8 pHairColour) { mActorData.mHairColour = pHairColour; }
	inline void setBeardColour(const u8 pBeardColour) { mActorData.mBeardColour = pBeardColour; }
	inline void setDrakkinHeritage(const u32 pDrakkinHeritage) { mActorData.mDrakkinHeritage = pDrakkinHeritage; }
	inline void setDrakkinTattoo(const u32 pDrakkinTattoo) { mActorData.mDrakkinTattoo = pDrakkinTattoo; }
	inline void setDrakkinDetails(const u32 pDrakkinDetails) { mActorData.mDrakkinDetails = pDrakkinDetails; }
	inline void setTexture(const u8 pValue) { mActorData.mTexture = pValue; }
	inline void setHelmTexture(const u8 pValue) { mActorData.mShowHelm = pValue; }

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

	inline const u8 getAnonymous() const { return mActorData.mFlags.mAnonymous; }
	inline const bool isAnonymous() const { return getAnonymous() == AnonType::Anonymous; }
	inline const bool isRoleplaying() const { return getAnonymous() == AnonType::Roleplay; }

	inline void setAnonymous(const u8 pAnonType) {
		mActorData.mFlags.mAnonymous = 0;
		switch (pAnonType) {
		case AnonType::None:
			mActorData.mFlags.mAnonymous = 0;
			return;
		case AnonType::Anonymous:
			mActorData.mFlags.mAnonymous = 1;
			return;
		case AnonType::Roleplay:
			mActorData.mFlags.mAnonymous = 2;
			return;
		default:
			Log::error("[Actor] Invalid anontype value in setAnonymous");
			break;
		}
	};

	inline const u8 getGender() const { return mActorData.mFlags.mGender; }
	inline const bool isMale() const { return getGender() == Gender::Male; }
	inline const bool isFemale() const { return getGender() == Gender::Female; }
	inline const bool isMonster() const { return getGender() == Gender::Monster; }

	inline void setGender(const u8 pGender) {
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

	// Sets whether this Actor is LD or not.
	inline const bool isLD() { return mActorData.mFlags.mIsLD == 1; }

	// Returns whether this Actor is LD or not.
	inline void setIsLD(const bool pValue) { mActorData.mFlags.mIsLD = pValue ? 1 : 0; }

	// Returns whether this Actor shows their helm or not.
	inline const bool getShowHelm() const { return mActorData.mFlags.mShowHelm & 1; }

	// Sets whether this Actor shows their helm or not.
	inline void setShowHelm(const bool pValue) { mActorData.mFlags.mShowHelm = pValue ? 1 : 0; }

	// Returns whether this Actor shows their name or not.
	inline const bool getShowName() const { return mActorData.mFlags.mShowName == 1; }

	// Sets whether  this Actor shows their name or not.
	inline void setShowName(const bool pValue) { mActorData.mFlags.mShowName = pValue ? 1 : 0; }

	// Returns whether this Actor is a trader or not.
	inline const bool isTrader() const { return mActorData.mFlags.mIsTrader == 1 ; }

	// Sets whether this Actor is a trader or not.
	inline void setIsTrader(const bool pValue) { mActorData.mFlags.mIsTrader = pValue ? 1 : 0; }

	// Returns whether this Actor is a buyer or not.
	inline const bool isBuyer() const { return mActorData.mFlags.mIsBuyer == 1; }
	
	// Sets whether this Actor is a buyer or not.
	inline void setIsBuyer(const bool pValue) { mActorData.mFlags.mIsBuyer = pValue ? 1 : 0; }

	// Returns whether this Actor is targetable is not.
	inline const bool isTargetable() { return mActorData.mFlags.mTargetable == 1; }

	// Sets whether this Actor is targetable or not.
	inline void setTargetable(const bool pValue) { mActorData.mFlags.mTargetable = pValue ? 1 : 0; }
	
	// Returns the class of this Actor.
	inline const u8 getClass() const { return mActorData.mClass; }

	// Sets the class of this Actor.
	inline void setClass(const u8 pClassID) { mActorData.mClass = pClassID; }
	
	//inline const AATitle getAATitle() const { return mSpawnData.mAATitle; }
	//inline void setAATitle(const AATitle pAATitle) { mSpawnData.mAATitle = pAATitle; }
	
	// Returns the deity of this Actor.
	inline const u32 getDeity() const { return mActorData.mDeity; }

	// Sets the deity of this Actor.
	inline void setDeityID(const u32 pDeityID) { mActorData.mDeity = pDeityID; }

	// Returns the size of this Actor.
	inline const float getSize() const { return mActorData.mSize; }

	// Sets the size of this Actor.
	inline void setSize(const float pSize) { mActorData.mSize = pSize; }

	// Returns the actor type of this Actor. See Constants 'ActorType' enumeration.
	inline const u8 getActorType() const { return mActorData.mActorType; }

	// Sets the actor type of this Actor. See Constants 'ActorType' enumeration.
	inline void setActorType(const u8 pActorType) { mActorData.mActorType = pActorType; }

	// Returns the health percentage of this Actor.
	inline const u8 getHPPercent() const { return mActorData.mHPPercent; }

	// Sets the health percentage of this Actor.
	inline void setHPPercent(const u8 pValue) { mActorData.mHPPercent = pValue; }

	// Returns the stand state of this Actor.
	inline const u8 getStandState() const { return mActorData.mStandState; }

	// Sets the stand state of this Actor.
	inline void setStandState(const u8 pStandState) { mActorData.mStandState = pStandState; }

	// Returns the level of this Actor.
	virtual const u8 getLevel() const { return mActorData.mLevel; }

	// Sets the level of this Actor.
	virtual void setLevel(const u8 pLevel) { mActorData.mLevel = pLevel; }

	// Returns the owner of this Actor.
	inline const Actor* getOwner() const { return mOwner; }

	// Sets the owner of this Actor.
	inline void setOwner(Actor* pOwner) { mOwner = pOwner; }

	// Returns the spawn ID of the owner of this Actor.
	inline const u32 getOwnerID() const { return mActorData.mOwnerID; }

	// Sets the spawn ID of the owner of this Actor.
	inline void setOwnerID(const u32 pID) { mActorData.mOwnerID = pID; }

	// Returns the guild rank of this Actor.
	inline const u32 getGuildRank() const { return mActorData.mGuildRank; }

	// Sets the guild rank of this Actor.
	inline void setGuildRank(const u32 pGuildRank) { mActorData.mGuildRank = pGuildRank; }

	inline const u32 getMaterial(const u8 pSlotID) const {
		// TODO: Guard this.
		//return mSpawnData.mEquipmentMaterials[pSlotID];
		// TODO!
		return 0;
	}
	inline const u32 getMaterial(const int pSlotID) const {
		// TODO: Guard this.
		//return mSpawnData.mEquipmentMaterials[pSlotID];
		// TODO!
		return 0;
	}
	inline void setMaterial(const u8 pSlotID, const u32 pMaterial) {
		mActorData.mEquipment[pSlotID].mMaterial = pMaterial;
	}

	inline const float getRunSpeed() const { return mActorData.mRunSpeed; }
	inline void setRunSpeed(const float pRunSpeed) { mActorData.mRunSpeed = pRunSpeed; }

	inline const u32 getGuildID() const { return mActorData.mGuildID; }
	inline void setGuildID(const u32 pGuildID) { mActorData.mGuildID = pGuildID; }

	inline const u32 getRace() const { return mActorData.mRace; }
	inline void setRace(const u32 pRaceID) { mActorData.mRace = pRaceID; }

	inline const bool isPVP() const { return mActorData.mPVP == 1; }
	inline void setIsPVP(const bool pValue) { mActorData.mPVP = pValue ? 1 : 0; }

	inline const float getWalkSpeed() const { return mActorData.mWalkSpeed; }
	inline void setWalkSpeed(const float pWalkSpeed) { mActorData.mWalkSpeed = pWalkSpeed; }

	inline const u8 getLight() const { return mActorData.mLight; }
	inline void setLight(const u8 pLight) { mActorData.mLight = pLight; }

	inline const u8 getFlyMode() const { return mActorData.mFlyMode; }
	inline void getFlyMode(const u8 pFlyMode) { mActorData.mFlyMode = pFlyMode; }

	inline const u32 getBodyType() const { return mActorData.mBodyType; }
	inline void setBodyType(const u32 pBodyType) { mActorData.mBodyType = pBodyType; }

	//inline const Colour getColour(const int pSlotID) const {
	//	// TODO: Guard this.
	//	return mActorData.mColours[pSlotID];
	//}
	//inline void setColour(const int pSlotID, const Colour pColour) {
	//	// TODO: Guard this.
	//	//mSpawnData.mEquipmentColours[pSlotID] = pColour;
	//	mActorData.mColours[pSlotID] = pColour.mColour;
	//}
	inline void setColour(const int pSlotID, const u32 pColour) {
		// TODO: Guard this.
		//mSpawnData.mEquipmentColours[pSlotID].mColour = pColour;
		mActorData.mColours[pSlotID] = pColour;
	}

	//// Looting
	//inline const bool hasLooter() const { return mLootController->hasLooter(); }
	//inline Character* getLooter() const { return mLooter; }
	//inline void setLooter(Character* pCharacter) { mLooter = pCharacter; }

	virtual const bool hasItems() const { return false; } // TODO: Items

	inline void destroy() { mDestroy = true; }
	inline const bool getDestroy() const { return mDestroy; }

	const u32 getDataSize() const;
	const bool copyData(Utility::MemoryWriter& pWriter);

	inline const u8 getPrimaryDamageType() const { return mPrimaryDamageType; }
	inline const u8 getSecondaryDamageType() const { return mSecondaryDamageType; }
	inline const u8 getRangeDamageType() const { return mRangeDamageType; }

	inline void setPrimaryDamageType(const u8 pValue) { mPrimaryDamageType = pValue; }
	inline void setSecondaryDamageType(const u8 pValue) { mSecondaryDamageType = pValue; }
	inline void setRangeDamageType(const u8 pValue) { mRangeDamageType = pValue; }

	inline const u8 getPrimaryAttackAnimation() const { return mPrimaryAttackAnimation; }
	inline const u8 getSecondaryAttackAnimation() const { return mSecondaryAttackAnimation; }
	inline const u8 getRangeAttackAnimation() const { return mRangeAttackAnimation; }

	inline void setPrimaryAttackAnimation(const u8 pValue) { mPrimaryAttackAnimation = pValue; }
	inline void setSecondaryAttackAnimation(const u8 pValue) { mSecondaryAttackAnimation = pValue; }
	inline void setRangeAttackAnimation(const u8 pValue) { mRangeAttackAnimation = pValue; }

	inline const bool isStunImmune() const { return mIsStunImmune; }
	inline void setStunImmune(const bool pValue) { mIsStunImmune = pValue; }

	inline const bool isStunned() const { return mIsStunned; }
	inline void setIsStunned(const bool pValue) { mIsStunned = pValue; }

	inline const bool isInvulnerable() const { return mIsInvulnerable; }
	inline void setInvulnerable(const bool pValue) { mIsInvulnerable = pValue; }

	inline const bool isCasting() const { return mIsCasting; }
	inline void setIsCasting(const bool pValue) { mIsCasting = pValue; }

	inline const i32 getCurrentHP() const { return mCurrentHP; }
	inline void setCurrentHP(const i32 pValue) { mCurrentHP = pValue; }
	inline const i32 getMaximumHP() const { return mMaximumHP; }
	inline void setMaximumHP(const i32 pValue) { mMaximumHP = pValue; }
	inline const i32 getCurrentMana() const { return mCurrentMana; }
	inline void setCurrentMana(const i32 pValue) { mCurrentMana = pValue; }
	inline const i32 getMaximumMana() const { return mMaximumMana; }
	inline void setMaximumMana(const i32 pValue) { mMaximumMana = pValue; }
	inline const i32 getCurrentEndurance() const { return mCurrentEndurance; }
	inline void setCurrentEndurance(const i32 pValue) { mCurrentEndurance = pValue; }
	inline const i32 getMaximumEndurance() const { return mMaximumEndurance; }
	inline void setMaximumEndurance(const i32 pValue) { mMaximumEndurance = pValue; }

	inline const bool isDead() const { return mIsDead; }
	inline void setIsDead(const bool pValue) { mIsDead = pValue; }
	inline const bool isAlive() const { return !isDead(); }

	void damage(const i32 pAmount);

	const TTimer& getDecayTimer() const { return mDecayTimer; }
	TTimer& getOpenTimer() { return mOpenTimer; }

	inline void addNimbus(const u32 pNimbusID) { mNimbuses.push_back(pNimbusID); }
	inline void removeNimbus(const u32 pNimbusID) { mNimbuses.remove(pNimbusID); }
	inline std::list<u32> getNimbuses() { return mNimbuses; }

protected:

	// Returns a number of type T between 0-100 representing current HP percentage.
	template <typename T = u8>
	inline const T _calcHPPercent() const {
		static_assert(std::is_arithmetic<T>::value, "T must be a number.");
		return static_cast<T>(std::floor((static_cast<float>(mCurrentHP) / static_cast<float>(mMaximumHP)) * 100));
	}

	HateController* mHateController = nullptr;
	BuffController* mBuffController = nullptr;
	ActorBonuses* mActorBonuses = nullptr;
	Bonuses* mBaseBonuses = nullptr;

	std::list<Actor*> mHaters; // List of Actors who currently hate this Actor.

	Vector3 mPosition;
	float mHeading = 0.0f;
	Vector3 mPositionDelta;
	float mHeadingDelta = 0.0f;
	i32 mAnimation = 0;

	TTimer mDecayTimer; // Time until corpse rots.
	TTimer mOpenTimer; // Time until corpse becomes open, i.e. anyone can loot.

	Payload::ActorData mActorData;
	Zone* mZone = nullptr;

	TTimer mPrimaryAttackTimer;
	TTimer mSecondaryAttackTimer;
	TTimer mRangeAttackTimer;

	void _clearPrimary();

	inline const u32 getPrimaryDamage() const { return mPrimaryDamage; }
	inline const u32 getPrimaryMagicDamage() const { return mPrimaryMagicDamage; }
	inline const u32 getPrimaryFireDamage() const { return mPriamryFireDamage; }
	inline const u32 getPrimaryColdDamage() const { return mPrimaryColdDamage; }
	inline const u32 getPrimaryPoisonDamage() const { return mPrimaryPoisonDamage; }
	inline const u32 getPrimaryDiseaseDamage() const { return mPrimaryDiseaseDamage; }
	inline const u32 getPrimaryChromaticDamage() const { return mPrimaryChromaticDamage; }
	inline const u32 getPrimaryPrismaticDamage() const { return mPrimaryPrismaticDamage; }
	inline const u32 getPrimaryPhysicalDamage() const { return mPrimaryPhysicalDamage; }
	inline const u32 getPrimaryCorruptionDamage() const { return mPrimaryCorruptionDamage; }

	inline void setPrimaryDamage(const u32 pValue) { mPrimaryDamage = pValue; }
	inline void setPrimaryMagicDamage(const u32 pValue) { mPrimaryMagicDamage = pValue; }
	inline void setPrimaryFireDamage(const u32 pValue) { mPriamryFireDamage = pValue; }
	inline void setPrimaryColdDamage(const u32 pValue) { mPrimaryColdDamage = pValue; }
	inline void setPrimaryPoisonDamage(const u32 pValue) { mPrimaryPoisonDamage = pValue; }
	inline void setPrimaryDiseaseDamage(const u32 pValue) { mPrimaryDiseaseDamage = pValue; }
	inline void setPrimaryChromaticDamage(const u32 pValue) { mPrimaryChromaticDamage = pValue; }
	inline void setPrimaryPrismaticDamage(const u32 pValue) { mPrimaryPrismaticDamage = pValue; }
	inline void setPrimaryPhysicalDamage(const u32 pValue) { mPrimaryPhysicalDamage = pValue; }
	inline void setPrimaryCorruptionDamage(const u32 pValue) { mPrimaryCorruptionDamage = pValue; }

private:

	void _onCopy();

	bool mDestroy = false;

	String mName = "";
	String mSurname = "";
	String mTitle = "";
	String mSuffix = "";

	// Primary Damage
	u32 mPrimaryDamage = 0;
	// Primary Elemental Damage
	u32 mPrimaryMagicDamage = 0;
	u32 mPriamryFireDamage = 0;
	u32 mPrimaryColdDamage = 0;
	u32 mPrimaryPoisonDamage = 0;
	u32 mPrimaryDiseaseDamage = 0;
	u32 mPrimaryChromaticDamage = 0;
	u32 mPrimaryPrismaticDamage = 0;
	u32 mPrimaryPhysicalDamage = 0;
	u32 mPrimaryCorruptionDamage = 0;

	u8 mPrimaryDamageType = DamageType::HandtoHand;
	u8 mSecondaryDamageType = DamageType::HandtoHand;
	u8 mRangeDamageType = DamageType::Archery;

	u8 mPrimaryAttackAnimation = Animation::ANIM_HAND2HAND;
	u8 mSecondaryAttackAnimation = Animation::ANIM_HAND2HAND;
	u8 mRangeAttackAnimation = Animation::ANIM_SHOOTBOW;
	
	bool mIsInvulnerable = false;
	bool mIsStunImmune = false;
	bool mIsStunned = false;
	bool mIsDead = false;
	bool mIsCasting = false;
	CastingState mCastingState;

	i32 mCurrentHP = 50;
	i32 mMaximumHP = 50;
	i32 mCurrentMana = 100;
	i32 mMaximumMana = 100;
	i32 mCurrentEndurance = 100;
	i32 mMaximumEndurance = 100;

	std::list<u32> mNimbuses;

	float mVisibleRange = 300.0f;
	std::list<Character*> mVisibleTo; // Characters who can see this Actor.
	std::bitset<65535> mVisibleToSpawnID;

	Actor* mOwner = nullptr;
	Actor* mTarget = nullptr; // Current target.
	std::list<Actor*> mTargeters; // Actors currently targeting this Actor.

	LootController* mLootController = nullptr;

	inline void _setName(const char* pName) { strncpy_s(mActorData.mName, pName, Limits::Character::MAX_NAME_LENGTH); }
	inline void _setSurname(const char* pSurname) { strncpy_s(mActorData.mSurname, pSurname, Limits::Character::MAX_LAST_NAME_LENGTH); }
	inline void _setTitle(const char* pTitle) { strncpy_s(mActorData.mTitle, pTitle, Limits::Character::MAX_TITLE_LENGTH); }
	inline void _setSuffix(const char* pSuffix) { strncpy_s(mActorData.mSuffix, pSuffix, Limits::Character::MAX_SUFFIX_LENGTH); }

	const bool sendsEquipment() const;
};