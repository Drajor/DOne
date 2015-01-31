#pragma once

#include "Constants.h"
#include "ItemData.h"
#include "Utility.h"

class MemoryWriter;
class Item {
public:
	Item(ItemData* pItemData);
	Item* copy() const;
	~Item();

	String getLink() const;

	const bool compare(Item * pItem, String& pText);

	inline ItemData* getData() const { return mItemData; }

	// Returns whether this Item can be traded or not.
	const bool isTradeable() const;

	// Returns whether this Item can be sold or not.
	const bool isSellable() const;

	// Returns the adjusted sell price based on a sell rate.
	const u32 getSellPrice(const u32 pStacks, const float pSellRate) const;

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

	// Returns whether this Item is food or not.
	inline const bool isFood() const { return getItemType() == ItemType::Food; }

	// Returns whether this Item is drink or not.
	inline const bool isDrink() const { return getItemType() == ItemType::Drink; }

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

	// Returns the number of stacks this Item has.
	inline const u32 getStacks() const { return mStacks; }
	
	// Returns the number of stacks that this item is missing. I.e. If the item is 91/100 stacks, it will return 9.
	inline const u32 getEmptyStacks() const { return getMaxStacks() - getStacks(); }

	// Returns the slot this Item is in.
	inline const u32 getSlot() const { return mSlot; }

	// Returns the shop price of the Item.
	inline const u32 getShopPrice() const { return mShopPrice; }

	// Returns the shop quantity of the Item.
	inline const i32 getShopQuantity() const { return mShopQuantity; }

	// Returns the instance ID of this Item.
	inline const u32 getInstanceID() const { return mInstanceID; }

	// Returns whether or not this Item is attuned.
	inline const bool isAttuned() const { return mAttuned; }

	// Returns the time stamp this Item was last cast.
	inline const u32 getLastCastTime() const { return mLastCastTime; }

	// Returns the number of charges this Item has.
	inline const u32 getCharges() const { return mCharges; }

	// Returns the power of this Item.
	inline const u32 getPower() const { return mPower; }

	// Sets the number of stacks this Item has.
	inline void setStacks(const u32 pStacks) { mStacks = pStacks; }

	// Sets the slot this Item is in.
	inline void setSlot(const u32 pSlot) { mSlot = pSlot; }

	// Sets the shop price of the Item.
	inline void setShopPrice(const u32 pValue) { mShopPrice = pValue; }

	// Sets the shop quantity of the Item.
	inline void setShopQuantity(const i32 pValue) { mShopQuantity = pValue; }

	// Sets the instance ID of this Item.
	inline void setInstanceID(const u32 pValue) { mInstanceID = pValue; }

	// Sets the time stamp this Item was last cast.
	inline void setLastCastTime(const u32 pValue) { mLastCastTime = pValue; }

	// Sets the number of charges this Item has.
	inline void setCharges(const u32 pValue) { mCharges = pValue; }

	// Sets the power of this Item.
	inline void setPower(const u32 pValue) { mPower = pValue; }

	// Sets whether or not this Item is attuned.
	inline void setIsAttuned(const bool pAttuned) { mAttuned = pAttuned; }
	
	// Adds stacks to the Item.
	const bool addStacks(const u32 pStacks);

	// Removes stacks from the Item.
	const bool removeStacks(const u32 pStacks);

	// Adds charges to the Item.
	const bool addCharges(const i32 pCharges) { return true; }

	// Removes charges from the Item.
	const bool removeCharges(const i32 pCharges) { return true; }

	inline const u32 getSubIndex() const { return mSubIndex; };
	inline const bool hasValidSubIndex() const { return mSubIndex >= 0 && mSubIndex < SlotID::MAX_CONTENTS; }
	const bool setSubIndex(const u32 pSubIndex);
	inline void clearSubIndex() { mSubIndex = -1; }

	inline Item* getParent() const { return mParent; }
	const bool setParent(Item* pParent);
	inline void clearParent() { mParent = nullptr; }
	inline const bool hasParent() const { return getParent() != nullptr; }

	Item* getAugment(const u8 pSlot) const;

	Item* getContents(const u8 pSlot) const;

	// Returns a copy of this Item's contents.
	void getContents(std::list<Item*>& pItems) const;

	const bool clearContents(const u32 pSubIndex);
	const bool setContents(Item* pItem, const u32 pSubIndex);
	void updateContentsSlots();

	// Returns a copy of this Item's augmentations.
	void getAugmentations(std::list<Item*>& pAugmentations) const;

	const u32 getDataSize(const u32 pCopyType) const;
	const unsigned char* copyData(u32& pSize, const u32 pCopyType);
	const bool copyData(Utility::MemoryWriter& pWriter, const u32 pCopyType);
	u32 getSubItems() const;

	const bool isContainer() const { return getItemClass() == ItemClass::Container; }
	const bool isCombineContainer() const;
	const bool isEmpty() const;
	const bool forEachContents(std::function<const bool(Item*)> pFunction) const;
	Item* findFirst(const u8 pItemType) const;
	Item* findStackable(const u32 pItemID) const;

	// Returns SlotID::None if no empty slot is found.
	const u32 findEmptySlot() const;

	// Augmentation
	// TODO: This needs some error checking.
	inline void setAugmentation(u32 pSlot, Item* pAugmentation) { mAugments[pSlot] = pAugmentation; }
	// Inserts an augment into the Item.
	const bool insertAugment(Item* pAugment);
	// Returns whether pAugment is allowed to go into this Item.
	const bool augmentAllowed(Item* pAugment);

	// Returns whether this Item has any augmentations or not.
	const bool hasAugmentations() const;

	// Evolving.
	inline const i32 getEvolvingLevel() const { return mEvolvingLevel; }
	inline const bool getEvolvingActive() const { return mEvolvingActive; }
	inline const i32 getEvolvingMaximumLevel() const { return mItemData->mEvolvingItem.mMaxLevel; }
	inline const double getEvolvingProgress() const { return mEvolvingProgress; }

	inline void setEvolvingLevel(const i32 pValue) { mEvolvingLevel = pValue; }
	inline void setEvolvingActive(const bool pValue) { mEvolvingActive = pValue; }
	inline void setEvolvingMaximumLevel(const i32 pValue) { mItemData->mEvolvingItem.mMaxLevel = pValue; }
	inline void setEvolvingProgress(const double pProgress) { mEvolvingProgress = pProgress; }

	// Ornamentation
	inline const u16 getOrnamentationIcon() const { return mOrnamentationIcon; }
	inline void setOrnamentationIcon(const u16 pValue) { mOrnamentationIcon = pValue; }
	inline const bool hasOrnamentationIcon() const { return getOrnamentationIcon() > 0; }

	inline const String getOrnamentationIDFile() const { return mOrnamentationIDFile; }
	inline void setOrnamentationIDFile(const String& pValue) { mOrnamentationIDFile = pValue; }
	inline const bool hasOrnamentationIDFile() const { return getOrnamentationIDFile().length() > 0; }

	inline const bool isEvolvingItem() const { return mItemData->mIsEvolvingItem > 0; }
	inline const bool isCopied() const { return mItemData->mCopied == 1; }
	inline const u8 getItemClass() const { return mItemData->mItemClass; }
	inline const u32 getID() const { return mItemData->mID; }
	inline const u8 getWeight() const { return mItemData->mWeight; }
	inline const bool isTemporary() const { return mItemData->mTemporary == 0; }
	inline const bool isNoDrop() const { return mItemData->mNoDrop == 0; }
	inline const bool isAttunable() const { return mItemData->mAttunable == 1; }
	inline const u8 getSize() const { return mItemData->mSize; }
	inline const u32 getSlots() const { return mItemData->mSlots; }
	// Returns slot restrictions based on this Item and augments.
	inline const u32 _getSlots() const { u32 slots = getSlots(); for (auto i : mAugments) if (i) slots &= i->getSlots(); return slots; }
	inline const u32 getPrice() const { return mItemData->mPrice; }
	inline const u32 getIcon() const { return mItemData->mIcon; }
	inline const u32 getBenefitFlag() const { return mItemData->mBenefitFlag; }
	inline const u8 isTradeskillsItem() const { return mItemData->mTradeSkills; }

	// Returns the cold resist on this Item.
	inline const i8 getColdResist() const { return mItemData->mColdResist; }

	// Returns the total cold resist on this Item and augments.
	inline const i32 _getColdResist() const { i32 count = 0; for (auto i : mAugments) if (i) count += i->getColdResist(); return getColdResist() + count; }

	// Returns the disease resist on this Item.
	inline const i8 getDiseaseResist() const { return mItemData->mDieaseResist; }

	// Returns the total disease resist on this Item and augments.
	inline const i32 _getDiseaseResist() const { i32 count = 0; for (auto i : mAugments) if (i) count += i->getDiseaseResist(); return getDiseaseResist() + count; }

	// Returns the poison resist on this Item.
	inline const i8 getPoisonResist() const { return mItemData->mPoisonResist; }

	// Returns the total poison resist on this Item and augments.
	inline const i32 _getPoisonResist() const { i32 count = 0; for (auto i : mAugments) if (i) count += i->getPoisonResist(); return getPoisonResist() + count; }

	// Returns the magic resist on this Item.
	inline const i8 getMagicResist() const { return mItemData->mMagicResist; }

	// Returns the total magic resist on this Item and augments.
	inline const i32 _getMagicResist() const { i32 count = 0; for (auto i : mAugments) if (i) count += i->getMagicResist(); return getMagicResist() + count; }

	// Returns the fire resist on this Item.
	inline const i8 getFireResist() const { return mItemData->mFireResist; }

	// Returns the total fire resist on this Item and augments.
	inline const i32 _getFireResist() const { i32 count = 0; for (auto i : mAugments) if (i) count += i->getFireResist(); return getFireResist() + count; }

	// Returns the corruption resist on this Item.
	inline const i8 getCorruptionResist() const { return mItemData->mCorruptionResist; }

	// Returns total corruption resist on this Item and augments.
	inline const i32 _getCorruptionResist() const { i32 count = 0; for (auto i : mAugments) if (i) count += i->getCorruptionResist(); return getCorruptionResist() + count; }

	// Returns the strength on this Item.
	inline const i8 getStrength() const { return mItemData->mStrength; }

	// Returns the total strength on this Item and augments.
	inline const i32 _getStrength() const { i32 count = 0; for (auto i : mAugments) if (i) count += i->getStrength(); return getStrength() + count; }

	// Returns the stamina on this Item.
	inline const i8 getStamina() const { return mItemData->mStamina; }

	// Returns the total stamina on this Item and augments.
	inline const i32 _getStamina() const { i32 count = 0; for (auto i : mAugments) if (i) count += i->getStamina(); return getStamina() + count; }

	// Returns the agility on this Item.
	inline const i8 getAgility() const { return mItemData->mAgility; }

	// Returns the total agility on this Item and augments.
	inline const i32 _getAgility() const { i32 count = 0; for (auto i : mAugments) if (i) count += i->getAgility(); return getAgility() + count; }

	// Returns the dexterity on this Item.
	inline const i8 getDexterity() const { return mItemData->mDexterity; }

	// Returns the total dexterity on this Item and augments.
	inline const i32 _getDexterity() const { i32 count = 0; for (auto i : mAugments) if (i) count += i->getDexterity(); return getDexterity() + count; }

	// Returns the charisma on this Item.
	inline const i8 getCharisma() const { return mItemData->mCharisma; }

	// Returns the total charisma on this Item and augments.
	inline const i32 _getCharisma() const { i32 count = 0; for (auto i : mAugments) if (i) count += i->getCharisma(); return getCharisma() + count; }

	// Returns the intelligence on this Item.
	inline const i8 getIntelligence() const { return mItemData->mIntelligence; }

	// Returns the total intelligence on this Item and augments.
	inline const i32 _getIntelligence() const { i32 count = 0; for (auto i : mAugments) if (i) count += i->getIntelligence(); return getIntelligence() + count; }

	// Returns the wisdom on this Item.
	inline const u8 getWisdom() const { return mItemData->mWisdom; }

	// Returns the total wisdom on this Item and augments.
	inline const i32 _getWisdom() const { i32 count = 0; for (auto i : mAugments) if (i) count += i->getWisdom(); return getWisdom() + count; }

	// Returns the health on this Item.
	inline const i32 getHealth() const { return mItemData->mHealth; }

	// Returns the total health on this Item and augments.
	inline const i32 _getHealth() const { i32 count = 0; for (auto i : mAugments) if (i) count += i->getHealth(); return getHealth() + count; }

	// Returns the mana on this Item.
	inline const i32 getMana() const { return mItemData->mMana; }

	// Returns the total mana on this Item and augments.
	inline const i32 _getMana() const { i32 count = 0; for (auto i : mAugments) if (i) count += i->getMana(); return getMana() + count; }

	// Returns the endurance on this Item.
	inline const u32 getEndurance() const { return mItemData->mEndurance; }

	// Returns the total endurance on this Item and augments.
	inline const i32 _getEndurance() const { i32 v = 0; for (auto i : mAugments) if (i) v += i->getEndurance(); return getEndurance() + v; }

	// Returns the armor class on this Item.
	inline const i32 getArmorClass() const { return mItemData->mArmorClass; }

	// Returns the total armor class on this Item and augments.
	inline const i32 _getArmorClass() const { i32 v = 0; for (auto i : mAugments) if (i) v += i->getArmorClass(); return getArmorClass() + v; }

	// Returns the health regeneration on this Item.
	inline const i32 getHealthRegen() const { return mItemData->mHealthRegen; }

	// Returns the total health regeneration on this Item and augments.
	inline const i32 _getHealthRegen() const { i32 v = getHealthRegen(); for (auto i : mAugments) if (i) v += i->getHealthRegen(); return v; }

	// Returns the mana regeneration on this Item.
	inline const i32 getManaRegen() const { return mItemData->mManaRegen; }

	// Returns the total mana regeneration on this Item and augments.
	inline const i32 _getManaRegen() const { i32 v = getManaRegen(); for (auto i : mAugments) if (i) v += i->getManaRegen(); return v; }
	
	// Returns the endurance regeneration on this Item.
	inline const i32 getEnduranceRegen() const { return mItemData->mEnduranceRegen; }
	
	// Returns the total endurance regeneration on this Item and augments.
	inline const i32 _getEnduranceRegen() const { i32 v = getEnduranceRegen(); for (auto i : mAugments) if (i) v += i->getEnduranceRegen(); return v; }
	
	// Returns the class restrictions on this Item.
	inline const u32 getClasses() const { return mItemData->mClasses; }
	
	// Returns the class restrictions on this Item and augments.
	inline const u32 _getClasses() const { u32 v = getClasses(); for (auto i : mAugments) if (i) v &= i->getSlots(); return v; }
	
	// Returns the race restrictions on this Item.
	inline const u32 getRaces() const { return mItemData->mRaces; }
	
	// Returns the race restrictions on this Item and augments.
	inline const u32 _getRaces() const { u32 v = getRaces(); for (auto i : mAugments) if (i) v &= i->getRaces(); return v; }
	
	// Returns the deity restrictions on this Item.
	inline const u32 getDeities() const { return mItemData->mDeities; }
	
	// Returns the deity restrictions on this Item and augments.
	inline const u32 _getDeities() const { u32 v = getDeities(); for (auto i : mAugments) if (i) v &= i->getDeities(); return v; }
	
	inline const i32 getSkillModAmount() const { return mItemData->mSkillModAmount; }
	inline const u32 getSkillMod() const { return mItemData->mSkillMod; }
	inline const u32 getBaneRace() const { return mItemData->mBaneDamageRace; }
	inline const u32 getBaneBodyType() const { return mItemData->mBaneDamageBodyType; }
	inline const u32 getBaneRaceAmount() const { return mItemData->mBaneDamageRaceAmount; }
	inline const i32 getBaneBodyTypeAmount() const { return mItemData->mBaneDamageBodyTypeAmount; }
	inline const u8 isMagic() const { return mItemData->mMagic; }
	inline const u32 getFoodSize() const { return mItemData->mFoodSize; }
	inline const u32 getDrinkSize() const { return mItemData->mDrinkSize; }
	inline const i32 getCastTime() const { return mItemData->mCastTime; }
	inline const u32 getRequiredLevel() const { return mItemData->mRequiredLevel; }
	inline const u32 getRecLevel() const { return mItemData->mRecLevel; }
	inline const u32 getRecSkill() const { return mItemData->mRecSkill; }
	inline const u32 getBardType() const { return mItemData->mBardType; }
	inline const i32 getBardValue() const { return mItemData->mBardValue; }
	inline const u8 getLight() const { return mItemData->mLight; }
	inline const u8 getDelay() const { return mItemData->mDelay; }
	
	// Returns the elemental damage type of this Item. See Constants.h ElementalDamageType enumeration.
	inline const u8 getElementalDamageType() const { return mItemData->mElementalDamageType; }
	
	// Returns the elemental damage on this Item.
	inline const u8 getElementalDamage() const { return mItemData->mElementalDamage; }

	// Returns the elemental damage of pType on this Item and augments. See Constants.h ElementalDamageType enumeration.
	inline const u32 _getElementalDamage(const u8 pType) {
		u32 v = getElementalDamageType() == pType ? getElementalDamage() : 0;
		for (auto i : mAugments)
			if (i && i->getElementalDamageType() == pType)
				v += i->getElementalDamage();

		return v;
	}
	
	// Returns the range on this Item.
	inline const u8 getRange() const { return mItemData->mRange; }

	// Returns the damage on this Item.
	inline const u32 getDamage() const { return mItemData->mDamage; }

	// Returns the total damage on this Item and augments.
	inline const u32 _getDamage() const { u32 v = getDamage(); for (auto i : mAugments) if (i) v += i->getDamage(); return v; }

	// Returns the colour of this Item.
	inline const u32 getColour() const { return mItemData->mColour; }

	// Returns the type of Item. See Constants.h ItemType enumeration.
	inline const u8 getItemType() const { return mItemData->mItemType; }

	// Returns the material on this Item. See Constants.h ItemMaterial enumeration.
	inline const u32 getMaterial() const { return mItemData->mMaterial; }
	
	// TODO: Not sure!
	inline const u32 getEliteMaterial() const { return mItemData->mEliteMaterial; }

	// TODO:
	inline const float getSellRate() const { return mItemData->mSellRate; }

	// (MOD2) Returns the combat effects on this Item.
	inline const i32 getCombatEffects() const { return mItemData->mCombatEffects; }

	// (MOD2) Returns the total combat effects on this Item and augments.
	inline const i32 _getCombatEffects() const { i32 count = 0; for (auto i : mAugments) if (i) count += i->getCombatEffects(); return getCombatEffects() + count; }

	// (MOD2) Returns the shielding on this Item.
	inline const i32 getShielding() const { return mItemData->mShielding; }

	// (MOD2) Returns the total shielding on this Item and augments.
	inline const i32 _getShielding() const { i32 count = 0; for (auto i : mAugments) if (i) count += i->getShielding(); return getShielding() + count; }

	// (MOD2) Returns the stun resist on this Item.
	inline const i32 getStunResist() const { return mItemData->mStunResist; }

	// (MOD2) Returns the total stun resist on this Item and augments.
	inline const i32 _getStunResist() const { i32 count = 0; for (auto i : mAugments) if (i) count += i->getStunResist(); return getStunResist() + count; }

	// (MOD2) Returns the strike through on this Item.
	inline const i32 getStrikeThrough() const { return mItemData->mStrikeThrough; }

	// (MOD2) Returns the total strike through on this Item and augments.
	inline const i32 _getStrikeThrough() const { i32 count = 0; for (auto i : mAugments) if (i) count += i->getStrikeThrough(); return getStrikeThrough() + count; }


	// TODO: I need to see whether augments stack this.
	inline const i32 getSkillDamageMod() const { return mItemData->mSkillDamageMod; }
	inline const i32 getSkillDamageModAmount() const { return mItemData->mSkillDamageModAmount; }

	// (MOD2) Returns the spell shield on this Item.
	inline const i32 getSpellShield() const { return mItemData->mSpellShield; }

	// (MOD2) Returns the total spell shield on this Item and augments.
	inline const i32 _getSpellShield() const { i32 count = 0; for (auto i : mAugments) if (i) count += i->getSpellShield(); return getSpellShield() + count; }

	// (MOD2) Returns the avoidance on this Item.
	inline const i32 getAvoidance() const { return mItemData->mAvoidance; }

	// (MOD2) Returns the total avoidance on this Item and augments.
	inline const i32 _getAvoidance() const { i32 count = 0; for (auto i : mAugments) if (i) count += i->getAvoidance(); return getAvoidance() + count; }

	// (MOD2) Returns the accuracy on this Item.
	inline const i32 getAccuracy() const { return mItemData->mAccuracy; }

	// (MOD2) Returns the total accuracy on this Item and augments.
	inline const i32 _getAccuracy() const { i32 count = 0; for (auto i : mAugments) if (i) count += i->getAccuracy(); return getAccuracy() + count; }
	
	// TODO: Faction Modifiers
	
	inline const u32 getAugmentationType() const { return mItemData->mAugType; }
	inline const u32 getAugmentationRestriction() const { return mItemData->mAugmentationRestriction; }
	
	// Augmentation Slots
	
	inline const u32 getLDONPointType() const { return mItemData->mLDONPointType; }
	inline const u32 getLDONTheme() const { return mItemData->mLDONTheme; }
	inline const u32 getLDONPrice() const { return mItemData->mLDONPrice; }
	inline const u32 getLDONSellBackPrice() const { return mItemData->mLDONSellBackPrice; }
	inline const u32 getLDONSold() const { return mItemData->mLDONSold; }

	// Returns the container type of this Item. See Constants.h ContainerType enumeration.
	inline const u8 getContainerType() const { return mItemData->mContainerType; }

	// Returns the number of container slots this Item has. 1-10.
	inline const u8 getContainerSlots() const { return mItemData->mContainerSlots; }

	// Returns the container size. See Constants.h ContainerSize enumeration.
	inline const u8 getContainerSize() const { return mItemData->mContainerSize; }

	// Returns the container weight reduction. 0-100.
	inline const u8 getContainerWR() const { return mItemData->mContainerWR; }

	inline const u8 getBook() const { return mItemData->mBook; }
	inline const u8 getBookType() const { return mItemData->mBookType; }
	inline const i32 getLoreGroup() const { return mItemData->mLoreGroup; }
	inline const bool isArtifact() const { return mItemData->mArtifact == 1; }
	inline const bool isSummoned() const { return mItemData->mSummoned == 1; }
	inline const u32 getFavor() const { return mItemData->mFavor; }
	inline const bool isFVNoDrop() const { return mItemData->mFVNoDrop == 1; }
	
	// Returns the DoT shield on this Item.
	inline const i32 getDoTShield() const { return mItemData->mDoTShield; }

	// Returns the total DoT shield on this Item and augments.
	inline const i32 _getDoTShield() const { i32 count = 0; for (auto i : mAugments) if (i) count += i->getDoTShield(); return getDoTShield() + count; }

	// TODO: Does attack stack with augs?
	inline const i32 getAttack() const { return mItemData->mAttack; }
	
	inline const i32 getHaste() const { return mItemData->mHaste; }
	inline const i32 getDamageShield() const { return mItemData->mDamageShield; }
	inline const u32 getGuildFavor() const { return mItemData->mGuildFavor; }
	inline const u32 getAugmentationDistiller() const { return mItemData->mAugmentationDistiller; }
	inline const u8 getIsNoPet() const { return mItemData->mNoPet; }
	// TODO: potion belt
	inline const bool isStackable() const { return getMaxStacks() > 1; }
	inline const u32 getMaxStacks() const { return mItemData->mMaxStacks; }
	inline const bool isNoTransfer() const { return mItemData->mNoTransfer == 1; }
	// TODO: expendablearrow
	// TODO: Effects
	inline const u32 getScriptFileID() const { return mItemData->mScriptFileID; }
	inline const u8 getIsQuest() const { return mItemData->mQuest; }
	inline const u32 getMaxPower() const { return mItemData->mMaxPower; }
	inline const u32 getPurity() const { return mItemData->mPurity; }
	// TODO: Power
	// TODO: Purity
	inline const u32 getBackstabDamage() const { return mItemData->mBackstabDamage; }
	inline const u32 getDamageShieldMitigation() const { return mItemData->mDamageShieldMitigation; }

	// Returns the heroic strength on this Item.
	inline const i32 getHeroicStrength() const { return mItemData->mHeroicStrength; }

	// Returns the total heroic strength on this Item and augments.
	inline const i32 _getHeroicStrength() const { i32 count = 0; for (auto i : mAugments) if (i) count += i->getHeroicStrength(); return getHeroicStrength() + count; }

	// Returns the heroic intelligence on this Item.
	inline const i32 getHeroicIntelligence() const { return mItemData->mHeroicIntelligence; }

	// Returns the total heroic intelligence on this Item and augments.
	inline const i32 _getHeroicIntelligence() const { i32 count = 0; for (auto i : mAugments) if (i) count += i->getHeroicIntelligence(); return getHeroicIntelligence() + count; }

	// Returns the heroic wisdom on this Item.
	inline const i32 getHeroicWisdom() const { return mItemData->mHeroicWisdom; }

	// Returns the total heroic wisdom on this Item and augments.
	inline const i32 _getHeroicWisdom() const { i32 count = 0; for (auto i : mAugments) if (i) count += i->getHeroicWisdom(); return getHeroicWisdom() + count; }

	// Returns the heroic agility on  this Item.
	inline const i32 getHeroicAgility() const { return mItemData->mHeroicAgility; }

	// Returns the total heroic agility on this Item.
	inline const i32 _getHeroicAgility() const { i32 count = 0; for (auto i : mAugments) if (i) count += i->getHeroicAgility(); return getHeroicAgility() + count; }

	// Returns the heroic dexterity on this Item.
	inline const i32 getHeroicDexterity() const { return mItemData->mHeroicDexterity; }

	// Returns the total heroic dexterity on this Item and augments.
	inline const i32 _getHeroicDexterity() const { i32 count = 0; for (auto i : mAugments) if (i) count += i->getHeroicDexterity(); return getHeroicDexterity() + count; }

	// Returns the heroic stamina on this Item.
	inline const i32 getHeroicStamina() const { return mItemData->mHeroicStamina; }

	// Returns the total heroic stamina on this Item and augments.
	inline const i32 _getHeroicStamina() const { i32 count = 0; for (auto i : mAugments) if (i) count += i->getHeroicStamina(); return getHeroicStamina() + count; }

	// Returns the heroic charisma on this Item.
	inline const i32 getHeroicCharisma() const { return mItemData->mHeroicCharisma; }

	// Returns the total heroic charisma on this Item and augments.
	inline const i32 _getHeroicCharisma() const { i32 count = 0; for (auto i : mAugments) if (i) count += i->getHeroicCharisma(); return getHeroicCharisma() + count; }

	// Returns the heroic magic resist on this Item.
	inline const i32 getHeroicMagicResist() const { return mItemData->mHeroicMagicResist; }
	
	// Returns the total heroic magic resist on this Item and augments.
	inline const i32 _getHeroicMagicResist() const { i32 count = 0; for (auto i : mAugments) if (i) count += i->getHeroicMagicResist(); return getHeroicMagicResist() + count; }

	// Returns the heroic fire resist on this Item.
	inline const i32 getHeroicFireResist() const { return mItemData->mHeroicFireResist; }

	// Returns the total heroic fire resist on this Item and augments.
	inline const i32 _getHeroicFireResist() const { i32 count = 0; for (auto i : mAugments) if (i) count += i->getHeroicFireResist(); return getHeroicFireResist() + count; }

	// Returns the heroic cold resist on this Item.
	inline const i32 getHeroicColdResist() const { return mItemData->mHeroicColdResist; }

	// Returns the total heroic cold resist on this Item and augments.
	inline const i32 _getHeroicColdResist() const { i32 count = 0; for (auto i : mAugments) if (i) count += i->getHeroicColdResist(); return getHeroicColdResist() + count; }

	// Returns the heroic disease resist on this Item.
	inline const i32 getHeroicDiseaseResist() const { return mItemData->mHeroicDiseaseResist; }

	// Returns the total heroic disease resist on this Item and augments.
	inline const i32 _getHeroicDiseaseResist() const { i32 count = 0; for (auto i : mAugments) if (i) count += i->getHeroicDiseaseResist(); return getHeroicDiseaseResist() + count; }

	// Returns the heroic poison resist on this Item.
	inline const i32 getHeroicPoisonResist() const { return mItemData->mHeroicPoisonResist; }

	// Returns the total heroic poison resist on this Item and augments.
	inline const i32 _getHeroicPoisonResist() const { i32 count = 0; for (auto i : mAugments) if (i) count += i->getHeroicPoisonResist(); return getHeroicPoisonResist() + count; }

	// Returns the heroic corruption resist on this Item.
	inline const i32 getHeroicCorruptionResist() const { return mItemData->mHeroicSVCorruption; }

	// Returns the total heroic corruption resist on this Item and augments.
	inline const i32 _getHeroicCorruptionResist() const { i32 count = 0; for (auto i : mAugments) if (i) count += i->getHeroicCorruptionResist(); return getHeroicCorruptionResist() + count; }

	// (MOD2) Returns the heal amount on this Item.
	inline const i32 getHealAmount() const { return mItemData->mHealAmount; }

	// (MOD2) Returns the total heal amount on this Item and augments.
	inline const i32 _getHealAmount() const { i32 count = 0; for (auto i : mAugments) if (i) count += i->getHealAmount(); return getHealAmount() + count; }

	// (MOD2) Returns the spell damage on this Item.
	inline const i32 getSpellDamage() const { return mItemData->mSpellDamage; }

	// (MOD2) Returns the total spell damage on this Item and augments.
	inline const i32 _getSpellDamage() const { i32 count = 0; for (auto i : mAugments) if (i) count += i->getSpellDamage(); return getSpellDamage() + count; }

	// (MOD2) Returns the clairvoyance on this Item.
	inline const i32 getClairvoyance() const { return mItemData->mClairvoyance; }

	// (MOD2) Returns the total clairvoyance on this Item and augments.
	inline const i32 _getClairvoyance() const { i32 count = 0; for (auto i : mAugments) if (i) count += i->getClairvoyance(); return getClairvoyance() + count; }

	// Returns whether or not this Item is an heirloom.
	inline const bool isHeirloom() const { return mItemData->mHeirloom == 1; }

	inline void setIsEvolvingItem(const bool pValue) { mItemData->mIsEvolvingItem = pValue ? 1 : 0; }
	inline void setIsCopied(const u8 pValue) { mItemData->mCopied = pValue; }
	inline void setItemClass(const u8 pValue) { mItemData->mItemClass = pValue; }
	inline void setID(const u32 pValue) { mItemData->mID = pValue; }
	inline void setWeight(const u8 pValue) { mItemData->mWeight = pValue; }
	inline void setIsTemporary(const bool pValue) { mItemData->mTemporary = pValue ? 0 : 1; }
	inline void setIsNoDrop(const bool pValue) { mItemData->mNoDrop = pValue ? 0 : 1; }
	inline void setIsAttunable(const bool pValue) { mItemData->mAttunable = pValue ? 1 : 0; }
	inline void setSize(const u8 pValue) { mItemData->mSize = pValue; }
	inline void setSlots(const u32 pValue) { mItemData->mSlots = pValue; }
	inline void setPrice(const u32 pValue) { mItemData->mPrice = pValue; }
	inline void setIcon(const u32 pValue) { mItemData->mIcon = pValue; }
	inline void setBenefitFlag(const u32 pValue) { mItemData->mBenefitFlag = pValue; }
	inline void setIsTradeskillsItem(const bool pValue) { mItemData->mTradeSkills = pValue ? 1 : 0; }
	
	// Sets the cold resist on this Item.
	inline void setColdResist(const i8 pValue) { mItemData->mColdResist = pValue; }

	// Sets the disease resist on this Item.
	inline void setDiseaseResist(const i8 pValue) { mItemData->mDieaseResist = pValue; }

	// Sets the poison resist on this Item.
	inline void setPoisonResist(const i8 pValue) { mItemData->mPoisonResist = pValue; }

	// Sets the magic resist on this Item.
	inline void setMagicResist(const i8 pValue) { mItemData->mMagicResist = pValue; }

	// Sets the fire resist on this Item.
	inline void setFireResist(const i8 pValue) { mItemData->mFireResist = pValue; }

	// Sets the corruption resist on this Item.
	inline void setCorruptionResist(const i8 pValue) { mItemData->mCorruptionResist = pValue; }

	// Sets the strength on this Item.
	inline void setStrength(const i8 pValue) { mItemData->mStrength = pValue; }

	// Sets the stamina on this Item.
	inline void setStamina(const i8 pValue) { mItemData->mStamina = pValue; }
	
	// Sets the agility on this Item.
	inline void setAgility(const i8 pValue) { mItemData->mAgility = pValue; }

	// Sets the dexterity on this Item.
	inline void setDexterity(const i8 pValue) { mItemData->mDexterity = pValue; }

	// Sets the charisma on this Item.
	inline void setCharisma(const i8 pValue) { mItemData->mCharisma = pValue; }

	// Sets the intelligence on this Item.
	inline void setIntelligence(const i8 pValue) { mItemData->mIntelligence = pValue; }

	// Sets the wisdom on this Item.
	inline void setWisdom(const i8 pValue) { mItemData->mWisdom = pValue; }

	// Sets the health on this Item.
	inline void setHealth(const i32 pValue) { mItemData->mHealth = pValue; }

	// Sets the mana on this Item.
	inline void setMana(const i32 pValue) { mItemData->mMana = pValue; }
	
	// Sets the endurance on this Item.
	inline void setEndurance(const u32 pValue) { mItemData->mEndurance = pValue; }

	// Sets the armor class on this Item.
	inline void setArmorClass(const i32 pValue) { mItemData->mArmorClass = pValue; }

	// Sets the health regeneration on this Item.
	inline void setHealthRegen(const i32 pValue) { mItemData->mHealthRegen = pValue; }

	// Sets the mana regeneration on this Item.
	inline void setManaRegen(const i32 pValue) { mItemData->mManaRegen = pValue; }

	// Sets the endurance regeneration on this Item.
	inline void setEnduranceRegen(const i32 pValue) { mItemData->mEnduranceRegen = pValue; }

	inline void setClasses(const u32 pValue) { mItemData->mClasses = pValue; }
	inline void setRaces(const u32 pValue) { mItemData->mRaces = pValue; }
	inline void setDeities(const u32 pValue) { mItemData->mDeities = pValue; }
	inline void setSkillModAmount(const i32 pValue) { mItemData->mSkillModAmount = pValue; }
	inline void setSkillMod(const u32 pType) { mItemData->mSkillMod = pType; }
	inline void setBaneRace(const u32 pValue) { mItemData->mBaneDamageRace = pValue; }
	inline void setBaneBodyType(const u32 pValue) { mItemData->mBaneDamageBodyType = pValue; }
	inline void setBaneRaceAmount(const u32 pValue) { mItemData->mBaneDamageRaceAmount = pValue; }
	inline void setBaneBodyTypeAmount(const i32 pValue) { mItemData->mBaneDamageBodyTypeAmount = pValue; }
	inline void setIsMagic(const bool pValue) { mItemData->mMagic = pValue ? 1 : 0; }
	inline void setFoodSize(const u32 pValue) { mItemData->mFoodSize = pValue; }
	inline void setDrinkSize(const u32 pValue) { mItemData->mDrinkSize = pValue; }
	inline void setCastTime(const i32 pValue) { mItemData->mCastTime = pValue; }
	inline void setRequiredLevel(const u32 pValue) { mItemData->mRequiredLevel = pValue; }
	inline void setRecLevel(const u32 pValue) { mItemData->mRecLevel = pValue; }
	inline void setRecSkill(const u32 pValue) { mItemData->mRecSkill = pValue; }
	inline void setBardType(const u32 pValue) { mItemData->mBardType = pValue; }
	inline void setBardValue(const i32 pValue) { mItemData->mBardValue = pValue; }
	inline void setLight(const u8 pValue) { mItemData->mLight = pValue; }
	inline void setDelay(const u8 pValue) { mItemData->mDelay = pValue; }
	inline void setElementalDamageType(const u8 pValue) { mItemData->mElementalDamageType = pValue; }
	inline void setElementalDamage(const u8 pValue) { mItemData->mElementalDamage = pValue; }
	inline void setRange(const u8 pValue) { mItemData->mRange = pValue; }
	inline void setDamage(const u32 pValue) { mItemData->mDamage = pValue; }
	inline void setColour(const u32 pValue) { mItemData->mColour = pValue; }
	inline void setItemType(const u8 pValue) { mItemData->mItemType = pValue; }
	inline void setMaterial(const u32 pValue) { mItemData->mMaterial = pValue; }
	inline void setEliteMaterial(const u32 pValue) { mItemData->mEliteMaterial = pValue; }
	inline void setSellRate(const float pValue) { mItemData->mSellRate = pValue; }
	inline void setCombatEffects(const i32 pValue) { mItemData->mCombatEffects = pValue; }
	inline void setShielding(const i32 pValue) { mItemData->mShielding = pValue; }
	inline void setStunResist(const i32 pValue) { mItemData->mStunResist = pValue; }
	inline void setStrikeThrough(const i32 pValue) { mItemData->mStrikeThrough = pValue; }
	inline void setSkillDamageMod(const i32 pValue) { mItemData->mSkillDamageMod = pValue; }
	inline void setSkillDamageModAmount(const i32 pValue) { mItemData->mSkillDamageModAmount = pValue; }
	inline void setSpellShield(const i32 pValue) { mItemData->mSpellShield = pValue; }
	inline void setAvoidance(const i32 pValue) { mItemData->mAvoidance = pValue; }
	inline void setAccuracy(const i32 pValue) { mItemData->mAccuracy = pValue; }
	// Faction Modifiers
	inline void setAugmentationType(const u32 pValue) const { mItemData->mAugType = pValue; }
	inline void setAugmentationRestriction(const u32 pValue) const { mItemData->mAugmentationRestriction = pValue; }
	// Augmentation Slots
	inline void setLDONPointType(const u32 pValue) { mItemData->mLDONPointType = pValue; }
	inline void setLDONTheme(const u32 pValue) { mItemData->mLDONTheme = pValue; }
	inline void setLDONPrice(const u32 pValue) { mItemData->mLDONPrice = pValue; }
	inline void setLDONSellBackPrice(const u32 pValue) { mItemData->mLDONSellBackPrice = pValue; }
	inline void setLDONSold(const u32 pValue) { mItemData->mLDONSold = pValue; }
	inline void setContainerType(const u8 pValue) { mItemData->mContainerType = pValue; }
	inline void setContainerSlots(const u8 pValue) { mItemData->mContainerSlots = pValue; }
	inline void setContainerSize(const u8 pValue) { mItemData->mContainerSize = pValue; }
	inline void setContainerWR(const u8 pValue) { mItemData->mContainerWR = pValue; }

	inline void setBook(const u8 pValue) { mItemData->mBook = pValue; }
	inline void setBookType(const u8 pValue) { mItemData->mBookType = pValue; }

	inline void setLoreGroup(const i32 pValue) { mItemData->mLoreGroup = pValue; }
	inline void setIsArtifact(const bool pValue) { mItemData->mArtifact = pValue ? 1 : 0; }
	inline void setIsSummoned(const bool pValue) { mItemData->mSummoned = pValue ? 1 : 0; }
	inline void setFavor(const u32 pValue) { mItemData->mFavor = pValue; }
	inline void setIsFVNoDrop(const bool pValue) { mItemData->mFVNoDrop = pValue ? 1 : 0; }
	inline void setDoTShield(const i32 pValue) { mItemData->mDoTShield = pValue; }
	inline void setAttack(const i32 pValue) { mItemData->mAttack = pValue; }
	inline void setHaste(const i32 pValue) { mItemData->mHaste = pValue; }
	inline void setDamageShield(const i32 pValue) { mItemData->mDamageShield = pValue; }
	inline void setGuildFavor(const u32 pValue) { mItemData->mGuildFavor = pValue; }
	inline void setAugmentationDistiller(const u32 pValue) { mItemData->mAugmentationDistiller = pValue; }
	inline void setIsNoPet(const u8 pValue) { mItemData->mNoPet = pValue; }
	// TODO: potion belt
	inline void setMaxStacks(const u32 pValue) { mItemData->mMaxStacks = pValue; }
	inline void setIsNoTransfer(const bool pValue) { mItemData->mNoTransfer = pValue ? 1 : 0; }
	// TODO: expendablearrow
	// TODO: Effects
	inline void setScriptFileID(const u32 pValue) { mItemData->mScriptFileID = pValue; }
	inline void setIsQuest(const u8 pValue) { mItemData->mQuest = pValue; }
	inline void setMaxPower(const u32 pValue) { mItemData->mMaxPower = pValue; }
	inline void setPurity(const u32 pValue) { mItemData->mPurity = pValue; }
	// TODO: Power
	// TODO: Purity
	inline void setBackstabDamage(const u32 pValue) { mItemData->mBackstabDamage = pValue; }
	inline void setDamageShieldMitigation(const u32 pValue) { mItemData->mDamageShieldMitigation = pValue; }

	// Sets the heroic strength on this Item.
	inline void setHeroicStrength(const i32 pValue) { mItemData->mHeroicStrength = pValue; }

	// Sets the heroic intelligence on this Item.
	inline void setHeroicIntelligence(const i32 pValue) { mItemData->mHeroicIntelligence = pValue; }

	// Sets the heroic wisdom on this Item.
	inline void setHeroicWisdom(const i32 pValue) { mItemData->mHeroicWisdom = pValue; }

	// Sets the heroic agility on this Item.
	inline void setHeroicAgility(const i32 pValue) { mItemData->mHeroicAgility = pValue; }

	// Sets the heroic dexterity on this Item.
	inline void setHeroicDexterity(const i32 pValue) { mItemData->mHeroicDexterity = pValue; }

	// Sets the heroic stamina on this Item.
	inline void setHeroicStamina(const i32 pValue) { mItemData->mHeroicStamina = pValue; }

	// Sets the heroic charisma on this Item.
	inline void setHeroicCharisma(const i32 pValue) { mItemData->mHeroicCharisma = pValue; }

	// Sets the heroic magic resist on this Item.
	inline void setHeroicMagicResist(const i32 pValue) { mItemData->mHeroicMagicResist = pValue; }

	// Sets the heroic fire resist on this Item.
	inline void setHeroicFireResist(const i32 pValue) { mItemData->mHeroicFireResist = pValue; }

	// Sets the  heroic cold resist on this Item.
	inline void setHeroicColdResist(const i32 pValue) { mItemData->mHeroicColdResist = pValue; }

	// Sets the heroic disease resist on this Item.
	inline void setHeroicDiseaseResist(const i32 pValue) { mItemData->mHeroicDiseaseResist = pValue; }

	// Sets the heroic poison resist on this Item.
	inline void setHeroicPoisonResist(const i32 pValue) { mItemData->mHeroicPoisonResist = pValue; }

	// Sets the heroic corruption resist on this Item.
	inline void setHeroicCorruptionResist(const i32 pValue) { mItemData->mHeroicSVCorruption = pValue; }

	// (MOD2) Sets the heal amount on this Item.
	inline void setHealAmount(const i32 pValue) { mItemData->mHealAmount = pValue; }

	// (MOD2) Sets the spell damage on this Item.
	inline void setSpellDamage(const i32 pValue) { mItemData->mSpellDamage = pValue; }

	// (MOD2) Sets the clairvoyance on this Item.
	inline void setClairvoyance(const i32 pValue) { mItemData->mClairvoyance = pValue; }

	// Sets whether this Item is an heirloom or not.
	inline void setIsHeirloom(const bool pValue) { mItemData->mHeirloom = pValue ? 1 : 0; }

private:
	const u32 _getDataSize(const u32 pCopyType) const;
	Item* mParent = nullptr;
	Item* mAugments[5];
	Item* mContents[10];
	ItemData* mItemData = nullptr;

	// ItemData Overrides
	u32 mSlot = 0;
	u32 mShopPrice = 0;
	i32 mShopQuantity = 0;
	u32 mStacks = 1;
	u32 mInstanceID = 0;
	u32 mLastCastTime = 0;
	u32 mCharges = 0;
	u32 mPower = 0;
	bool mAttuned = false;
	u16 mOrnamentationIcon = 0;
	String mOrnamentationIDFile = "";

	// When an Item is a sub-Item such as an augment or container contents
	// mSubIndex will be a value 0-9 that specifies the position it is stored.
	i32 mSubIndex = -1;

	bool mEvolvingActive = false;
	i32 mEvolvingLevel = 0;
	double mEvolvingProgress = 0.0f;
};