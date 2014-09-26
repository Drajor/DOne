#pragma once

#include "Constants.h"
#include "ItemData.h"
#include "Utility.h"

class DynamicStructure;
class Item {
public:
	Item(ItemData* pItemData);
	inline const String& getName() const { return mName; }
	inline const uint32 getSlot() const { return mSlot; }
	inline void setSlot(const uint32 pSlot) { mSlot = pSlot; }
	Item* getAugment(const uint8 pSlot) const;
	Item* getContents(const uint8 pSlot) const;

	const uint32 getDataSize() const;
	const bool copyData(Utility::DynamicStructure& pStructure);
	uint32 getSubItems() const;

	inline const uint32 getStackSize() const { return mItemData->mStackSize; }
	inline const uint32 getMerchantPrice() const { return mItemData->mMerchantPrice; }
	inline const uint32 getMerchantSlot() const { return mItemData->mMerchantSlot; }
	inline const uint32 getInstanceID() const { return mItemData->mInstanceID; }
	inline const uint32 getLastCastTime() const { return mItemData->mLastCastTime; }
	inline const uint32 getCharges() const { return mItemData->mCharges; }
	inline const uint32 getAttuned() const { return mItemData->mAttuned; }
	inline const uint8 getItemClass() const { return mItemData->mItemClass; }
	inline const uint32 getID() const { return mItemData->mID; }
	inline const uint8 getWeight() const { return mItemData->mWeight; }
	inline const uint8 isNoRent() const { return mItemData->mNoRent; }
	inline const uint8 isNoDrop() const { return mItemData->mNoDrop; }
	inline const uint8 isAttuneable() const { return mItemData->mAttuneable; }
	inline const uint8 getSize() const { return mItemData->mSize; }
	inline const uint32 getSlots() const { return mItemData->mSlots; }
	inline const uint32 getPrice() const { return mItemData->mPrice; }
	inline const uint32 getIcon() const { return mItemData->mIcon; }
	inline const uint32 getBenefitFlag() const { return mItemData->mBenefitFlag; }
	inline const uint8 isTradeskillsItem() const { return mItemData->mTradeSkills; }
	inline const int8 getColdResist() const { return mItemData->mColdResist; }
	inline const int8 getDiseaseResist() const { return mItemData->mDieaseResist; }
	inline const int8 getPoisonResist() const { return mItemData->mPoisonResist; }
	inline const int8 getMagicResist() const { return mItemData->mMagicResist; }
	inline const int8 getFireResist() const { return mItemData->mFireResist; }
	inline const int8 getSVCorruption() const { return mItemData->mSVCorruption; }
	inline const int8 getStrength() const { return mItemData->mStrength; }
	inline const int8 getStamina() const { return mItemData->mStamina; }
	inline const int8 getAgility() const { return mItemData->mAgility; }
	inline const int8 getDexterity() const { return mItemData->mDexterity; }
	inline const int8 getCharisma() const { return mItemData->mCharisma; }
	inline const int8 getIntelligence() const { return mItemData->mIntelligence; }
	inline const uint8 getWisdom() const { return mItemData->mWisdom; }
	inline const int32 getHealth() const { return mItemData->mHealth; }
	inline const int32 getMana() const { return mItemData->mMana; }
	inline const uint32 getEndurance() const { return mItemData->mEndurance; }
	inline const int32 getArmorClass() const { return mItemData->mArmorClass; }
	inline const int32 getHealthRegen() const { return mItemData->mHealthRegen; }
	inline const int32 getManaRegen() const { return mItemData->mManaRegen; }
	inline const int32 getEnduranceRegen() const { return mItemData->mEnduranceRegen; }
	inline const uint32 getClasses() const { return mItemData->mClasses; }
	inline const uint32 getRaces() const { return mItemData->mRaces; }
	inline const uint32 getDeity() const { return mItemData->mDeity; }
	inline const int32 getSkillModifierValue() const { return mItemData->mSkillModifierValue; }
	inline const uint32 getSkillModifierType() const { return mItemData->mSkillModifierType; }
	inline const uint32 getBaneDamageRace() const { return mItemData->mBaneDamageRace; }
	inline const uint32 getBaneDamageBodyType() const { return mItemData->mBaneDamageBodyType; }
	inline const uint32 getBaneDamageRaceAmount() const { return mItemData->mBaneDamageRaceAmount; }
	inline const int32 getBaneDamageAmount() const { return mItemData->mBaneDamageAmount; }
	inline const uint8 isMagic() const { return mItemData->mMagic; }

	inline const uint32 getReqLevel() const { return mItemData->mReqLevel; }
	inline const uint32 getRecLevel() const { return mItemData->mRecLevel; }

	inline void setStackSize(const uint32 pValue) { mItemData->mStackSize = pValue; }
	inline void setMerchantPrice(const uint32 pValue) { mItemData->mMerchantPrice = pValue; }
	inline void setMerchantSlot(const uint32 pValue) { mItemData->mMerchantSlot = pValue; }
	inline void setInstanceID(const uint32 pValue) { mItemData->mInstanceID = pValue; }
	inline void setLastCastTime(const uint32 pValue) { mItemData->mLastCastTime = pValue; }
	inline void setCharges(const uint32 pValue) { mItemData->mCharges = pValue; }
	inline void setAttuned(const uint32 pValue) { mItemData->mAttuned = pValue; }
	inline void setItemClass(const uint8 pValue) { mItemData->mItemClass = pValue; }
	inline void setID(const uint32 pValue) { mItemData->mID = pValue; }
	inline void setWeight(const uint8 pValue) { mItemData->mWeight = pValue; }
	inline void setIsNoRent(const bool pValue) { mItemData->mNoRent = pValue ? 1 : 0; }
	inline void setIsNoDrop(const bool pValue) { mItemData->mNoDrop = pValue ? 1 : 0; }
	inline void setIsAttuneable(const bool pValue) { mItemData->mAttuneable = pValue ? 1 : 0; }
	inline void setSize(const uint8 pValue) { mItemData->mSize = pValue; }
	inline void setSlots(const uint32 pValue) { mItemData->mSlots = pValue; }
	inline void setPrice(const uint32 pValue) { mItemData->mPrice = pValue; }
	inline void setIcon(const uint32 pValue) { mItemData->mIcon = pValue; }
	inline void setBenefitFlag(const uint32 pValue) { mItemData->mBenefitFlag = pValue; }
	inline void setIsTradeskillsItem(const bool pValue) { mItemData->mTradeSkills = pValue ? 1 : 0; }
	inline void setColdResist(const int8 pValue) { mItemData->mColdResist = pValue; }
	inline void setDiseaseResist(const int8 pValue) { mItemData->mDieaseResist = pValue; }
	inline void setPoisonResist(const int8 pValue) { mItemData->mPoisonResist = pValue; }
	inline void setMagicResist(const int8 pValue) { mItemData->mMagicResist = pValue; }
	inline void setFireResist(const int8 pValue) { mItemData->mFireResist = pValue; }
	inline void setSVCorruption(const int8 pValue) { mItemData->mSVCorruption = pValue; }
	inline void setStrength(const int8 pValue) { mItemData->mStrength = pValue; }
	inline void setStamina(const int8 pValue) { mItemData->mStamina = pValue; }
	inline void setAgility(const int8 pValue) { mItemData->mAgility = pValue; }
	inline void setDexterity(const int8 pValue) { mItemData->mDexterity = pValue; }
	inline void setCharisma(const int8 pValue) { mItemData->mCharisma = pValue; }
	inline void setIntelligence(const int8 pValue) { mItemData->mIntelligence = pValue; }
	inline void setWisdom(const int8 pValue) { mItemData->mWisdom = pValue; }
	inline void setHealth(const int32 pValue) { mItemData->mHealth = pValue; }
	inline void setMana(const int32 pValue) { mItemData->mMana = pValue; }
	inline void setEndurance(const uint32 pValue) { mItemData->mEndurance = pValue; }
	inline void setArmorClass(const int32 pValue) { mItemData->mArmorClass = pValue; }
	inline void setHealthRegen(const int32 pValue) { mItemData->mHealthRegen = pValue; }
	inline void setManaRegen(const int32 pValue) { mItemData->mManaRegen = pValue; }
	inline void setEnduranceRegen(const int32 pValue) { mItemData->mEnduranceRegen = pValue; }
	inline void setClasses(const uint32 pValue) { mItemData->mClasses = pValue; }
	inline void setRaces(const uint32 pValue) { mItemData->mRaces = pValue; }
	inline void setDeity(const uint32 pValue) { mItemData->mDeity = pValue; }
	inline void setSkillModifierValue(const int32 pValue) { mItemData->mSkillModifierValue = pValue; }
	inline void setSkillModifierType(const uint32 pType) { mItemData->mSkillModifierType = pType; }
	inline void setBaneDamageRace(const uint32 pValue) { mItemData->mBaneDamageRace = pValue; }
	inline void setBaneDamageBodyType(const uint32 pValue) { mItemData->mBaneDamageBodyType = pValue; }
	inline void setBaneDamageRaceAmount(const uint32 pValue) { mItemData->mBaneDamageRaceAmount = pValue; }
	inline void setBaneDamageAmount(const int32 pValue) { mItemData->mBaneDamageAmount = pValue; }
	inline void setIsMagic(const bool pValue) { mItemData->mMagic = pValue ? 1 : 0; }

	inline void setReqLevel(const uint32 pValue) { mItemData->mReqLevel = pValue; }
	inline void setRecLevel(const uint32 pValue) { mItemData->mRecLevel = pValue; }
private:
	const uint32 _getDataSize() const;
	void _onCopy();

	String mName;
	Item* mAugments[5];
	Item* mContents[10];
	ItemData* mItemData = nullptr;

	// ItemData Overrides
	uint32 mSlot = 0;
};