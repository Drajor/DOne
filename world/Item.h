#pragma once

#include "Constants.h"
#include "ItemData.h"
#include "Utility.h"

class DynamicStructure;
class Item {
public:
	Item(ItemData* pItemData);
	inline void setName(const String& pName) { mName = pName; strcpy(mItemData->mItemName, pName.c_str()); }
	inline const String& getName() const { return mName; }
	inline void setIDFile(const String& pValue) { memset(mItemData->mIDFile, 0, sizeof(mItemData->mIDFile)); strcpy(mItemData->mIDFile, pValue.c_str()); }
	inline const uint32 getSlot() const { return mSlot; }
	inline void setSlot(const uint32 pSlot) { mSlot = pSlot; }
	Item* getAugment(const uint8 pSlot) const;
	Item* getContents(const uint8 pSlot) const;

	const uint32 getDataSize() const;
	const bool copyData(Utility::DynamicStructure& pStructure);
	uint32 getSubItems() const;

	inline const uint32 getStackSize() const { return mItemData->mStacks; }
	inline const uint32 getMerchantPrice() const { return mItemData->mMerchantPrice; }
	inline const uint32 getMerchantSlot() const { return mItemData->mMerchantSlot; }
	inline const uint32 getInstanceID() const { return mItemData->mInstanceID; }
	inline const uint32 getLastCastTime() const { return mItemData->mLastCastTime; }
	inline const uint32 getCharges() const { return mItemData->mCharges; }
	inline const uint32 getAttuned() const { return mItemData->mAttuned; }
	inline const uint8 getItemClass() const { return mItemData->mItemClass; }
	inline const uint32 getID() const { return mItemData->mID; }
	inline const uint8 getWeight() const { return mItemData->mWeight; }
	inline const uint8 isTemporary() const { return mItemData->mTemporary; }
	inline const uint8 isNoDrop() const { return mItemData->mNoDrop; }
	inline const uint8 isAttunable() const { return mItemData->mAttunable; }
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
	inline const int32 getCastTime() const { return mItemData->mCastTime; }
	inline const uint32 getReqLevel() const { return mItemData->mReqLevel; }
	inline const uint32 getRecLevel() const { return mItemData->mRecLevel; }
	inline const uint32 getRecSkill() const { return mItemData->mRecSkill; }
	inline const uint32 getBardType() const { return mItemData->mBardType; }
	inline const int32 getBardValue() const { return mItemData->mBardValue; }
	inline const uint8 getLight() const { return mItemData->mLight; }
	inline const uint8 getDelay() const { return mItemData->mDelay; }
	inline const uint8 getElementalDamageType() const { return mItemData->mElementalDamageType; }
	inline const uint8 getElementalDamageAmount() const { return mItemData->mElementalDamageAmount; }
	inline const uint8 getRange() const { return mItemData->mRange; }
	inline const uint32 getDamage() const { return mItemData->mDamage; }
	inline const uint32 getColour() const { return mItemData->mColour; }
	inline const uint8 getItemType() const { return mItemData->mItemType; }
	inline const uint32 getMaterial() const { return mItemData->mMaterial; }
	inline const uint32 getEliteMaterial() const { return mItemData->mEliteMaterial; }
	inline const float getSellRate() const { return mItemData->mSellRate; }
	inline const int32 getCombatEffects() const { return mItemData->mCombatEffects; }
	inline const int32 getShielding() const { return mItemData->mShielding; }
	inline const int32 getStunResist() const { return mItemData->mStunResist; }
	inline const int32 getStrikeThrough() const { return mItemData->mStrikeThrough; }
	inline const int32 getExtraDamageSkill() const { return mItemData->mExtraDamageSkill; }
	inline const int32 getExtraDamageAmount() const { return mItemData->mExtraDamageAmount; }
	inline const int32 getSpellShield() const { return mItemData->mSpellShield; }
	inline const int32 getAvoidance() const { return mItemData->mAvoidance; }
	inline const int32 getAccuracy() const { return mItemData->mAccuracy; }
	// Faction Modifiers
	inline const uint32 getAugType() const { return mItemData->mAugType; }
	inline const uint32 getAugRestrict() const { return mItemData->mAugRestrict; }
	// Augmentation Slots
	inline const uint32 getLDONPointType() const { return mItemData->mLDONPointType; }
	inline const uint32 getLDONTheme() const { return mItemData->mLDONTheme; }
	inline const uint32 getLDONPrice() const { return mItemData->mLDONPrice; }
	inline const uint32 getLDONSellBackPrice() const { return mItemData->mLDONSellBackPrice; }
	inline const uint32 getLDONSold() const { return mItemData->mLDONSold; }
	inline const uint8 getContainerType() const { return mItemData->mContainerType; }
	inline const uint8 getContainerSlots() const { return mItemData->mContainerSlots; }
	inline const uint8 getContainerSize() const { return mItemData->mContainerSize; }
	inline const uint8 getContainerWR() const { return mItemData->mContainerWR; }

	inline void setStacks(const uint32 pValue) { mItemData->mStacks = pValue; }
	inline void setMerchantPrice(const uint32 pValue) { mItemData->mMerchantPrice = pValue; }
	inline void setMerchantSlot(const uint32 pValue) { mItemData->mMerchantSlot = pValue; }
	inline void setSerial(const uint32 pValue) { mItemData->mInstanceID = pValue; }
	inline void setLastCastTime(const uint32 pValue) { mItemData->mLastCastTime = pValue; }
	inline void setCharges(const uint32 pValue) { mItemData->mCharges = pValue; }
	inline void setAttuned(const uint32 pValue) { mItemData->mAttuned = pValue; }
	inline void setItemClass(const uint8 pValue) { mItemData->mItemClass = pValue; }
	inline void setID(const uint32 pValue) { mItemData->mID = pValue; }
	inline void setWeight(const uint8 pValue) { mItemData->mWeight = pValue; }
	inline void setIsTemporary(const bool pValue) { mItemData->mTemporary = pValue ? 1 : 0; }
	inline void setIsNoDrop(const bool pValue) { mItemData->mNoDrop = pValue ? 1 : 0; }
	inline void setIsAttunable(const bool pValue) { mItemData->mAttunable = pValue ? 1 : 0; }
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
	inline void setCastTime(const int32 pValue) { mItemData->mCastTime = pValue; }
	inline void setReqLevel(const uint32 pValue) { mItemData->mReqLevel = pValue; }
	inline void setRecLevel(const uint32 pValue) { mItemData->mRecLevel = pValue; }
	inline void setRecSkill(const uint32 pValue) { mItemData->mRecSkill = pValue; }
	inline void setBardType(const uint32 pValue) { mItemData->mBardType = pValue; }
	inline void setBardValue(const int32 pValue) { mItemData->mBardValue = pValue; }
	inline void setLight(const uint8 pValue) { mItemData->mLight = pValue; }
	inline void setDelay(const uint8 pValue) { mItemData->mDelay = pValue; }
	inline void setElementalDamasetype(const uint8 pValue) { mItemData->mElementalDamageType = pValue; }
	inline void setElementalDamageAmount(const uint8 pValue) { mItemData->mElementalDamageAmount = pValue; }
	inline void setRange(const uint8 pValue) { mItemData->mRange = pValue; }
	inline void setDamage(const uint32 pValue) { mItemData->mDamage = pValue; }
	inline void setColour(const uint32 pValue) { mItemData->mColour = pValue; }
	inline void setItemType(const uint8 pValue) { mItemData->mItemType = pValue; }
	inline void setMaterial(const uint32 pValue) { mItemData->mMaterial = pValue; }
	inline void setEliteMaterial(const uint32 pValue) { mItemData->mEliteMaterial = pValue; }
	inline void setSellRate(const float pValue) { mItemData->mSellRate = pValue; }
	inline void setCombatEffects(const int32 pValue) { mItemData->mCombatEffects = pValue; }
	inline void setShielding(const int32 pValue) { mItemData->mShielding = pValue; }
	inline void setStunResist(const int32 pValue) { mItemData->mStunResist = pValue; }
	inline void setStrikeThrough(const int32 pValue) { mItemData->mStrikeThrough = pValue; }
	inline void setExtraDamageSkill(const int32 pValue) { mItemData->mExtraDamageSkill = pValue; }
	inline void setExtraDamageAmount(const int32 pValue) { mItemData->mExtraDamageAmount = pValue; }
	inline void setSpellShield(const int32 pValue) { mItemData->mSpellShield = pValue; }
	inline void setAvoidance(const int32 pValue) { mItemData->mAvoidance = pValue; }
	inline void setAccuracy(const int32 pValue) { mItemData->mAccuracy = pValue; }
	// Faction Modifiers
	inline void setAugType(const uint32 pValue) const { mItemData->mAugType = pValue; }
	inline void setAugRestrict(const uint32 pValue) const { mItemData->mAugRestrict = pValue; }
	// Augmentation Slots
	inline void setLDONPointType(const uint32 pValue) { mItemData->mLDONPointType = pValue; }
	inline void setLDONTheme(const uint32 pValue) { mItemData->mLDONTheme = pValue; }
	inline void setLDONPrice(const uint32 pValue) { mItemData->mLDONPrice = pValue; }
	inline void setLDONSellBackPrice(const uint32 pValue) { mItemData->mLDONSellBackPrice = pValue; }
	inline void setLDONSold(const uint32 pValue) { mItemData->mLDONSold = pValue; }
	inline void setContainerType(const uint8 pValue) { mItemData->mContainerType = pValue; }
	inline void setContainerSlots(const uint8 pValue) { mItemData->mContainerSlots = pValue; }
	inline void setContainerSize(const uint8 pValue) { mItemData->mContainerSize = pValue; }
	inline void setContainerWR(const uint8 pValue) { mItemData->mContainerWR = pValue; }

	inline void setMaxStacks(const uint32 pValue) { mItemData->mMaxStacks = pValue; }
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