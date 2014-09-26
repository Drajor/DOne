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

	// BEGIN chunk0
	uint32 mStackSize = 0;
	uint32 mUnknown0 = 0;
	uint32 mSlot = 0; // NOT USED.
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
	// END chunk0

	char mItemName[64];
	char mLore[80];
	char mIDFile[30];

	// BEGIN chunk1
	uint32 mID = 0;
	uint8 mWeight = 0;
	uint8 mNoRent = 0;
	uint8 mNoDrop = 0;
	uint8 mAttuneable = 0;
	uint8 mSize = 0;
	uint32 mSlots = 0;
	uint32 mPrice = 0;
	uint32 mIcon = 0;
	uint8 mUnknown10 = 0;
	uint8 mUnknown11 = 0;
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
	uint32 mUnknown12 = 0;
	uint32 mSkillModifierType = 0;
	uint32 mBaneDamageRace = 0;
	uint32 mBaneDamageBodyType = 0;
	uint32 mBaneDamageRaceAmount = 0;
	int32 mBaneDamageAmount = 0;
	uint8 mMagic = 0;
	int32 CastTime_ = 0;
	uint32 mReqLevel = 0;
	uint32 mRecLevel = 0;
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
	uint32 unknown13 = 0;
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
	// END chunk1

	char mCharmFile[32];

	// BEGIN chunk2
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
	// END chunk2

	char mFileName[33];

	// BEGIN chunk3
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
	int32 mUnknown14 = 0;
	uint32 mUnknown15 = 0;
	uint8 no_pet = 0;
	uint8 mUnknown16 = 0;

	uint8 potion_belt_enabled = 0;
	uint32 potion_belt_slots = 0;

	uint32 stacksize = 0;
	uint8 no_transfer = 0;
	uint16 expendablearrow = 0;

	uint32 mUnknown17 = 0;
	uint32 mUnknown18 = 0;
	uint32 mUnknown19 = 0;
	uint32 mUnknown20 = 0;
	uint8 unknown21 = 0;
	uint8 unknown22 = 0;
	uint8 unknown23 = 0;
	// END chunk3

	struct ClickEffect {
		int32 effect = 0;
		uint8 level2 = 0;
		uint32 type = 0;
		uint8 level = 0;
		int32 max_charges = 0;
		int32 cast_time = 0;
		uint32 recast = 0;
		int32 recast_type = 0;
		uint32 clickunk5 = 0;
	};
	ClickEffect mClickEffectStruct;
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
	};
	ProcEffectStruct mProcEffectStruct;
	char mProcName[65];
	int32 mProcUnknown = 0;

	struct Effect {
		uint32 effect = 0;
		uint8 level2 = 0;
		uint32 type = 0;
		uint8 level = 0;
		uint32 unknown1 = 0;
		uint32 unknown2 = 0;
		uint32 unknown3 = 0;
		uint32 unknown4 = 0;
		uint32 unknown5 = 0;
	};
	Effect mWornEffectStruct;
	char mWornName[65];
	int32 mWornUnknown = 0;

	Effect mFocusEffect;
	char mFocusName[65];
	int32 mFocusUnknown = 0;

	Effect mScrollEffect;
	char mScrollName[65];
	int32 mScrollUnknown = 0;

	Effect mBardEffect;
	char mBardName[65];
	int32 mBardUnknown = 0;

	// BEGIN chunk4
	uint32 scriptfileid = 0;
	uint8 quest_item = 0;
	uint32 unknown24 = 0; //0xffffffff - Power Source Capacity?
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
	uint8 unknown25 = 0;	//Power Source Capacity or evolve filename?
	uint32 evolve_string = 0; // Some String, but being evolution related is just a guess
	uint8 unknown26 = 0;
	uint32 unknown27 = 0;	// Bard Stuff?
	uint32 unknown28 = 0;
	uint32 unknown29 = 0;
	// END chunk4
	uint32 subitem_count = 0; // NOT USED.
};

#pragma pack()