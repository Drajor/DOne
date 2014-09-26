#pragma once

#include "Constants.h"
#pragma pack(1)


struct ItemData {
	ItemData() {
		memset(mItemName, 0, sizeof(mItemName));
		memset(mLore, 0, sizeof(mLore));
		memset(mIDFile, 0, sizeof(mIDFile));
		memset(mCharmFile, 0, sizeof(mCharmFile));
		memset(mFileName, 0, sizeof(mFileName));
		memset(mClickName, 0, sizeof(mClickName));
		memset(mProcName, 0, sizeof(mProcName));
		memset(mWornName, 0, sizeof(mWornName));
		memset(mFocusName, 0, sizeof(mFocusName));
		memset(mScrollName, 0, sizeof(mScrollName));
		memset(mBardName, 0, sizeof(mBardName));
	}
	inline const uint32 getStackSize() const { return mP0.mStackSize; }
	inline const uint32 getSlot() const { return mP0.mSlot; }
	inline const uint32 getMerchantPrice() const { return mP0.mMerchantPrice; }
	inline const uint32 getMerchantSlot() const { return mP0.mMerchantSlot; }
	inline const uint32 getInstanceID() const { return mP0.mInstanceID; }
	inline const uint32 getLastCastTime() const { return mP0.mLastCastTime; }
	inline const uint32 getCharges() const { return mP0.mCharges; }
	inline const uint32 getAttuned() const { return mP0.mAttuned; }
	inline const uint8 getItemClass() const { return mP0.mItemClass; }
	inline const uint32 getID() const { return mP1.mID; }
	inline const uint8 getWeight() const { return mP1.mWeight; }
	inline const uint8 isNoRent() const { return mP1.mNoRent; }
	inline const uint8 isNoDrop() const { return mP1.mNoDrop; }
	inline const uint8 isAttuneable() const { return mP1.mAttuneable; }
	inline const uint8 getSize() const { return mP1.mSize; }
	inline const uint32 getSlots() const { return mP1.mSlots; }
	inline const uint32 getPrice() const { return mP1.mPrice; }
	inline const uint32 getIcon() const { return mP1.mIcon; }
	inline const uint32 getBenefitFlag() const { return mP1.mBenefitFlag; }
	inline const uint8 isTradeskillsItem() const { return mP1.mTradeSkills; }
	inline const int8 getColdResist() const { return mP1.mColdResist; }
	inline const int8 getDiseaseResist() const { return mP1.mDieaseResist; }
	inline const int8 getPoisonResist() const { return mP1.mPoisonResist; }
	inline const int8 getMagicResist() const { return mP1.mMagicResist; }
	inline const int8 getFireResist() const { return mP1.mFireResist; }
	inline const int8 getSVCorruption() const { return mP1.mSVCorruption; }
	inline const int8 getStrength() const { return mP1.mStrength; }
	inline const int8 getStamina() const { return mP1.mStamina; }
	inline const int8 getAgility() const { return mP1.mAgility; }
	inline const int8 getDexterity() const { return mP1.mDexterity; }
	inline const int8 getCharisma() const { return mP1.mCharisma; }
	inline const int8 getIntelligence() const { return mP1.mIntelligence; }
	inline const uint8 getWisdom() const { return mP1.mWisdom; }
	inline const int32 getHealth() const { return mP1.mHealth; }
	inline const int32 getMana() const { return mP1.mMana; }
	inline const uint32 getEndurance() const { return mP1.mEndurance; }
	inline const int32 getArmorClass() const { return mP1.mArmorClass; }
	inline const int32 getHealthRegen() const { return mP1.mHealthRegen; }
	inline const int32 getManaRegen() const { return mP1.mManaRegen; }
	inline const int32 getEnduranceRegen() const { return mP1.mEnduranceRegen; }
	inline const uint32 getClasses() const { return mP1.mClasses; }
	inline const uint32 getRaces() const { return mP1.mRaces; }
	inline const uint32 getDeity() const { return mP1.mDeity; }
	inline const int32 getSkillModifierValue() const { return mP1.mSkillModifierValue; }
	inline const uint32 getSkillModifierType() const { return mP1.mSkillModifierType; }
	inline const uint32 getBaneDamageRace() const { return mP1.mBaneDamageRace; }
	inline const uint32 getBaneDamageBodyType() const { return mP1.mBaneDamageBodyType; }
	inline const uint32 getBaneDamageRaceAmount() const { return mP1.mBaneDamageRaceAmount; }
	inline const int32 getBaneDamageAmount() const { return mP1.mBaneDamageAmount; }
	inline const uint8 isMagic() const { return mP1.mMagic; }

	struct P0 {
		uint32 mStackSize = 0;
		uint32 mUnknown0 = 0;
		uint32 mSlot = 0;
		uint32 mMerchantPrice = 0; // Maybe vendor price?
		uint32 mMerchantSlot = 1; //1 if not a merchant item
		uint32 mUnknown1 = 0;
		uint32 mInstanceID = 0; //unique instance id if not merchant item, else is merchant slot // what.the.fuck.
		uint32 mUnknown2 = 0;
		uint32 mLastCastTime = 0;	// Unix Time from PP of last cast for this recast type if recast delay > 0
		uint32 mCharges = 0; //Total Charges an item has (-1 for unlimited)
		uint32 mAttuned = 0; // 1 if the item is no drop (attuned items)
		uint32 mUnknown3 = 0;
		uint32 mUnknown4 = 0;
		uint32 mUnknown5 = 0;
		uint32 mUnknown6 = 0;
		uint8 mUnknown7 = 0;
		uint8 mUnknown8 = 0; //0 - Add Evolving Item struct if this isn't set to 0?
		uint8 mUnknown9 = 0;
		uint8 mItemClass = 0;
	};
	P0 mP0;
	char mItemName[64];
	char mLore[80];
	char mIDFile[30];
	struct P1 {
		uint32 mID = 0;
		uint8 mWeight = 0;
		uint8 mNoRent = 0;
		uint8 mNoDrop = 0;
		uint8 mAttuneable = 0;
		uint8 mSize = 0;
		uint32 mSlots = 0;
		uint32 mPrice = 0;
		uint32 mIcon = 0;
		uint8 mUnknown0 = 0;
		uint8 mUnknown1 = 0;
		uint32 mBenefitFlag = 0;
		uint8 mTradeSkills = 0;
		int8 mColdResist = 0;
		int8 mDieaseResist = 0;
		int8 mPoisonResist = 0;
		int8 mMagicResist = 0;
		int8 mFireResist = 0;
		int8 mSVCorruption = 0;
		int8 mStrength = 0;
		int8 mStamina = 0;
		int8 mAgility = 0;
		int8 mDexterity = 0;
		int8 mCharisma = 0;
		int8 mIntelligence = 0;
		int8 mWisdom = 0;
		int32 mHealth = 0;
		int32 mMana = 0;
		uint32 mEndurance = 0;
		int32 mArmorClass = 0;
		int32 mHealthRegen = 0;
		int32 mManaRegen = 0;
		int32 mEnduranceRegen = 0;
		uint32 mClasses = 0;
		uint32 mRaces = 0;
		uint32 mDeity = 0;
		int32 mSkillModifierValue = 0;
		uint32 mUnknown2 = 0;
		uint32 mSkillModifierType = 0;
		uint32 mBaneDamageRace = 0;
		uint32 mBaneDamageBodyType = 0;
		uint32 mBaneDamageRaceAmount = 0;
		int32 mBaneDamageAmount = 0;
		uint8 mMagic = 0;
		int32 CastTime_ = 0;
		uint32 ReqLevel = 0;
		uint32 RecLevel = 0;
		uint32 RecSkill = 0;
		uint32 BardType = 0;
		int32 BardValue = 0;
		uint8 Light = 0;
		uint8 Delay = 0;
		uint8 ElemDmgType = 0;
		uint8 ElemDmgAmt = 0;
		uint8 Range = 0;
		uint32 Damage = 0;
		uint32 Color = 0;
		uint8 ItemType = 0;
		uint32 Material = 0;
		uint32 unknown7 = 0;
		uint32 EliteMaterial = 0;
		float SellRate = 0.0f;
		int32 CombatEffects = 0;
		int32 Shielding = 0;
		int32 StunResist = 0;
		int32 StrikeThrough = 0;
		int32 ExtraDmgSkill = 0;
		int32 ExtraDmgAmt = 0;
		int32 SpellShield = 0;
		int32 Avoidance = 0;
		int32 Accuracy = 0;
		uint32 CharmFileID = 0;
		struct FactionModifier {
			uint32 mFactionID = 0;
			int32 mAmount = 0;
		};
		FactionModifier mFactionModifiers[4];
	};
	P1 mP1;
	char mCharmFile[32];

	struct P2 {
		uint32 augtype = 0;
		uint32 augrestrict = 0;
		struct AugmentationSlot {
			uint32 type = 0;
			uint8 visible = 0;
			uint8 unknown = 0;
		};
		AugmentationSlot augslots[5];

		uint32 ldonpoint_type = 0;
		uint32 ldontheme = 0;
		uint32 ldonprice = 0;
		uint32 ldonsellbackrate = 0;
		uint32 ldonsold = 0;

		uint8 bagtype = 0;
		uint8 bagslots = 0;
		uint8 bagsize = 0;
		uint8 wreduction = 0;

		uint8 book = 0;
		uint8 booktype = 0;
	};
	P2 mP2;
	char mFileName[33];

	struct P3 {
		int32 loregroup = 0;
		uint8 artifact = 0;
		uint8 summonedflag = 0;
		uint32 favor = 0;
		uint8 fvnodrop = 0;
		int32 dotshield = 0;
		int32 atk = 0;
		int32 haste = 0;
		int32 damage_shield = 0;
		uint32 guildfavor = 0;
		uint32 augdistil = 0;
		int32 unknown3 = 0;
		uint32 unknown4 = 0;
		uint8 no_pet = 0;
		uint8 unknown5 = 0;

		uint8 potion_belt_enabled = 0;
		uint32 potion_belt_slots = 0;

		uint32 stacksize = 0;
		uint8 no_transfer = 0;
		uint16 expendablearrow = 0;

		uint32 unknown8 = 0;
		uint32 unknown9 = 0;
		uint32 unknown10 = 0;
		uint32 unknown11 = 0;
		uint8 unknown12 = 0;
		uint8 unknown13 = 0;
		uint8 unknown14 = 0;
	};
	P3 mP3;

	struct ClickEffectStruct
	{
		int32 effect = 0;
		uint8 level2 = 0;
		uint32 type = 0;
		uint8 level = 0;
		int32 max_charges = 0;
		int32 cast_time = 0;
		uint32 recast = 0;
		int32 recast_type = 0;
		uint32 clickunk5 = 0;
		//uint8 effect_string = 0; //unused
		//int32 clickunk7 = 0;
	};
	ClickEffectStruct mClickEffectStruct;
	char mClickName[65];
	int32 mClickUnknown = 0;

	struct ProcEffectStruct
	{
		uint32 effect = 0;
		uint8 level2 = 0;
		uint32 type = 0;
		uint8 level = 0;
		uint32 unknown1 = 0; // poison?
		uint32 unknown2 = 0;
		uint32 unknown3 = 0;
		uint32 unknown4 = 0;
		uint32 procrate = 0;
		//uint8 effect_string = 0;
		//uint32 unknown5 = 0;
	};
	ProcEffectStruct mProcEffectStruct;
	char mProcName[65];
	int32 mProcUnknown = 0;

	struct WornEffectStruct //worn, focus and scroll effect
	{
		uint32 effect = 0;
		uint8 level2 = 0;
		uint32 type = 0;
		uint8 level = 0;
		uint32 unknown1 = 0;
		uint32 unknown2 = 0;
		uint32 unknown3 = 0;
		uint32 unknown4 = 0;
		uint32 unknown5 = 0;
		//uint8 effect_string = 0;
		//uint32 unknown6 = 0;
	};
	WornEffectStruct mWornEffectStruct;
	char mWornName[65];
	int32 mWornUnknown = 0;

	WornEffectStruct mFocusEffect;
	char mFocusName[65];
	int32 mFocusUnknown = 0;

	WornEffectStruct mScrollEffect;
	char mScrollName[65];
	int32 mScrollUnknown = 0;

	WornEffectStruct mBardEffect;
	char mBardName[65];
	//uint8 mBardName = 0; // TODO: Test this out.
	int32 mBardUnknown = 0;

	struct ItemQuaternaryBodyStruct {
		uint32 scriptfileid = 0;
		uint8 quest_item = 0;
		uint32 unknown15 = 0; //0xffffffff - Power Source Capacity?
		uint32 Purity = 0;
		uint32 BackstabDmg = 0;
		uint32 DSMitigation = 0;
		int32 HeroicStr = 0;
		int32 HeroicInt = 0;
		int32 HeroicWis = 0;
		int32 HeroicAgi = 0;
		int32 HeroicDex = 0;
		int32 HeroicSta = 0;
		int32 HeroicCha = 0;
		int32 HeroicMR = 0;
		int32 HeroicFR = 0;
		int32 HeroicCR = 0;
		int32 HeroicDR = 0;
		int32 HeroicPR = 0;
		int32 HeroicSVCorrup = 0;
		int32 HealAmt = 0;
		int32 SpellDmg = 0;
		int32 clairvoyance = 0;
		uint8 unknown18 = 0;	//Power Source Capacity or evolve filename?
		uint32 evolve_string = 0; // Some String, but being evolution related is just a guess
		uint8 unknown19 = 0;
		uint32 unknown20 = 0;	// Bard Stuff?
		uint32 unknown21 = 0;
		uint32 unknown22 = 0;
		uint32 subitem_count = 0;
	};

	ItemQuaternaryBodyStruct mItemQuaternaryBodyStruct;
};

#pragma pack()