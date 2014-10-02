#pragma once

#include "Constants.h"
#include "ItemData.h"
#include "Utility.h"

class DynamicStructure;
class Item {
public:
	Item(ItemData* pItemData);
	
	inline const String getName() const { return String(mItemData->mName); }
	inline const String getLore() const { return String(mItemData->mLore); }
	inline const String getIDFile() const { return String(mItemData->mIDFile); }
	inline const String getCharmFile() const { return String(mItemData->mCharmFile); }
	inline const String getFileName() const { return String(mItemData->mFileName); }
	inline const String getClickName() const { return String(mItemData->mClickName); }
	inline const String getProcName() const { return String(mItemData->mProcName); }
	inline const String getWornName() const { return String(mItemData->mWornName); }
	inline const String getFocusName() const { return String(mItemData->mFocusName); }
	inline const String getScrollName() const { return String(mItemData->mScrollName); }
	inline const String getBardName() const { return String(mItemData->mBardName); }

	inline void setName(const String& pValue) { memset(mItemData->mName, 0, sizeof(mItemData->mName)); strcpy(mItemData->mName, pValue.c_str()); }
	inline void setLore(const String& pValue) { memset(mItemData->mLore, 0, sizeof(mItemData->mLore)); strcpy(mItemData->mLore, pValue.c_str()); }
	inline void setIDFile(const String& pValue) { memset(mItemData->mIDFile, 0, sizeof(mItemData->mIDFile)); strcpy(mItemData->mIDFile, pValue.c_str()); }
	inline void setCharmFile(const String& pValue) { memset(mItemData->mCharmFile, 0, sizeof(mItemData->mCharmFile)); strcpy(mItemData->mCharmFile, pValue.c_str()); }
	inline void setFileName(const String& pValue) { memset(mItemData->mFileName, 0, sizeof(mItemData->mFileName)); strcpy(mItemData->mFileName, pValue.c_str()); }
	inline void setClickName(const String& pValue) { memset(mItemData->mClickName, 0, sizeof(mItemData->mClickName)); strcpy(mItemData->mClickName, pValue.c_str()); }
	inline void setProcName(const String& pValue) { memset(mItemData->mProcName, 0, sizeof(mItemData->mProcName)); strcpy(mItemData->mProcName, pValue.c_str()); }
	inline void setWornName(const String& pValue) { memset(mItemData->mWornName, 0, sizeof(mItemData->mWornName)); strcpy(mItemData->mWornName, pValue.c_str()); }
	inline void setFocusName(const String& pValue) { memset(mItemData->mFocusName, 0, sizeof(mItemData->mFocusName)); strcpy(mItemData->mFocusName, pValue.c_str()); }
	inline void setScrollName(const String& pValue) { memset(mItemData->mScrollName, 0, sizeof(mItemData->mScrollName)); strcpy(mItemData->mScrollName, pValue.c_str()); }
	inline void setBardName(const String& pValue) { memset(mItemData->mBardName, 0, sizeof(mItemData->mBardName)); strcpy(mItemData->mBardName, pValue.c_str()); }
	
	inline const uint32 getSlot() const { return mSlot; }
	inline void setSlot(const uint32 pSlot) { mSlot = pSlot; }
	inline const uint32 getStacks() const { return mStacks; }
	inline void setStacks(const uint32 pStacks) { mStacks = pStacks; }
	inline const bool isAttuned() { return mAttuned; }
	inline void setIsAttuned(const bool pAttuned) { mAttuned = pAttuned; }

	Item* getAugment(const uint8 pSlot) const;
	Item* getContents(const uint8 pSlot) const;

	const uint32 getDataSize() const;
	const unsigned char* copyData(uint32& pSize);
	const bool copyData(Utility::DynamicStructure& pStructure);
	uint32 getSubItems() const;

	const bool isContainer() const { return getItemClass() == ItemClass::Container; }
	Item* findFirst(const uint8 pItemType);

	inline const uint32 getMerchantPrice() const { return mItemData->mMerchantPrice; }
	inline const uint32 getMerchantSlot() const { return mItemData->mMerchantSlot; }
	inline const uint32 getInstanceID() const { return mItemData->mInstanceID; }
	inline const uint32 getLastCastTime() const { return mItemData->mLastCastTime; }
	inline const uint32 getCharges() const { return mItemData->mCharges; }
	inline const uint32 getPower() const { return mItemData->mPower; }
	inline const bool isEvolvingItem() const { return mItemData->mIsEvolvingItem == 1; }
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
	inline const int32 _getColdResist() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getColdResist(); return getColdResist() + count; }
	inline const int8 getDiseaseResist() const { return mItemData->mDieaseResist; }
	inline const int32 _getDiseaseResist() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getDiseaseResist(); return getDiseaseResist() + count; }
	inline const int8 getPoisonResist() const { return mItemData->mPoisonResist; }
	inline const int32 _getPoisonResist() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getPoisonResist(); return getPoisonResist() + count; }
	inline const int8 getMagicResist() const { return mItemData->mMagicResist; }
	inline const int32 _getMagicResist() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getMagicResist(); return getMagicResist() + count; }
	inline const int8 getFireResist() const { return mItemData->mFireResist; }
	inline const int32 _getFireResist() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getFireResist(); return getFireResist() + count; }
	inline const int8 getCorruptionResist() const { return mItemData->mCorruptionResist; }
	inline const int32 _getCorruptionResist() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getCorruptionResist(); return getCorruptionResist() + count; }
	inline const int8 getStrength() const { return mItemData->mStrength; }
	inline const int32 _getStrength() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getStrength(); return getStrength() + count; }
	inline const int8 getStamina() const { return mItemData->mStamina; }
	inline const int32 _getStamina() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getStamina(); return getStamina() + count; }
	inline const int8 getAgility() const { return mItemData->mAgility; }
	inline const int32 _getAgility() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getAgility(); return getAgility() + count; }
	inline const int8 getDexterity() const { return mItemData->mDexterity; }
	inline const int32 _getDexterity() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getDexterity(); return getDexterity() + count; }
	inline const int8 getCharisma() const { return mItemData->mCharisma; }
	inline const int32 _getCharisma() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getCharisma(); return getCharisma() + count; }
	inline const int8 getIntelligence() const { return mItemData->mIntelligence; }
	inline const int32 _getIntelligence() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getIntelligence(); return getIntelligence() + count; }
	inline const uint8 getWisdom() const { return mItemData->mWisdom; }
	inline const int32 _getWisdom() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getWisdom(); return getWisdom() + count; }
	inline const int32 getHealth() const { return mItemData->mHealth; }
	inline const int32 _getHealth() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getHealth(); return getHealth() + count; }
	inline const int32 getMana() const { return mItemData->mMana; }
	inline const int32 _getMana() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getMana(); return getMana() + count; }
	inline const uint32 getEndurance() const { return mItemData->mEndurance; }
	inline const int32 _getEndurance() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getEndurance(); return getEndurance() + count; }
	inline const int32 getArmorClass() const { return mItemData->mArmorClass; }
	inline const int32 _getArmorClass() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getArmorClass(); return getArmorClass() + count; }
	inline const int32 getHealthRegen() const { return mItemData->mHealthRegen; }
	inline const int32 _getHealthRegen() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getHealthRegen(); return getHealthRegen() + count; }
	inline const int32 getManaRegen() const { return mItemData->mManaRegen; }
	inline const int32 _getManaRegen() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getManaRegen(); return getManaRegen() + count; }
	inline const int32 getEnduranceRegen() const { return mItemData->mEnduranceRegen; }
	inline const int32 _getEnduranceRegen() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getEnduranceRegen(); return getEnduranceRegen() + count; }
	inline const uint32 getClasses() const { return mItemData->mClasses; }
	inline const uint32 getRaces() const { return mItemData->mRaces; }
	inline const uint32 getDeity() const { return mItemData->mDeity; }
	inline const int32 getSkillModAmount() const { return mItemData->mSkillModAmount; }
	inline const uint32 getSkillMod() const { return mItemData->mSkillMod; }
	inline const uint32 getBaneRace() const { return mItemData->mBaneDamageRace; }
	inline const uint32 getBaneBodyType() const { return mItemData->mBaneDamageBodyType; }
	inline const uint32 getBaneRaceAmount() const { return mItemData->mBaneDamageRaceAmount; }
	inline const int32 getBaneBodyTypeAmount() const { return mItemData->mBaneDamageBodyTypeAmount; }
	inline const uint8 isMagic() const { return mItemData->mMagic; }
	inline const uint32 getFoodSize() const { return mItemData->mFoodSize; }
	inline const uint32 getDrinkSize() const { return mItemData->mDrinkSize; }
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
	inline const int32 _getDamage() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getDamage(); return getDamage() + count; }
	inline const uint32 getColour() const { return mItemData->mColour; }
	inline const uint8 getItemType() const { return mItemData->mItemType; }
	inline const uint32 getMaterial() const { return mItemData->mMaterial; }
	inline const uint32 getEliteMaterial() const { return mItemData->mEliteMaterial; }
	inline const float getSellRate() const { return mItemData->mSellRate; }
	inline const int32 getCombatEffects() const { return mItemData->mCombatEffects; }
	inline const int32 _getCombatEffects() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getCombatEffects(); return getCombatEffects() + count; }
	inline const int32 getShielding() const { return mItemData->mShielding; }
	inline const int32 _getShielding() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getShielding(); return getShielding() + count; }
	inline const int32 getStunResist() const { return mItemData->mStunResist; }
	inline const int32 _getStunResist() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getStunResist(); return getStunResist() + count; }
	inline const int32 getStrikeThrough() const { return mItemData->mStrikeThrough; }
	inline const int32 _getStrikeThrough() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getStrikeThrough(); return getStrikeThrough() + count; }
	inline const int32 getSkillDamageMod() const { return mItemData->mSkillDamageMod; }
	inline const int32 getSkillDamageModAmount() const { return mItemData->mSkillDamageModAmount; }
	inline const int32 getSpellShield() const { return mItemData->mSpellShield; }
	inline const int32 _getSpellShield() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getSpellShield(); return getSpellShield() + count; }
	inline const int32 getAvoidance() const { return mItemData->mAvoidance; }
	inline const int32 _getAvoidance() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getAvoidance(); return getAvoidance() + count; }
	inline const int32 getAccuracy() const { return mItemData->mAccuracy; }
	inline const int32 _getAccuracy() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getAccuracy(); return getAccuracy() + count; }
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

	inline const uint8 getBook() const { return mItemData->mBook; }
	inline const uint8 getBookType() const { return mItemData->mBookType; }
	inline const int32 getLoreGroup() const { return mItemData->mLoreGroup; }
	inline const uint8 getIsArtifact() const { return mItemData->mArtifact; }
	inline const uint8 getIsSummoned() const { return mItemData->mSummoned; }
	inline const uint32 getFavor() const { return mItemData->mFavor; }
	inline const uint8 getFVNoDrop() const { return mItemData->mFVNoDrop; }
	inline const int32 getDoTShield() const { return mItemData->mDoTShield; }
	inline const int32 getAttack() const { return mItemData->mAttack; }
	inline const int32 getHaste() const { return mItemData->mHaste; }
	inline const int32 getDamageShield() const { return mItemData->mDamageShield; }
	inline const uint32 getGuildFavor() const { return mItemData->mGuildFavor; }
	// TODO: augdistil
	inline const uint8 getIsNoPet() const { return mItemData->mNoPet; }
	// TODO: potion belt
	inline const uint32 getMaxStacks() const { return mItemData->mMaxStacks; }
	inline const uint8 getIsNoTransfer() const { return mItemData->mNoTransfer; }
	// TODO: expendablearrow
	// TODO: Effects
	inline const uint32 getScriptFileID() const { return mItemData->mScriptFileID; }
	inline const uint8 getIsQuest() const { return mItemData->mQuest; }
	inline const uint32 getMaxPower() const { return mItemData->mMaxPower; }
	inline const uint32 getPurity() const { return mItemData->mPurity; }
	// TODO: Power
	// TODO: Purity
	inline const uint32 getBackstabDamage() const { return mItemData->mBackstabDamage; }
	inline const uint32 getDamageShieldMitigation() const { return mItemData->mDamageShieldMitigation; }
	inline const int32 getHeroicStrength() const { return mItemData->mHeroicStrength; }
	inline const int32 _getHeroicStrength() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getHeroicStrength(); return getHeroicStrength() + count; }
	inline const int32 getHeroicIntelligence() const { return mItemData->mHeroicIntelligence; }
	inline const int32 _getHeroicIntelligence() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getHeroicIntelligence(); return getHeroicIntelligence() + count; }
	inline const int32 getHeroicWisdom() const { return mItemData->mHeroicWisdom; }
	inline const int32 _getHeroicWisdom() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getHeroicWisdom(); return getHeroicWisdom() + count; }
	inline const int32 getHeroicAgility() const { return mItemData->mHeroicAgility; }
	inline const int32 _getHeroicAgility() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getHeroicAgility(); return getHeroicAgility() + count; }
	inline const int32 getHeroicDexterity() const { return mItemData->mHeroicDexterity; }
	inline const int32 _getHeroicDexterity() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getHeroicDexterity(); return getHeroicDexterity() + count; }
	inline const int32 getHeroicStamina() const { return mItemData->mHeroicStamina; }
	inline const int32 _getHeroicStamina() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getHeroicStamina(); return getHeroicStamina() + count; }
	inline const int32 getHeroicCharisma() const { return mItemData->mHeroicCharisma; }
	inline const int32 _getHeroicCharisma() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getHeroicCharisma(); return getHeroicCharisma() + count; }
	inline const int32 getHeroicMagicResist() const { return mItemData->mHeroicMagicResist; }
	inline const int32 _getHeroicMagicResist() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getHeroicMagicResist(); return getHeroicMagicResist() + count; }
	inline const int32 getHeroicFireResist() const { return mItemData->mHeroicFireResist; }
	inline const int32 _getHeroicFireResist() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getHeroicFireResist(); return getHeroicFireResist() + count; }
	inline const int32 getHeroicColdResist() const { return mItemData->mHeroicColdResist; }
	inline const int32 _getHeroicColdResist() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getHeroicColdResist(); return getHeroicColdResist() + count; }
	inline const int32 getHeroicDiseaseResist() const { return mItemData->mHeroicDiseaseResist; }
	inline const int32 _getHeroicDiseaseResist() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getHeroicDiseaseResist(); return getHeroicDiseaseResist() + count; }
	inline const int32 getHeroicPoisonResist() const { return mItemData->mHeroicPoisonResist; }
	inline const int32 _getHeroicPoisonResist() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getHeroicPoisonResist(); return getHeroicPoisonResist() + count; }
	inline const int32 getHeroicCorruptionResist() const { return mItemData->mHeroicSVCorruption; }
	inline const int32 _getHeroicCorruptionResist() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getHeroicCorruptionResist(); return getHeroicCorruptionResist() + count; }
	inline const int32 getHealAmount() const { return mItemData->mHealAmount; }
	inline const int32 _getHealAmount() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getHealAmount(); return getHealAmount() + count; }
	inline const int32 getSpellDamage() const { return mItemData->mSpellDamage; }
	inline const int32 _getSpellDamage() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getSpellDamage(); return getSpellDamage() + count; }
	inline const int32 getClairvoyance() const { return mItemData->mClairvoyance; }
	inline const int32 _getClairvoyance() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getClairvoyance(); return getClairvoyance() + count; }


	inline void setMerchantPrice(const uint32 pValue) { mItemData->mMerchantPrice = pValue; }
	inline void setMerchantSlot(const uint32 pValue) { mItemData->mMerchantSlot = pValue; }
	inline void setSerial(const uint32 pValue) { mItemData->mInstanceID = pValue; }
	inline void setLastCastTime(const uint32 pValue) { mItemData->mLastCastTime = pValue; }
	inline void setCharges(const uint32 pValue) { mItemData->mCharges = pValue; }
	inline void setPower(const uint32 pValue) { mItemData->mPower = pValue; }
	inline void setIsEvolvingItem(const uint32 pValue) { mItemData->mIsEvolvingItem = pValue; }
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
	inline void setSVCorruption(const int8 pValue) { mItemData->mCorruptionResist = pValue; }
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
	inline void setSkillModAmount(const int32 pValue) { mItemData->mSkillModAmount = pValue; }
	inline void setSkillMod(const uint32 pType) { mItemData->mSkillMod = pType; }
	inline void setBaneRace(const uint32 pValue) { mItemData->mBaneDamageRace = pValue; }
	inline void setBaneBodyType(const uint32 pValue) { mItemData->mBaneDamageBodyType = pValue; }
	inline void setBaneRaceAmount(const uint32 pValue) { mItemData->mBaneDamageRaceAmount = pValue; }
	inline void setBaneBodyTypeAmount(const int32 pValue) { mItemData->mBaneDamageBodyTypeAmount = pValue; }
	inline void setIsMagic(const bool pValue) { mItemData->mMagic = pValue ? 1 : 0; }
	inline void setFoodSize(const uint32 pValue) { mItemData->mFoodSize = pValue; }
	inline void setDrinkSize(const uint32 pValue) { mItemData->mDrinkSize = pValue; }
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
	inline void setSkillDamageMod(const int32 pValue) { mItemData->mSkillDamageMod = pValue; }
	inline void setSkillDamageModAmount(const int32 pValue) { mItemData->mSkillDamageModAmount = pValue; }
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

	inline void setBook(const uint8 pValue) { mItemData->mBook = pValue; }
	inline void setBookType(const uint8 pValue) { mItemData->mBookType = pValue; }

	inline void setLoreGroup(const int32 pValue) { mItemData->mLoreGroup = pValue; }
	inline void setIsArtifact(const uint8 pValue) { mItemData->mArtifact = pValue; }
	inline void setIsSummoned(const uint8 pValue) { mItemData->mSummoned = pValue; }
	inline void setFavor(const uint32 pValue) { mItemData->mFavor = pValue; }
	inline void setFVNoDrop(const uint8 pValue) { mItemData->mFVNoDrop = pValue; }
	inline void setDoTShield(const int32 pValue) { mItemData->mDoTShield = pValue; }
	inline void setAttack(const int32 pValue) { mItemData->mAttack = pValue; }
	inline void setHaste(const int32 pValue) { mItemData->mHaste = pValue; }
	inline void setDamageShield(const int32 pValue) { mItemData->mDamageShield = pValue; }
	inline void setGuildFavor(const uint32 pValue) { mItemData->mGuildFavor = pValue; }
	// TODO: augdistil
	inline void setIsNoPet(const uint8 pValue) { mItemData->mNoPet = pValue; }
	// TODO: potion belt
	inline void setMaxStacks(const uint32 pValue) { mItemData->mMaxStacks = pValue; }
	inline void setIsNoTransfer(const uint8 pValue) { mItemData->mNoTransfer = pValue; }
	// TODO: expendablearrow
	// TODO: Effects
	inline void setScriptFileID(const uint32 pValue) { mItemData->mScriptFileID = pValue; }
	inline void setIsQuest(const uint8 pValue) { mItemData->mQuest = pValue; }
	inline void setMaxPower(const uint32 pValue) { mItemData->mMaxPower = pValue; }
	inline void setPurity(const uint32 pValue) { mItemData->mPurity = pValue; }
	// TODO: Power
	// TODO: Purity
	inline void setBackstabDamage(const uint32 pValue) { mItemData->mBackstabDamage = pValue; }
	inline void setDamageShieldMitigation(const uint32 pValue) { mItemData->mDamageShieldMitigation = pValue; }
	inline void setHeroicStrength(const int32 pValue) { mItemData->mHeroicStrength = pValue; }
	inline void setHeroicIntelligence(const int32 pValue) { mItemData->mHeroicIntelligence = pValue; }
	inline void setHeroicWisdom(const int32 pValue) { mItemData->mHeroicWisdom = pValue; }
	inline void setHeroicAgility(const int32 pValue) { mItemData->mHeroicAgility = pValue; }
	inline void setHeroicDexterity(const int32 pValue) { mItemData->mHeroicDexterity = pValue; }
	inline void setHeroicStamina(const int32 pValue) { mItemData->mHeroicStamina = pValue; }
	inline void setHeroicCharisma(const int32 pValue) { mItemData->mHeroicCharisma = pValue; }
	inline void setHeroicMagicResist(const int32 pValue) { mItemData->mHeroicMagicResist = pValue; }
	inline void setHeroicFireResist(const int32 pValue) { mItemData->mHeroicFireResist = pValue; }
	inline void setHeroicColdResist(const int32 pValue) { mItemData->mHeroicColdResist = pValue; }
	inline void setHeroicDiseaseResist(const int32 pValue) { mItemData->mHeroicDiseaseResist = pValue; }
	inline void setHeroicPoisonResist(const int32 pValue) { mItemData->mHeroicPoisonResist = pValue; }
	inline void setHeroicCorruptionResist(const int32 pValue) { mItemData->mHeroicSVCorruption = pValue; }
	inline void setHealAmount(const int32 pValue) { mItemData->mHealAmount = pValue; }
	inline void setSpellDamage(const int32 pValue) { mItemData->mSpellDamage = pValue; }
	inline void setClairvoyance(const int32 pValue) { mItemData->mClairvoyance = pValue; }
private:
	const uint32 _getDataSize() const;
	void _onCopy();

	Item* mAugments[5];
	Item* mContents[10];
	ItemData* mItemData = nullptr;

	// ItemData Overrides
	uint32 mSlot = 0;
	uint32 mStacks = 1;
	bool mAttuned = false;
};