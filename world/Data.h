#pragma once

#include "Constants.h"
#include "Vector3.h"

struct AccountData {
	AccountData() {
		setRaceUnlocked(PlayableRaceIDs::Human, true);
		setRaceUnlocked(PlayableRaceIDs::Barbarian, false);
		setRaceUnlocked(PlayableRaceIDs::Erudite, false);
		setRaceUnlocked(PlayableRaceIDs::WoodElf, false);
		setRaceUnlocked(PlayableRaceIDs::HighElf, false);
		setRaceUnlocked(PlayableRaceIDs::DarkElf, false);
		setRaceUnlocked(PlayableRaceIDs::HalfElf, false);
		setRaceUnlocked(PlayableRaceIDs::Dwarf, false);
		setRaceUnlocked(PlayableRaceIDs::Troll, false);
		setRaceUnlocked(PlayableRaceIDs::Ogre, false);
		setRaceUnlocked(PlayableRaceIDs::Halfling, false);
		setRaceUnlocked(PlayableRaceIDs::Iksar, false);
		setRaceUnlocked(PlayableRaceIDs::Vahshir, false);
		setRaceUnlocked(PlayableRaceIDs::Froglok, false);
		setRaceUnlocked(PlayableRaceIDs::Drakkin, false);
	}
	void setRaceUnlocked(const uint32 pRaceID, const bool pLocked) { mRacesUnlocked.push_back(std::make_pair(pRaceID, pLocked)); }

	uint32 mAccountID = 0;		// Provided by Login Server
	String mAccountName = "";	// Provided by Login Server
	AccountStatus mStatus = 0;
	uint32 mSuspendedUntil = 0;
	uint32 mCreated = 0;
	int32 mPlatinumSharedBank = 0;
	struct CharacterData {
		String mName = "<none>";
		uint32 mLevel = 0;
		uint8 mClass = 0;
		RaceID mRace = 0;
		GenderID mGender = 0;
		DeityID mDeity = 0;
		ZoneID mZoneID = 0;
		FaceID mFaceStyle = 0;
		uint8 mHairStyle = 0;
		uint8 mHairColour = 0;
		uint8 mBeardStyle = 0;
		uint8 mBeardColour = 0;
		uint8 mEyeColourLeft = 0;
		uint8 mEyeColourRight = 0;
		uint32 mDrakkinHeritage = 0;
		uint32 mDrakkinTattoo = 0;
		uint32 mDrakkinDetails = 0;
		uint32 mPrimary = 0;
		uint32 mSecondary = 0;
		struct Equipment {
			uint32 mMaterial = 0;
			uint32 mColour = 0;
		};
		Equipment mEquipment[Limits::Account::MAX_EQUIPMENT_SLOTS];
	};
	std::list<CharacterData*> mCharacterData;
	std::list<std::pair<uint32, bool>> mRacesUnlocked;
	bool mCharacterDataLoaded = false;
};

struct CharacterData {
	CharacterData() {
		mSpellBook.resize(Limits::SpellBook::MAX_SLOTS);
		mSpellBar.resize(Limits::SpellBar::MAX_SLOTS);

		for (auto& i : mSkills) i = 0;
		for (auto& i : mLanguages) i = 0;
	}
	String mName = "";
	uint32 mLevel = 1;
	uint8 mClass = ClassID::Warrior;
	uint32 mExperience = 0;
	String mLastName = "";
	String mTitle = "";
	String mSuffix = "";
	bool mGM = false;
	uint8 mStatus = 0;

	ZoneID mZoneID = 0; // TODO: Choose a good default.
	uint16 mInstanceID = 0;
	float mX = 0.0f;
	float mY = 0.0f;
	float mZ = 0.0f;
	float mHeading = 0.0f;

	uint32 mSkills[Limits::Skills::MAX_ID];
	uint32 mLanguages[Limits::Languages::MAX_ID];

	GuildID mGuildID = NO_GUILD;
	GuildRank mGuildRank = GuildRanks::GR_None;

	uint32 mDyes[MAX_ARMOR_DYE_SLOTS];

	uint16 mDeity = 0;
	RaceID mRace = 0;
	GenderID mGender = 0;
	FaceID mFaceStyle = 0;
	uint8 mHairStyle = 0;
	uint8 mHairColour = 0;
	uint8 mBeardStyle = 0;
	uint8 mBeardColour = 0;
	uint8 mEyeColourLeft = 0;
	uint8 mEyeColourRight = 0;
	uint32 mDrakkinHeritage = 0;
	uint32 mDrakkinTattoo = 0;
	uint32 mDrakkinDetails = 0;

	// Currency
	int32 mPlatinumCharacter = 0;
	int32 mPlatinumBank = 0;
	int32 mPlatinumCursor = 0;
	int32 mGoldCharacter = 0;
	int32 mGoldBank = 0;
	int32 mGoldCursor = 0;
	int32 mSilverCharacter = 0;
	int32 mSilverBank = 0;
	int32 mSilverCursor = 0;
	int32 mCopperCharacter = 0;
	int32 mCopperBank = 0;
	int32 mCopperCursor = 0;

	// Base Stats
	uint32 mStrength = 0;
	uint32 mStamina = 0;
	uint32 mCharisma = 0;
	uint32 mDexterity = 0;
	uint32 mIntelligence = 0;
	uint32 mAgility = 0;
	uint32 mWisdom = 0;

	bool mAutoConsentGroup = false;
	bool mAutoConsentRaid = false;
	bool mAutoConsentGuild = false;

	uint32 mRadiantCrystals = 0;
	uint32 mTotalRadiantCrystals = 0;
	uint32 mEbonCrystals = 0;
	uint32 mTotalEbonCrystals = 0;
	std::map<uint32, uint32> mAlternateCurrency;

	std::vector<uint32> mSpellBook;
	std::vector<uint32> mSpellBar;
};

struct NPCAppearanceData {
	NPCAppearanceData() {
		mOverrides.set(0);
		memset(&mColours, 0, sizeof(mColours));
	}
	uint32 mID = 0;
	uint32 mParentID = 0;

	uint32 mRaceID = 0;
	uint8 mGender = Gender::Female;
	uint8 mTexture = 0;
	uint8 mBodyType = BodyType::BT_Humanoid;
	float mSize = 5.0f;

	FaceID mFaceStyle = 0;
	uint8 mHairStyle = 0;
	uint8 mHairColour = 0;
	uint8 mBeardStyle = 0;
	uint8 mBeardColour = 0;
	uint8 mEyeColourLeft = 0;
	uint8 mEyeColourRight = 0;
	uint32 mDrakkinHeritage = 0;
	uint32 mDrakkinTattoo = 0;
	uint32 mDrakkinDetails = 0;

	uint8 mHelmTexture = 0;

	uint32 mPrimaryMaterial = 0;
	uint32 mSecondaryMaterial = 0;

	uint32 mColours[7];

	enum Attributes {
		RaceID,
		Gender,
		Texture,
		BodyType,
		Size,
		FaceStyle,
		HairStyle,
		HairColour,
		BeardStyle,
		BeardColour,
		EyeColourLeft,
		EyeColourRight,
		DrakkinHeritage,
		DrakkinTattoo,
		DrakkinDetails,
		HelmTexture,
		PrimaryMaterial,
		SecondaryMaterial,
		MAX
	};
	std::bitset<Attributes::MAX> mOverrides;
	bool mResolved = false;
};

struct NPCTypeData {
	uint32 mID = 0;
	uint32 mAppearanceID = 0;

	String mName = "";
	String mLastName = "";
	uint8 mClass = 0;
	uint32 mShopID = 0;
};

struct ShopData {
	uint32 mID = 0;
	std::map<uint32, int32> mItems;
};

struct SpellData {
	bool mInUse = false;
	uint32 mID = 0;
	String mName = "";

	String mYouCastMessage = "";
	String mOtherCastsMessage = "";
	String mCastOnYouMessage = ""; // "You are completely healed."
	String mCastOnOtherMessage = ""; // " is completely healed."
	String mFadeMessage = "";

	int32 mManaCost = 0;

	// Class required levels.
	uint8 mWarriorLevel = 255;
	uint8 mClericLevel = 255;
	uint8 mPaladinLevel = 255;
	uint8 mRangerLevel = 255;
	uint8 mShadowKnightLevel = 255;
	uint8 mDruidLevel = 255;
	uint8 mMonkLevel = 255;
	uint8 mBardLevel = 255;
	uint8 mRogueLevel = 255;
	uint8 mShamanLevel = 255;
	uint8 mNecromancerLevel = 255;
	uint8 mWizardLevel = 255;
	uint8 mMagicianLevel = 255;
	uint8 mEnchanterLevel = 255;
	uint8 mBeastlordLevel = 255;
	uint8 mBerserkerLevel = 255;
	uint8 mRequiredClassLevels[Limits::Character::MAX_CLASS_ID];

};

namespace Data {
	struct SpawnGroup {
		struct Entry {
			u32 mNPCType = 0;
			u32 mChance = 0;
		};

		u32 mID = 0;
		std::list<Entry*> mEntries;
	};
	typedef SpawnGroup::Entry SpawnGroupEntry;
}

struct SpawnPointData {
	Vector3 mPosition;
	float mHeading = 0.0f;
	u32 mSpawnGroupID = 0;
	u32 mRespawnTime = DEFAULT_RESPAWN_TIME;
};

struct ZonePointData {
	u16 mID = 0;
	Vector3 mPosition;
	float mHeading = 0.0f;

	u16 mDestinationZoneID = 0;
	u16 mDestinationInstanceID = 0;
	Vector3 mDestinationPosition;
	float mDestinationHeading = 0.0f;
};

struct ZoneData {
	u16 mID = 0;
	u32 mLongNameStringID = 0;
	String mLongName = "";
	String mShortName = "";
	float mSafeX = 0.0f;
	float mSafeY = 0.0f;
	float mSafeZ = 0.0f;
	std::list<ZonePointData*> mZonePoints;
	std::list<Data::SpawnGroup*> mSpawnGroups;
	std::list<SpawnPointData*> mSpawnPoints;
};

namespace AffixType{
	enum {
		Strength,
		Stamina,
		Agility,
		Dexterity,
		Wisdom,
		Intelligence,
		Charisma,
		AllStats,
		PoisonResist,
		MagicResist,
		DiseaseResist,
		FireResist,
		ColdResist,
		CorruptionResist,
		AllResist,
		HeroicStrength,
		HeroicStamina,
		HeroicAgility,
		HeroicDexterity,
		HeroicWisdom,
		HeroicIntelligence,
		HeroicCharisma,
		AllHeroicStats,
		HeroicPoisonResist,
		HeroicMagicResist,
		HeroicDiseaseResist,
		HeroicFireResist,
		HeroicColdResist,
		HeroicCorruptionResist,
		AllHeroicResists,

		Health,
		Mana,
		Endurance,

		HealthRegen,
		ManaRegen,
		EnduranceRegen,

		ArmorClass,
		Haste,
	};
};

struct ItemAffix {

};

struct TransmutationComponent {
	uint32 mItemID = 0;
	String mAttribute = "";
	int32 mMinimum = 0;
	int32 mMaximum = 0;
	uint8 mRequiredLevel = 0;
};

struct AlternateCurrency {
	uint32 mCurrencyID = 0;
	uint32 mItemID = 0;
	uint32 mIcon = 0;
	uint32 mMaxStacks = 0;
};