#pragma once

class Zone;

#include "Constants.h"
#include "Payload.h"
#include "Vector3.h"

class Actor {
public:
	Actor() : mTarget(nullptr) {};
	virtual ~Actor() {};

	inline virtual const bool isCharacter() const { return false; }
	inline virtual bool isNPC() const { return false; }
	inline void setTarget(Actor* pActor) { mTarget = pActor; }
	inline Actor* getTarget() const { return mTarget; }
	inline const bool hasTarget() const { return mTarget != nullptr; }
	template <typename T>
	inline static T cast(Actor* pActor) {
		return static_cast<T>(pActor);
	}
	inline const Vector3& getPosition() const { return mPosition; }
	inline const float distanceTo(const Actor* pActor) {
		return mPosition.distance(pActor->mPosition);
	}
	inline const float squareDistanceTo(const Actor* pActor) {
		return mPosition.squareDistance(pActor->mPosition);
	}

	inline const Payload::SpawnData* getSpawnData() const { return &mSpawnData; }

	inline const SpawnID getSpawnID() const { return mSpawnData.mSpawnID; }
	inline void setSpawnID(const SpawnID pSpawnID) { mSpawnData.mSpawnID = pSpawnID; }

	// Name
	// Suffix
	// Title
	// Last Name

	// Visual
	inline const uint8 getFaceStyle() const { return mSpawnData.mFaceStyle; }
	inline const uint8 getLeftEyeColour() const { return mSpawnData.mLeftEyeColour; }
	inline const uint8 getRightEyeColour() const { return mSpawnData.mRightEyeColour; }
	inline const uint8 getHairStyle() const { return mSpawnData.mHairColor; }
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

	// TODO Position / Deltas.

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

	inline const uint32 getMaterial(const MaterialSlot pMaterialSlot) const {
		// TODO: Guard this.
		return mSpawnData.mEquipmentMaterials[pMaterialSlot];
	}
	inline void setMaterial(const MaterialSlot pMaterialSlot, const uint32 pMaterial) {
		// TODO: Guard this.
		mSpawnData.mEquipmentMaterials[pMaterialSlot] = pMaterial;
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

	inline const Colour getColour(const MaterialSlot pMaterialSlot) const {
		// TODO: Guard this.
		return mSpawnData.mEquipmentColours[pMaterialSlot];
	}
	inline void setColour(const MaterialSlot pMaterialSlot, const Colour pColour) {
		// TODO: Guard this.
		mSpawnData.mEquipmentColours[pMaterialSlot] = pColour;
	}

	inline const bool isLFG() const { return mSpawnData.mIsLFG == 1; }
	inline void setIsLFG(const bool pIsLFG) { mSpawnData.mIsLFG = pIsLFG ? 1 : 0; }

protected:
	Vector3 mPosition;
	Actor* mTarget;

	// mSpawnData is sent directly 
	Payload::SpawnData mSpawnData;
private:
};