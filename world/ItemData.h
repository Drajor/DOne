#pragma once

#include "Constants.h"
#pragma pack(1)

/*

Infusible
Placeable

*/

struct ItemData {
	ItemData() {
		memset(mName, 0, sizeof(mName));
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

		strcpy(mIDFile, "IT63");
	}

	// BEGIN chunk0
	uint32 mStacks = 1; // Specified by Item.
	uint32 mUnknown0 = 0;
	uint32 mSlot = 0; // Specified by Item.
	uint32 mMerchantPrice = 0; // Maybe vendor price?
	uint32 mMerchantSlot = 1; //1 if not a merchant item
	uint32 mUnknown1 = 0;
	uint32 mInstanceID = 0; //unique instance id if not merchant item, else is merchant slot // what.the.fuck.
	uint32 mUnknown2 = 0;
	uint32 mLastCastTime = 0;	// Unix Time from PP of last cast for this recast type if recast delay > 0
	uint32 mCharges = 0; //Total Charges an item has (-1 for unlimited)
	uint32 mAttuned = 0;
	uint32 mPower = 0;
	uint32 mUnknown4 = 0;
	uint32 mUnknown5 = 0;
	uint32 mUnknown6 = 0; // A non-zero value here causes UF to crash
	uint8 mUnknown7 = 0;
	uint8 mUnknown8 = 0; //0 - Add Evolving Item struct if this isn't set to 0? //(Non-zero messes with things).
	uint8 mCopied = 0; // 0 = OFF, 1 = ON
	uint8 mItemClass = ItemClass::Common;
	// END chunk0

	char mName[64];
	char mLore[80];
	char mIDFile[30];

	// BEGIN chunk1
	uint32 mID = 0;
	uint8 mWeight = 0;
	uint8 mTemporary = 1; // 0 = ON, 1 = OFF
	uint8 mNoDrop = 1; // 0 = ON, 1 = OFF
	uint8 mAttunable = 0; // 0 = OFF, 1 = ON
	uint8 mSize = 0;
	uint32 mSlots = 0;
	uint32 mPrice = 0;
	uint32 mIcon = 0;
	uint8 mUnknown10 = 1; // 1 copied
	uint8 mUnknown11 = 1; // 1 copied
	uint32 mBenefitFlag = 0; // UNSURE
	uint8 mTradeSkills = 0; // 0 = OFF, 1 = ON
	int8 mColdResist = 0;
	int8 mDieaseResist = 0;
	int8 mPoisonResist = 0;
	int8 mMagicResist = 0;
	int8 mFireResist = 0;
	int8 mCorruptionResist = 0;
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
	uint32 mClasses = 65535;
	uint32 mRaces = 65535;
	uint32 mDeity = 0;
	int32 mSkillModAmount = 0;
	uint32 mUnknown12 = 0;
	uint32 mSkillMod = 0;
	uint32 mBaneDamageRace = 0;
	uint32 mBaneDamageBodyType = 0;
	uint32 mBaneDamageRaceAmount = 0;
	int32 mBaneDamageAmount = 0;
	uint8 mMagic = 1; // 0 = NOT MAGIC, 1 = MAGIC
	union {
		uint32 mFoodSize;
		uint32 mDrinkSize;
		int32 mCastTime = 0;
	};
	uint32 mReqLevel = 0;
	uint32 mRecLevel = 0;
	uint32 mRecSkill = 0;
	uint32 mBardType = 0;
	int32 mBardValue = 0;
	uint8 mLight = 0;
	uint8 mDelay = 0;
	uint8 mElementalDamageType = 0;
	uint8 mElementalDamageAmount = 0;
	uint8 mRange = 0;
	uint32 mDamage = 0;
	uint32 mColour = 4278190080;
	uint8 mItemType = 0;
	uint32 mMaterial = 0;
	uint32 mUnknown13 = 0;
	uint32 mEliteMaterial = 0;
	float mSellRate = 1.0f;
	int32 mCombatEffects = 0;
	int32 mShielding = 0;
	int32 mStunResist = 0;
	int32 mStrikeThrough = 0;
	int32 mSkillDamageMod = 0; // Bash, Backstab, Dragon Punch, Eagle Strike, Flying Kick, Kick, Round Kick, Tiger Claw, Frenzy.
	int32 mSkillDamageModAmount = 0;
	int32 mSpellShield = 0;
	int32 mAvoidance = 0;
	int32 mAccuracy = 0;
	uint32 CharmFileID = 0;
	struct FactionModifier {
		uint32 mFactionID = 0;
		int32 mAmount = 0;
	};
	FactionModifier mFactionModifiers[4];
	// END chunk1

	char mCharmFile[32];

	// BEGIN chunk2
	uint32 mAugType = 0;
	uint32 mAugRestrict = 0;
	struct AugmentationSlot {
		uint32 mType = 0;
		uint8 mVisible = 1;
		uint8 unknown = 0;
	};
	AugmentationSlot augslots[5];

	uint32 mLDONPointType = 0;
	uint32 mLDONTheme = 0;
	uint32 mLDONPrice = 0;
	uint32 mLDONSellBackPrice = 0;
	uint32 mLDONSold = 0;

	uint8 mContainerType = ContainerType::NONE;
	uint8 mContainerSlots = 0;
	uint8 mContainerSize = ContainerSize::Tiny;
	uint8 mContainerWR = 0; // Weight Reduction: 0% - 100%

	uint8 mBook = 0;
	uint8 mBookType = 0;
	// END chunk2

	char mFileName[33];

	// BEGIN chunk3
	int32 mLoreGroup = 0; // -1 = Lore, 0 = Not Lore, >= 1 Specific Lore Group ID.
	uint8 mArtifact = 0; // 0 = OFF, 1 = ON
	uint8 mSummoned = 0; // 0 = OFF, 1 = ON
	uint32 mFavor = 0;
	uint8 mFVNoDrop = 0;
	int32 mDoTShield = 0;
	int32 mAttack = 0;
	int32 mHaste = 0;
	int32 mDamageShield = 0;
	uint32 mGuildFavor = 0;
	uint32 augdistil = 0;
	int32 mUnknown14 = 0;
	uint32 mUnknown15 = 0;
	uint8 mNoPet = 0; // 0 = OFF, 1 = ON
	uint8 mUnknown16 = 0;

	uint8 potion_belt_enabled = 0;
	uint32 potion_belt_slots = 0;

	uint32 mMaxStacks = 1; // This is the MAX stack size.
	uint8 mNoTransfer = 0; // 0 = OFF, 1 = ON
	uint16 expendablearrow = 0;

	uint32 mUnknown17 = 0;
	uint32 mUnknown18 = 0;
	uint32 mUnknown19 = 0;
	uint32 mUnknown20 = 0;
	uint8 mUnknown21 = 0;
	uint8 mUnknown22 = 0;
	uint8 mUnknown23 = 0;
	// END chunk3

	struct ClickEffect {
		int32 mEffectID = -1;
		uint8 level2 = 0;
		uint32 type = 0;
		uint8 level = 0;
		int32 max_charges = 0;
		int32 cast_time = 0;
		uint32 recast = 0;
		int32 recast_type = 0;
		uint32 clickunk5 = 0;
	};
	ClickEffect mClickEffect;
	char mClickName[65];
	int32 mClickUnknown = -1;

	struct ProcEffectStruct {
		int32 mEffectID = -1;
		uint8 level2 = 0;
		uint32 type = 0;
		uint8 level = 0;
		uint32 unknown1 = 0; // poison?
		uint32 unknown2 = 0;
		uint32 unknown3 = 0;
		uint32 unknown4 = 0;
		uint32 procrate = 0;
	};
	ProcEffectStruct mProcEffect;
	char mProcName[65];
	int32 mProcUnknown = -1;

	struct Effect {
		int32 mEffectID = -1;
		uint8 level2 = 0;
		uint32 type = 0;
		uint8 level = 0;
		uint32 unknown1 = 0;
		uint32 unknown2 = 0;
		uint32 unknown3 = 0;
		uint32 unknown4 = 0;
		uint32 unknown5 = 0;
	};
	Effect mWornEffect;
	char mWornName[65];
	int32 mWornUnknown = -1;

	Effect mFocusEffect;
	char mFocusName[65];
	int32 mFocusUnknown = -1;

	Effect mScrollEffect;
	char mScrollName[65];
	int32 mScrollUnknown = -1;


	// http://everquest.allakhazam.com/db/item.html?item=87185
	// http://lucy.allakhazam.com/itemraw.html?id=110788
	// This item appears to use the bard effect.
	Effect mBardEffect;
	char mBardName[65];
	int32 mBardUnknown = -1;

	// BEGIN chunk4
	uint32 mScriptFileID = 0;
	uint8 mQuest = 0; // 1 = ON
	///////////////////////////////
	uint32 mMaxPower = 100;
	uint32 Purity = 0; // 1 = Purity: 100, 10 = Purity: 100 and Infusible.
	uint32 mBackstabDamage = 0;
	uint32 mDamageShieldMitigation = 10;
	int32 mHeroicStrength = 0;
	int32 mHeroicIntelligence = 0;
	int32 mHeroicWisdom = 0;
	int32 mHeroicAgility = 0;
	int32 mHeroicDexterity = 0;
	int32 mHeroicStamina = 0;
	int32 mHeroicCharisma = 0;
	int32 mHeroicMagicResist = 0;
	int32 mHeroicFireResist = 0;
	int32 mHeroicColdResist = 0;
	int32 mHeroicDiseaseResist = 0;
	int32 mHeroicPoisonResist = 0;
	int32 mHeroicSVCorruption = 0;
	int32 mHealAmount = 0;
	int32 mSpellDamage = 0;
	int32 mClairvoyance = 0;
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