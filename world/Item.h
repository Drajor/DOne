#pragma once

#include "Constants.h"
#include "ItemData.h"
#include "Utility.h"

class DynamicStructure;
class Item {
public:
	Item(ItemData* pItemData);
	Item* copy() const;
	~Item();

	// Returns whether this Item is tradeable or not.
	const bool isTradeable();

	// Returns whether this Item is armor or not.
	inline const bool isArmor() const { return getItemType() == ItemType::Armor; }

	// Returns whether this Item is a shield or not.
	inline const bool isShield() const { return getItemType() == ItemType::Shield; }

	// Returns whether this Item is an augment or not.
	inline const bool isAugmentation() const { return getItemType() == ItemType::Augmentation; }

	// Returns whether this Item is one hand slash or not.
	inline const bool isOneHandSlash() const { return getItemType() == ItemType::OneHandSlash; }

	// Returns whether this Item is two hand slash or not.
	inline const bool isTwoHandSlash() const { return getItemType() == ItemType::TwoHandSlash; }

	// Returns whether this Item is one hand blunt or not.
	inline const bool isOneHandBlunt() const { return getItemType() == ItemType::OneHandBlunt; }

	// Returns whether this Item is two hand blunt or not.
	inline const bool isTwoHandBlunt() const { return getItemType() == ItemType::TwoHandBlunt; }

	// Returns whether this Item is one hand pierce or not.
	inline const bool isOneHandPierce() const { return getItemType() == ItemType::OneHandPierce; }

	// Returns whether this Item is two hand pierce or not.
	inline const bool isTwoHandPierce() const { return getItemType() == ItemType::TwoHandPierce; }

	// Returns whether this Item is a bow or not.
	inline const bool isBow() const { return getItemType() == ItemType::Bow; }

	// Returns whether this Item is hand to hand or not.
	inline const bool isHandToHand() const { return getItemType() == ItemType::HandToHand; }

	// Returns whether this Item is a one handed weapon or not.
	inline const bool isOneHandWeapon() const { return isOneHandSlash() || isOneHandBlunt() || isOneHandPierce() || isHandToHand(); }
	
	// Returns whether this Item is a two handed weapon or not.
	inline const bool isTwoHandWeapon() const { return isTwoHandSlash() || isTwoHandBlunt() || isTwoHandPierce(); }

	// Returns whether this Item is a weapon or not.
	inline const bool isWeapon() const { return isOneHandWeapon() || isTwoHandWeapon() || isBow(); }

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
	const bool removeStacks(const uint32 pStacks);;
	inline const bool isAttuned() { return mAttuned; }
	inline void setIsAttuned(const bool pAttuned) { mAttuned = pAttuned; }

	inline const uint32 getSubIndex() const { return mSubIndex; };
	inline const bool hasValidSubIndex() const { return mSubIndex >= 0 && mSubIndex < SlotID::MAX_CONTENTS; }
	const bool setSubIndex(const uint32 pSubIndex);
	inline void clearSubIndex() { mSubIndex = -1; }

	inline Item* getParent() const { return mParent; }
	const bool setParent(Item* pParent);
	inline void clearParent() { mParent = nullptr; }
	inline const bool hasParent() const { return getParent() != nullptr; }

	Item* getAugment(const uint8 pSlot) const;

	Item* getContents(const uint8 pSlot) const;
	const bool clearContents(const uint32 pSubIndex);
	const bool setContents(Item* pItem, const uint32 pSubIndex);
	void updateContentsSlots();

	const uint32 getDataSize() const;
	const unsigned char* copyData(uint32& pSize, const uint32 pCopyType);
	const bool copyData(Utility::DynamicStructure& pStructure);
	uint32 getSubItems() const;

	const bool isContainer() const { return getItemClass() == ItemClass::Container; }
	Item* findFirst(const uint8 pItemType);

	// Augmentation
	// TODO: This needs some error checking.
	inline void setAugmentation(uint32 pSlot, Item* pAugmentation) { mAugments[pSlot] = pAugmentation; }
	// Inserts an augment into the Item.
	const bool insertAugment(Item* pAugment);
	// Returns whether pAugment is allowed to go into this Item.
	const bool augmentAllowed(Item* pAugment);

	// Evolving.
	inline const int32 getCurrentEvolvingLevel() const { return mCurrentEvolvingLevel; }
	inline void setCurrentEvolvingLevel(const int32 pValue) { mCurrentEvolvingLevel = pValue; }
	inline const int32 getMaximumEvolvingLevel() const { return mItemData->mEvolvingItem.mMaxLevel; }
	inline void setMaximumEvolvingLevel(const int32 pValue) { mItemData->mEvolvingItem.mMaxLevel = pValue; }
	inline const double getEvolvingProgress() const { return mEvolvingProgress; }
	inline void setEvolvingProgress(const double pProgress) { mEvolvingProgress = pProgress; }

	// Ornamentation
	inline const uint16 getOrnamentationIcon() const { return mOrnamentationIcon; }
	inline void setOrnamentationIcon(const uint16 pValue) { mOrnamentationIcon = pValue; }
	inline const bool hasOrnamentationIcon() const { return getOrnamentationIcon() > 0; }

	inline const String getOrnamentationIDFile() const { return mOrnamentationIDFile; }
	inline void setOrnamentationIDFile(const String& pValue) { mOrnamentationIDFile = pValue; }
	inline const bool hasOrnamentationIDFile() const { return getOrnamentationIDFile().length() > 0; }

	inline const uint32 getMerchantPrice() const { return mItemData->mMerchantPrice; }
	inline const uint32 getMerchantSlot() const { return mItemData->mMerchantSlot; }
	inline const uint32 getInstanceID() const { return mItemData->mInstanceID; }
	inline const uint32 getLastCastTime() const { return mItemData->mLastCastTime; }
	inline const uint32 getCharges() const { return mItemData->mCharges; }
	inline const uint32 getPower() const { return mItemData->mPower; }
	inline const bool isEvolvingItem() const { return mItemData->mIsEvolvingItem > 0; }
	inline const uint8 getIsCopied() const { return mItemData->mCopied; }
	inline const uint8 getItemClass() const { return mItemData->mItemClass; }
	inline const uint32 getID() const { return mItemData->mID; }
	inline const uint8 getWeight() const { return mItemData->mWeight; }
	inline const uint8 isTemporary() const { return mItemData->mTemporary; }
	inline const uint8 isNoDrop() const { return mItemData->mNoDrop; }
	inline const uint8 isAttunable() const { return mItemData->mAttunable; }
	inline const uint8 getSize() const { return mItemData->mSize; }
	inline const uint32 getSlots() const { return mItemData->mSlots; }
	// Returns slot restrictions based on this Item and augments.
	inline const uint32 _getSlots() const { uint32 slots = getSlots(); for (auto i : mAugments) if (i) slots &= i->getSlots(); return slots; }
	inline const uint32 getPrice() const { return mItemData->mPrice; }
	inline const uint32 getIcon() const { return mItemData->mIcon; }
	inline const uint32 getBenefitFlag() const { return mItemData->mBenefitFlag; }
	inline const uint8 isTradeskillsItem() const { return mItemData->mTradeSkills; }

	// Returns the cold resist on this Item.
	inline const int8 getColdResist() const { return mItemData->mColdResist; }

	// Returns the total cold resist on this Item and augments.
	inline const int32 _getColdResist() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getColdResist(); return getColdResist() + count; }

	// Returns the disease resist on this Item.
	inline const int8 getDiseaseResist() const { return mItemData->mDieaseResist; }

	// Returns the total disease resist on this Item and augments.
	inline const int32 _getDiseaseResist() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getDiseaseResist(); return getDiseaseResist() + count; }

	// Returns the poison resist on this Item.
	inline const int8 getPoisonResist() const { return mItemData->mPoisonResist; }

	// Returns the total poison resist on this Item and augments.
	inline const int32 _getPoisonResist() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getPoisonResist(); return getPoisonResist() + count; }

	// Returns the magic resist on this Item.
	inline const int8 getMagicResist() const { return mItemData->mMagicResist; }

	// Returns the total magic resist on this Item and augments.
	inline const int32 _getMagicResist() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getMagicResist(); return getMagicResist() + count; }

	// Returns the fire resist on this Item.
	inline const int8 getFireResist() const { return mItemData->mFireResist; }

	// Returns the total fire resist on this Item and augments.
	inline const int32 _getFireResist() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getFireResist(); return getFireResist() + count; }

	// Returns the corruption resist on this Item.
	inline const int8 getCorruptionResist() const { return mItemData->mCorruptionResist; }

	// Returns total corruption resist on this Item and augments.
	inline const int32 _getCorruptionResist() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getCorruptionResist(); return getCorruptionResist() + count; }

	// Returns the strength on this Item.
	inline const int8 getStrength() const { return mItemData->mStrength; }

	// Returns the total strength on this Item and augments.
	inline const int32 _getStrength() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getStrength(); return getStrength() + count; }

	// Returns the stamina on this Item.
	inline const int8 getStamina() const { return mItemData->mStamina; }

	// Returns the total stamina on this Item and augments.
	inline const int32 _getStamina() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getStamina(); return getStamina() + count; }

	// Returns the agility on this Item.
	inline const int8 getAgility() const { return mItemData->mAgility; }

	// Returns the total agility on this Item and augments.
	inline const int32 _getAgility() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getAgility(); return getAgility() + count; }

	// Returns the dexterity on this Item.
	inline const int8 getDexterity() const { return mItemData->mDexterity; }

	// Returns the total dexterity on this Item and augments.
	inline const int32 _getDexterity() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getDexterity(); return getDexterity() + count; }

	// Returns the charisma on this Item.
	inline const int8 getCharisma() const { return mItemData->mCharisma; }

	// Returns the total charisma on this Item and augments.
	inline const int32 _getCharisma() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getCharisma(); return getCharisma() + count; }

	// Returns the intelligence on this Item.
	inline const int8 getIntelligence() const { return mItemData->mIntelligence; }

	// Returns the total intelligence on this Item and augments.
	inline const int32 _getIntelligence() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getIntelligence(); return getIntelligence() + count; }

	// Returns the wisdom on this Item.
	inline const uint8 getWisdom() const { return mItemData->mWisdom; }

	// Returns the total wisdom on this Item and augments.
	inline const int32 _getWisdom() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getWisdom(); return getWisdom() + count; }

	// Returns the health on this Item.
	inline const int32 getHealth() const { return mItemData->mHealth; }

	// Returns the total health on this Item and augments.
	inline const int32 _getHealth() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getHealth(); return getHealth() + count; }

	// Returns the mana on this Item.
	inline const int32 getMana() const { return mItemData->mMana; }

	// Returns the total mana on this Item and augments.
	inline const int32 _getMana() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getMana(); return getMana() + count; }

	// Returns the endurance on this Item.
	inline const uint32 getEndurance() const { return mItemData->mEndurance; }

	// Returns the total endurance on this Item and augments.
	inline const int32 _getEndurance() const { int32 v = 0; for (auto i : mAugments) if (i) v += i->getEndurance(); return getEndurance() + v; }

	// Returns the armor class on this Item.
	inline const int32 getArmorClass() const { return mItemData->mArmorClass; }

	// Returns the total armor class on this Item and augments.
	inline const int32 _getArmorClass() const { int32 v = 0; for (auto i : mAugments) if (i) v += i->getArmorClass(); return getArmorClass() + v; }

	// Returns the health regeneration on this Item.
	inline const int32 getHealthRegen() const { return mItemData->mHealthRegen; }

	// Returns the total health regeneration on this Item and augments.
	inline const int32 _getHealthRegen() const { int32 v = getHealthRegen(); for (auto i : mAugments) if (i) v += i->getHealthRegen(); return v; }

	// Returns the mana regeneration on this Item.
	inline const int32 getManaRegen() const { return mItemData->mManaRegen; }

	// Returns the total mana regeneration on this Item and augments.
	inline const int32 _getManaRegen() const { int32 v = getManaRegen(); for (auto i : mAugments) if (i) v += i->getManaRegen(); return v; }
	
	// Returns the endurance regeneration on this Item.
	inline const int32 getEnduranceRegen() const { return mItemData->mEnduranceRegen; }
	
	// Returns the total endurance regeneration on this Item and augments.
	inline const int32 _getEnduranceRegen() const { int32 v = getEnduranceRegen(); for (auto i : mAugments) if (i) v += i->getEnduranceRegen(); return v; }
	
	// Returns the class restrictions on this Item.
	inline const uint32 getClasses() const { return mItemData->mClasses; }
	
	// Returns the class restrictions on this Item and augments.
	inline const uint32 _getClasses() const { uint32 v = getClasses(); for (auto i : mAugments) if (i) v &= i->getSlots(); return v; }
	
	// Returns the race restrictions on this Item.
	inline const uint32 getRaces() const { return mItemData->mRaces; }
	
	// Returns the race restrictions on this Item and augments.
	inline const uint32 _getRaces() const { uint32 v = getRaces(); for (auto i : mAugments) if (i) v &= i->getRaces(); return v; }
	
	// Returns the deity restrictions on this Item.
	inline const uint32 getDeities() const { return mItemData->mDeities; }
	
	// Returns the deity restrictions on this Item and augments.
	inline const uint32 _getDeities() const { uint32 v = getDeities(); for (auto i : mAugments) if (i) v &= i->getDeities(); return v; }
	
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
	inline const uint32 getRequiredLevel() const { return mItemData->mReqLevel; }
	inline const uint32 getRecLevel() const { return mItemData->mRecLevel; }
	inline const uint32 getRecSkill() const { return mItemData->mRecSkill; }
	inline const uint32 getBardType() const { return mItemData->mBardType; }
	inline const int32 getBardValue() const { return mItemData->mBardValue; }
	inline const uint8 getLight() const { return mItemData->mLight; }
	inline const uint8 getDelay() const { return mItemData->mDelay; }
	inline const uint8 getElementalDamageType() const { return mItemData->mElementalDamageType; }
	inline const uint8 getElementalDamageAmount() const { return mItemData->mElementalDamageAmount; }
	inline const uint8 getRange() const { return mItemData->mRange; }

	// Returns the damage on this Item.
	inline const uint32 getDamage() const { return mItemData->mDamage; }

	// Returns the total damage on this Item and augments.
	inline const int32 _getDamage() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getDamage(); return getDamage() + count; }

	// Returns the colour of this Item.
	inline const uint32 getColour() const { return mItemData->mColour; }

	// Returns the type of Item. See Constants.h ItemType enumeration.
	inline const uint8 getItemType() const { return mItemData->mItemType; }

	// Returns the material on this Item. See Constants.h ItemMaterial enumeration.
	inline const uint32 getMaterial() const { return mItemData->mMaterial; }
	
	// TODO: Not sure!
	inline const uint32 getEliteMaterial() const { return mItemData->mEliteMaterial; }

	// TODO:
	inline const float getSellRate() const { return mItemData->mSellRate; }

	// (MOD2) Returns the combat effects on this Item.
	inline const int32 getCombatEffects() const { return mItemData->mCombatEffects; }

	// (MOD2) Returns the total combat effects on this Item and augments.
	inline const int32 _getCombatEffects() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getCombatEffects(); return getCombatEffects() + count; }

	// (MOD2) Returns the shielding on this Item.
	inline const int32 getShielding() const { return mItemData->mShielding; }

	// (MOD2) Returns the total shielding on this Item and augments.
	inline const int32 _getShielding() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getShielding(); return getShielding() + count; }

	// (MOD2) Returns the stun resist on this Item.
	inline const int32 getStunResist() const { return mItemData->mStunResist; }

	// (MOD2) Returns the total stun resist on this Item and augments.
	inline const int32 _getStunResist() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getStunResist(); return getStunResist() + count; }

	// (MOD2) Returns the strike through on this Item.
	inline const int32 getStrikeThrough() const { return mItemData->mStrikeThrough; }

	// (MOD2) Returns the total strike through on this Item and augments.
	inline const int32 _getStrikeThrough() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getStrikeThrough(); return getStrikeThrough() + count; }


	// TODO: I need to see whether augments stack this.
	inline const int32 getSkillDamageMod() const { return mItemData->mSkillDamageMod; }
	inline const int32 getSkillDamageModAmount() const { return mItemData->mSkillDamageModAmount; }

	// (MOD2) Returns the spell shield on this Item.
	inline const int32 getSpellShield() const { return mItemData->mSpellShield; }

	// (MOD2) Returns the total spell shield on this Item and augments.
	inline const int32 _getSpellShield() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getSpellShield(); return getSpellShield() + count; }

	// (MOD2) Returns the avoidance on this Item.
	inline const int32 getAvoidance() const { return mItemData->mAvoidance; }

	// (MOD2) Returns the total avoidance on this Item and augments.
	inline const int32 _getAvoidance() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getAvoidance(); return getAvoidance() + count; }

	// (MOD2) Returns the accuracy on this Item.
	inline const int32 getAccuracy() const { return mItemData->mAccuracy; }

	// (MOD2) Returns the total accuracy on this Item and augments.
	inline const int32 _getAccuracy() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getAccuracy(); return getAccuracy() + count; }
	
	// TODO: Faction Modifiers
	
	inline const uint32 getAugmentationType() const { return mItemData->mAugType; }
	inline const uint32 getAugmentationRestriction() const { return mItemData->mAugmentationRestriction; }
	
	// Augmentation Slots
	
	inline const uint32 getLDONPointType() const { return mItemData->mLDONPointType; }
	inline const uint32 getLDONTheme() const { return mItemData->mLDONTheme; }
	inline const uint32 getLDONPrice() const { return mItemData->mLDONPrice; }
	inline const uint32 getLDONSellBackPrice() const { return mItemData->mLDONSellBackPrice; }
	inline const uint32 getLDONSold() const { return mItemData->mLDONSold; }

	// Returns the container type of this Item. See Constants.h ContainerType enumeration.
	inline const uint8 getContainerType() const { return mItemData->mContainerType; }

	// Returns the number of container slots this Item has. 1-10.
	inline const uint8 getContainerSlots() const { return mItemData->mContainerSlots; }

	// Returns the container size. See Constants.h ContainerSize enumeration.
	inline const uint8 getContainerSize() const { return mItemData->mContainerSize; }

	// Returns the container weight reduction. 0-100.
	inline const uint8 getContainerWR() const { return mItemData->mContainerWR; }

	inline const uint8 getBook() const { return mItemData->mBook; }
	inline const uint8 getBookType() const { return mItemData->mBookType; }
	inline const int32 getLoreGroup() const { return mItemData->mLoreGroup; }
	inline const uint8 getIsArtifact() const { return mItemData->mArtifact; }
	inline const uint8 getIsSummoned() const { return mItemData->mSummoned; }
	inline const uint32 getFavor() const { return mItemData->mFavor; }
	inline const uint8 getFVNoDrop() const { return mItemData->mFVNoDrop; }
	
	// Returns the DoT shield on this Item.
	inline const int32 getDoTShield() const { return mItemData->mDoTShield; }

	// Returns the total DoT shield on this Item and augments.
	inline const int32 _getDoTShield() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getDoTShield(); return getDoTShield() + count; }

	// TODO: Does attack stack with augs?
	inline const int32 getAttack() const { return mItemData->mAttack; }
	
	inline const int32 getHaste() const { return mItemData->mHaste; }
	inline const int32 getDamageShield() const { return mItemData->mDamageShield; }
	inline const uint32 getGuildFavor() const { return mItemData->mGuildFavor; }
	inline const uint32 getAugmentationDistiller() const { return mItemData->mAugmentationDistiller; }
	inline const uint8 getIsNoPet() const { return mItemData->mNoPet; }
	// TODO: potion belt
	inline const bool isStackable() const { return getMaxStacks() > 1; }
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

	// Returns the heroic strength on this Item.
	inline const int32 getHeroicStrength() const { return mItemData->mHeroicStrength; }

	// Returns the total heroic strength on this Item and augments.
	inline const int32 _getHeroicStrength() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getHeroicStrength(); return getHeroicStrength() + count; }

	// Returns the heroic intelligence on this Item.
	inline const int32 getHeroicIntelligence() const { return mItemData->mHeroicIntelligence; }

	// Returns the total heroic intelligence on this Item and augments.
	inline const int32 _getHeroicIntelligence() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getHeroicIntelligence(); return getHeroicIntelligence() + count; }

	// Returns the heroic wisdom on this Item.
	inline const int32 getHeroicWisdom() const { return mItemData->mHeroicWisdom; }

	// Returns the total heroic wisdom on this Item and augments.
	inline const int32 _getHeroicWisdom() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getHeroicWisdom(); return getHeroicWisdom() + count; }

	// Returns the heroic agility on  this Item.
	inline const int32 getHeroicAgility() const { return mItemData->mHeroicAgility; }

	// Returns the total heroic agility on this Item.
	inline const int32 _getHeroicAgility() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getHeroicAgility(); return getHeroicAgility() + count; }

	// Returns the heroic dexterity on this Item.
	inline const int32 getHeroicDexterity() const { return mItemData->mHeroicDexterity; }

	// Returns the total heroic dexterity on this Item and augments.
	inline const int32 _getHeroicDexterity() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getHeroicDexterity(); return getHeroicDexterity() + count; }

	// Returns the heroic stamina on this Item.
	inline const int32 getHeroicStamina() const { return mItemData->mHeroicStamina; }

	// Returns the total heroic stamina on this Item and augments.
	inline const int32 _getHeroicStamina() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getHeroicStamina(); return getHeroicStamina() + count; }

	// Returns the heroic charisma on this Item.
	inline const int32 getHeroicCharisma() const { return mItemData->mHeroicCharisma; }

	// Returns the total heroic charisma on this Item and augments.
	inline const int32 _getHeroicCharisma() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getHeroicCharisma(); return getHeroicCharisma() + count; }

	// Returns the heroic magic resist on this Item.
	inline const int32 getHeroicMagicResist() const { return mItemData->mHeroicMagicResist; }
	
	// Returns the total heroic magic resist on this Item and augments.
	inline const int32 _getHeroicMagicResist() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getHeroicMagicResist(); return getHeroicMagicResist() + count; }

	// Returns the heroic fire resist on this Item.
	inline const int32 getHeroicFireResist() const { return mItemData->mHeroicFireResist; }

	// Returns the total heroic fire resist on this Item and augments.
	inline const int32 _getHeroicFireResist() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getHeroicFireResist(); return getHeroicFireResist() + count; }

	// Returns the heroic cold resist on this Item.
	inline const int32 getHeroicColdResist() const { return mItemData->mHeroicColdResist; }

	// Returns the total heroic cold resist on this Item and augments.
	inline const int32 _getHeroicColdResist() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getHeroicColdResist(); return getHeroicColdResist() + count; }

	// Returns the heroic disease resist on this Item.
	inline const int32 getHeroicDiseaseResist() const { return mItemData->mHeroicDiseaseResist; }

	// Returns the total heroic disease resist on this Item and augments.
	inline const int32 _getHeroicDiseaseResist() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getHeroicDiseaseResist(); return getHeroicDiseaseResist() + count; }

	// Returns the heroic poison resist on this Item.
	inline const int32 getHeroicPoisonResist() const { return mItemData->mHeroicPoisonResist; }

	// Returns the total heroic poison resist on this Item and augments.
	inline const int32 _getHeroicPoisonResist() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getHeroicPoisonResist(); return getHeroicPoisonResist() + count; }

	// Returns the heroic corruption resist on this Item.
	inline const int32 getHeroicCorruptionResist() const { return mItemData->mHeroicSVCorruption; }

	// Returns the total heroic corruption resist on this Item and augments.
	inline const int32 _getHeroicCorruptionResist() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getHeroicCorruptionResist(); return getHeroicCorruptionResist() + count; }

	// (MOD2) Returns the heal amount on this Item.
	inline const int32 getHealAmount() const { return mItemData->mHealAmount; }

	// (MOD2) Returns the total heal amount on this Item and augments.
	inline const int32 _getHealAmount() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getHealAmount(); return getHealAmount() + count; }

	// (MOD2) Returns the spell damage on this Item.
	inline const int32 getSpellDamage() const { return mItemData->mSpellDamage; }

	// (MOD2) Returns the total spell damage on this Item and augments.
	inline const int32 _getSpellDamage() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getSpellDamage(); return getSpellDamage() + count; }

	// (MOD2) Returns the clairvoyance on this Item.
	inline const int32 getClairvoyance() const { return mItemData->mClairvoyance; }

	// (MOD2) Returns the total clairvoyance on this Item and augments.
	inline const int32 _getClairvoyance() const { int32 count = 0; for (auto i : mAugments) if (i) count += i->getClairvoyance(); return getClairvoyance() + count; }

	// Returns whether or not this Item is an heirloom.
	inline const bool getIsHeirloom() const { return mItemData->mHeirloom == 1; }


	inline void setMerchantPrice(const uint32 pValue) { mItemData->mMerchantPrice = pValue; }
	inline void setMerchantSlot(const uint32 pValue) { mItemData->mMerchantSlot = pValue; }
	inline void setSerial(const uint32 pValue) { mItemData->mInstanceID = pValue; }
	inline void setLastCastTime(const uint32 pValue) { mItemData->mLastCastTime = pValue; }
	inline void setCharges(const uint32 pValue) { mItemData->mCharges = pValue; }
	inline void setPower(const uint32 pValue) { mItemData->mPower = pValue; }
	inline void setIsEvolvingItem(const bool pValue) { mItemData->mIsEvolvingItem = pValue ? 1 : 0; }
	inline void setIsCopied(const uint8 pValue) { mItemData->mCopied = pValue; }
	inline void setItemClass(const uint8 pValue) { mItemData->mItemClass = pValue; }
	inline void setID(const uint32 pValue) { mItemData->mID = pValue; }
	inline void setWeight(const uint8 pValue) { mItemData->mWeight = pValue; }
	inline void setIsTemporary(const bool pValue) { mItemData->mTemporary = pValue ? 1 : 0; }
	inline void setIsNoDrop(const bool pValue) { mItemData->mNoDrop = pValue ? 0 : 1; }
	inline void setIsAttunable(const bool pValue) { mItemData->mAttunable = pValue ? 1 : 0; }
	inline void setSize(const uint8 pValue) { mItemData->mSize = pValue; }
	inline void setSlots(const uint32 pValue) { mItemData->mSlots = pValue; }
	inline void setPrice(const uint32 pValue) { mItemData->mPrice = pValue; }
	inline void setIcon(const uint32 pValue) { mItemData->mIcon = pValue; }
	inline void setBenefitFlag(const uint32 pValue) { mItemData->mBenefitFlag = pValue; }
	inline void setIsTradeskillsItem(const bool pValue) { mItemData->mTradeSkills = pValue ? 1 : 0; }
	
	// Sets the cold resist on this Item.
	inline void setColdResist(const int8 pValue) { mItemData->mColdResist = pValue; }

	// Sets the disease resist on this Item.
	inline void setDiseaseResist(const int8 pValue) { mItemData->mDieaseResist = pValue; }

	// Sets the poison resist on this Item.
	inline void setPoisonResist(const int8 pValue) { mItemData->mPoisonResist = pValue; }

	// Sets the magic resist on this Item.
	inline void setMagicResist(const int8 pValue) { mItemData->mMagicResist = pValue; }

	// Sets the fire resist on this Item.
	inline void setFireResist(const int8 pValue) { mItemData->mFireResist = pValue; }

	// Sets the corruption resist on this Item.
	inline void setCorruptionResist(const int8 pValue) { mItemData->mCorruptionResist = pValue; }

	// Sets the strength on this Item.
	inline void setStrength(const int8 pValue) { mItemData->mStrength = pValue; }

	// Sets the stamina on this Item.
	inline void setStamina(const int8 pValue) { mItemData->mStamina = pValue; }
	
	// Sets the agility on this Item.
	inline void setAgility(const int8 pValue) { mItemData->mAgility = pValue; }

	// Sets the dexterity on this Item.
	inline void setDexterity(const int8 pValue) { mItemData->mDexterity = pValue; }

	// Sets the charisma on this Item.
	inline void setCharisma(const int8 pValue) { mItemData->mCharisma = pValue; }

	// Sets the intelligence on this Item.
	inline void setIntelligence(const int8 pValue) { mItemData->mIntelligence = pValue; }

	// Sets the wisdom on this Item.
	inline void setWisdom(const int8 pValue) { mItemData->mWisdom = pValue; }

	// Sets the health on this Item.
	inline void setHealth(const int32 pValue) { mItemData->mHealth = pValue; }

	// Sets the mana on this Item.
	inline void setMana(const int32 pValue) { mItemData->mMana = pValue; }
	
	// Sets the endurance on this Item.
	inline void setEndurance(const uint32 pValue) { mItemData->mEndurance = pValue; }

	// Sets the armor class on this Item.
	inline void setArmorClass(const int32 pValue) { mItemData->mArmorClass = pValue; }

	// Sets the health regeneration on this Item.
	inline void setHealthRegen(const int32 pValue) { mItemData->mHealthRegen = pValue; }

	// Sets the mana regeneration on this Item.
	inline void setManaRegen(const int32 pValue) { mItemData->mManaRegen = pValue; }

	// Sets the endurance regeneration on this Item.
	inline void setEnduranceRegen(const int32 pValue) { mItemData->mEnduranceRegen = pValue; }

	inline void setClasses(const uint32 pValue) { mItemData->mClasses = pValue; }
	inline void setRaces(const uint32 pValue) { mItemData->mRaces = pValue; }
	inline void setDeities(const uint32 pValue) { mItemData->mDeities = pValue; }
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
	inline void setAugmentationType(const uint32 pValue) const { mItemData->mAugType = pValue; }
	inline void setAugmentationRestriction(const uint32 pValue) const { mItemData->mAugmentationRestriction = pValue; }
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
	inline void setAugmentationDistiller(const uint32 pValue) { mItemData->mAugmentationDistiller = pValue; }
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

	// Sets the heroic strength on this Item.
	inline void setHeroicStrength(const int32 pValue) { mItemData->mHeroicStrength = pValue; }

	// Sets the heroic intelligence on this Item.
	inline void setHeroicIntelligence(const int32 pValue) { mItemData->mHeroicIntelligence = pValue; }

	// Sets the heroic wisdom on this Item.
	inline void setHeroicWisdom(const int32 pValue) { mItemData->mHeroicWisdom = pValue; }

	// Sets the heroic agility on this Item.
	inline void setHeroicAgility(const int32 pValue) { mItemData->mHeroicAgility = pValue; }

	// Sets the heroic dexterity on this Item.
	inline void setHeroicDexterity(const int32 pValue) { mItemData->mHeroicDexterity = pValue; }

	// Sets the heroic stamina on this Item.
	inline void setHeroicStamina(const int32 pValue) { mItemData->mHeroicStamina = pValue; }

	// Sets the heroic charisma on this Item.
	inline void setHeroicCharisma(const int32 pValue) { mItemData->mHeroicCharisma = pValue; }

	// Sets the heroic magic resist on this Item.
	inline void setHeroicMagicResist(const int32 pValue) { mItemData->mHeroicMagicResist = pValue; }

	// Sets the heroic fire resist on this Item.
	inline void setHeroicFireResist(const int32 pValue) { mItemData->mHeroicFireResist = pValue; }

	// Sets the  heroic cold resist on this Item.
	inline void setHeroicColdResist(const int32 pValue) { mItemData->mHeroicColdResist = pValue; }

	// Sets the heroic disease resist on this Item.
	inline void setHeroicDiseaseResist(const int32 pValue) { mItemData->mHeroicDiseaseResist = pValue; }

	// Sets the heroic poison resist on this Item.
	inline void setHeroicPoisonResist(const int32 pValue) { mItemData->mHeroicPoisonResist = pValue; }

	// Sets the heroic corruption resist on this Item.
	inline void setHeroicCorruptionResist(const int32 pValue) { mItemData->mHeroicSVCorruption = pValue; }

	// (MOD2) Sets the heal amount on this Item.
	inline void setHealAmount(const int32 pValue) { mItemData->mHealAmount = pValue; }

	// (MOD2) Sets the spell damage on this Item.
	inline void setSpellDamage(const int32 pValue) { mItemData->mSpellDamage = pValue; }

	// (MOD2) Sets the clairvoyance on this Item.
	inline void setClairvoyance(const int32 pValue) { mItemData->mClairvoyance = pValue; }

	// Sets whether this Item is an heirloom or not.
	inline void setIsHeirloom(const bool pValue) { mItemData->mHeirloom = pValue ? 1 : 0; }

private:
	const uint32 _getDataSize() const;
	void _onCopy();
	Item* mParent = nullptr;
	Item* mAugments[5];
	Item* mContents[10];
	ItemData* mItemData = nullptr;

	// ItemData Overrides
	uint32 mSlot = 0;
	uint32 mStacks = 1;
	bool mAttuned = false;
	uint16 mOrnamentationIcon = 0;
	String mOrnamentationIDFile = "";

	// When an Item is a sub-Item such as an augment or container contents
	// mSubIndex will be a value 0-9 that specifies the position it is stored.
	int32 mSubIndex = -1;

	int32 mCurrentEvolvingLevel = 0;
	double mEvolvingProgress = 0.0f;
};