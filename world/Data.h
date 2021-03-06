#pragma once

#include "TaskConstants.h"
#include "Constants.h"
#include "Vector3.h"

namespace Data {

	struct CompletedTask {
		u32 mTaskID = 0;
		u32 mCompleted = 0; // Time stamp.
	};

	struct AccountCharacter {
		String mName = "<none>";
		u32 mLevel = 0;
		u8 mClass = 0;
		u32 mRace = 0;
		u8 mGender = 0;
		u16 mDeity = 0;
		u16 mZoneID = 0;
		u8 mFaceStyle = 0;
		u8 mHairStyle = 0;
		u8 mHairColour = 0;
		u8 mBeardStyle = 0;
		u8 mBeardColour = 0;
		u8 mEyeColourLeft = 0;
		u8 mEyeColourRight = 0;
		u32 mDrakkinHeritage = 0;
		u32 mDrakkinTattoo = 0;
		u32 mDrakkinDetails = 0;
		u32 mPrimary = 0;
		u32 mSecondary = 0;
		bool mCanReturnHome = true;
		bool mCanEnterTutorial = false;
		struct Equipment {
			u32 mMaterial = 0;
			u32 mColour = 0;
		};
		Equipment mEquipment[Limits::Account::MAX_EQUIPMENT_SLOTS];
	};

	struct Account {
		Account() {
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
		void setRaceUnlocked(const u32 pRaceID, const bool pLocked) { mRacesUnlocked.push_back(std::make_pair(pRaceID, pLocked)); }

		u32 mLoginAccountID = 0;		// Provided by Login Server
		String mLoginAccountName = "";	// Provided by Login Server
		u32 mLoginServerID = 1;
		u32 mAccountID = 0; // Internal identifier.
		i8 mStatus = 0;
		i64 mSuspendedUntil = 0;
		i64 mCreated = 0;
		i32 mPlatinumSharedBank = 0;
		std::list<AccountCharacter*> mCharacterData;
		std::list<std::pair<u32, bool>> mRacesUnlocked;
		bool mCharacterDataLoaded = false;
	};

	struct Item {
		u32 mItemID = 0;
		u32 mSlot = 0;
		u32 mStacks = 1;
		u32 mCharges = 0;
		u32 mAttuned = 0;
		u32 mLastCastTime = 0;
		std::list<Item> mSubItems;
	};

	struct Inventory {
		std::list<Item> mItems;
	};

	struct BindLocation {
		u16 mZoneID = 0;
		Vector3 mPosition;
		float mHeading = 0.0f;
	};

	struct Experience {
		u8 mLevel = 0;
		u8 mMaximumLevel = 0;
		u32 mExperience = 0;
		u32 mExperienceToAA = 0;
		u32 mAAExperience = 0;
		u32 mUnspentAAPoints = 0;
		u32 mMaximumUnspentAA = 0;
		u32 mSpentAAPoints = 0;
		u32 mMaximumSpentAA = 0;
		bool mLeadershipExperienceOn = false;
		u32 mGroupPoints = 0;
		u32 mGroupExperience = 0;
		u32 mRaidPoints = 0;
		u32 mRaidExperience = 0;
	};

	struct Buff {
		u32 mSlotIndex = 0;
		u32 mSpellID = 0;
		u32 mTicksRemaining = 0;
	};

	struct Character {
		Character() {
			mSpellBook.resize(Limits::SpellBook::MAX_SLOTS);
			mSpellBar.resize(Limits::SpellBar::MAX_SLOTS);

			for (auto& i : mSkills) i = 0;
			for (auto& i : mLanguages) i = 0;
			for (auto& i : mDyes) i = 0;
		}
		String mName = "";
		Experience mExperience;
		u8 mClass = ClassID::Warrior;
		String mLastName = "";
		String mTitle = "";
		String mSuffix = "";
		String mInspectMessage = "";
		bool mGM = false;
		bool mMuted = false;
		bool mNew = true; // This flag is set to false the first time the Character enters the world.
		u32 mStatus = 0;

		u16 mZoneID = 0; // TODO: Choose a good default.
		u16 mInstanceID = 0;
		Vector3 mPosition;
		float mHeading = 0.0f;

		u32 mSkills[Limits::Skills::MAX_ID];
		u32 mLanguages[Limits::Languages::MAX_ID];

		u32 mGuildID = GuildID::None;
		u8 mGuildRank = GuildRank::None;

		u32 mDyes[MAX_ARMOR_DYE_SLOTS];

		u16 mDeity = 0;
		u32 mRace = 0;
		u8 mGender = 0;
		u8 mFaceStyle = 0;
		u8 mHairStyle = 0;
		u8 mHairColour = 0;
		u8 mBeardStyle = 0;
		u8 mBeardColour = 0;
		u8 mEyeColourLeft = 0;
		u8 mEyeColourRight = 0;
		u32 mDrakkinHeritage = 0;
		u32 mDrakkinTattoo = 0;
		u32 mDrakkinDetails = 0;

		// Currency
		i32 mPlatinumCharacter = 0;
		i32 mPlatinumBank = 0;
		i32 mPlatinumCursor = 0;
		i32 mGoldCharacter = 0;
		i32 mGoldBank = 0;
		i32 mGoldCursor = 0;
		i32 mSilverCharacter = 0;
		i32 mSilverBank = 0;
		i32 mSilverCursor = 0;
		i32 mCopperCharacter = 0;
		i32 mCopperBank = 0;
		i32 mCopperCursor = 0;

		// Base Stats
		u32 mStrength = 0;
		u32 mStamina = 0;
		u32 mCharisma = 0;
		u32 mDexterity = 0;
		u32 mIntelligence = 0;
		u32 mAgility = 0;
		u32 mWisdom = 0;

		BindLocation mBindLocations[5];

		bool mAutoConsentGroup = false;
		bool mAutoConsentRaid = false;
		bool mAutoConsentGuild = false;

		u32 mRadiantCrystals = 0;
		u32 mTotalRadiantCrystals = 0;
		u32 mEbonCrystals = 0;
		u32 mTotalEbonCrystals = 0;
		std::map<u32, u32> mAlternateCurrency;

		std::vector<u32> mSpellBook;
		std::vector<u32> mSpellBar;
		std::list<Buff*> mBuffs;
		std::list<CompletedTask> mCompletedTasks;

		Inventory mInventory;
	};

	struct GuildMember {
		String mName;
		u8 mRank = 0;
		u8 mLevel = 0;
		u8 mClass = 0;
		bool mTributeEnabled = false;
		u32 mFlags = 0;
		u32 mTimeLastOn = 0;
		u32 mTotalTribute = 0;
		u32 mLastTribute = 0;
		String mPublicNote;
	};

	struct Guild {
		u32 mID = 0;
		String mName;
		String mMOTD;
		String mMOTDSetter;
		String mURL;
		String mChannel;
		std::list<GuildMember*> mMembers;
	};

	struct NPCAppearance {
		NPCAppearance() {
			mOverrides.set(0);
			memset(&mColours, 0, sizeof(mColours));
		}
		u32 mID = 0;
		u32 mParentID = 0;

		u32 mRaceID = 0;
		u8 mGender = Gender::Female;
		u8 mTexture = 0;
		u8 mBodyType = BodyType::BT_Humanoid;
		float mSize = 5.0f;

		u8 mFaceStyle = 0;
		u8 mHairStyle = 0;
		u8 mHairColour = 0;
		u8 mBeardStyle = 0;
		u8 mBeardColour = 0;
		u8 mEyeColourLeft = 0;
		u8 mEyeColourRight = 0;
		u32 mDrakkinHeritage = 0;
		u32 mDrakkinTattoo = 0;
		u32 mDrakkinDetails = 0;

		u8 mHelmTexture = 0;

		u32 mPrimaryMaterial = 0;
		u32 mSecondaryMaterial = 0;

		u32 mColours[7];

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

	struct NPCType {
		u32 mID = 0;
		u32 mAppearanceID = 0;

		String mName = "";
		String mLastName = "";
		u8 mClass = 0;
		u32 mShopID = 0;
	};

	struct Shop {
		u32 mID = 0;
		std::map<u32, i32> mItems;
	};

	struct SpellEffect {
		u32 mBaseValue = 0;
		u32 mLimitValue = 0;
		u32 mMaximum = 0;
		u32 mFormula = 0;
		u32 mType = 0;
	};

	struct SpellComponent {
		u32 mItemID = 0;
		u32 mStacks = 0;
	};

	struct Spell {

		bool mInUse = false;
		u32 mID = 0;
		String mName = "";
		String mPlayer = "";
		String mTeleportZone = "";

		String mYouCastMessage = "";
		String mOtherCastsMessage = "";
		String mCastOnYouMessage = ""; // "You are completely healed."
		String mCastOnOtherMessage = ""; // " is completely healed."
		String mFadeMessage = "";

		i32 mManaCost = 0;
		u32 mIcon = 0;
		u32 mMemIcon = 0;

		u32 mRange = 0;
		u32 mAOERange = 0;
		u32 mPushBack = 0;
		u32 mPushUp = 0;
		u32 mBuffDurationFormula = 0;
		u32 mBuffDuration = 0;
		u32 mAOEDuration = 0;
		
		u32 mCastTime = 0;
		u32 mRecoveryTime = 0;
		u32 mRecastTime = 0;

		u32 mTargetType = 0; // TODO: Enumeration.

		u8 mLevels[Limits::Character::MAX_CLASS_ID];
		u8 mDeities[16];

		std::vector<SpellEffect*> mEffects;
		std::vector<SpellComponent*> mComponents;

	};

	struct Fog {
		u8 mRed = 0;
		u8 mGreen = 0;
		u8 mBlue = 0;
		float mMinimumClip = 0.0f;
		float mMaximumClip = 0.0f;
	};

	struct Weather {
		u8 mRainChance = 0;
		u8 mRainDuration = 0;
		u8 mSnowChance = 0;
		u8 mSnowDuration = 0;
	};

	struct Object {
		u32 mType = 0; // TODO: Enumeration?
		String mAsset;
		float mSize = 1.0f;
		float mHeading = 0.0f;
		Vector3 mPosition;
	};

	struct Door {
		String mAsset;
		float mHeading = 0.0f;
		Vector3 mPosition;
	};

	struct SpawnGroup {
		~SpawnGroup() {
			for (auto i : mEntries) delete i;
			mEntries.clear();
		}
		struct Entry {
			u32 mNPCType = 0;
			u32 mChance = 0;
		};

		u32 mID = 0;
		std::list<Entry*> mEntries;
	};

	struct SpawnPoint {
		Vector3 mPosition;
		float mHeading = 0.0f;
		u32 mSpawnGroupID = 0;
		u32 mRespawnTime = DEFAULT_RESPAWN_TIME;
	};

	struct ZonePoint {
		u16 mID = 0;
		Vector3 mPosition;
		float mHeading = 0.0f;

		u16 mDestinationZoneID = 0;
		u16 mDestinationInstanceID = 0;
		Vector3 mDestinationPosition;
		float mDestinationHeading = 0.0f;
	};

	struct Zone {
		~Zone() {
			for (auto i : mZonePoints) delete i;
			mZonePoints.clear();

			for (auto i : mObjects) delete i;
			mObjects.clear();

			for (auto i : mDoors) delete i;
			mDoors.clear();

			for (auto i : mSpawnGroups) delete i;
			mSpawnGroups.clear();

			for (auto i : mSpawnPoints) delete i;
			mSpawnPoints.clear();
		}
		u16 mID = 0;
		u32 mLongNameStringID = 0;
		String mLongName = "";
		String mShortName = "";
		float mMinimumClip = 200.0f;
		float mMaximumClip = 1000.0f;
		u8 mZoneType = 0;
		u8 mTimeType = 0;
		u8 mSkyType = 0;
		Vector3 mSafePosition;
		float mFogDensity = 0.0f;
		Fog mFog[4];
		Weather mWeather[4];
		std::list<ZonePoint*> mZonePoints;
		std::list<Object*> mObjects;
		std::list<Door*> mDoors;
		std::list<SpawnGroup*> mSpawnGroups;
		std::list<SpawnPoint*> mSpawnPoints;
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
		u32 mItemID = 0;
		String mAttribute = "";
		i32 mMinimum = 0;
		i32 mMaximum = 0;
		u8 mRequiredLevel = 0;
	};

	struct AlternateCurrency {
		u32 mCurrencyID = 0;
		u32 mItemID = 0;
		u32 mIcon = 0;
		u32 mMaxStacks = 0;
	};

	struct ItemAppearanceType {

	};

	struct ItemAppearance {
		u16 mIcon = 0;
		String IDFile = "IT63";
		u32 mMaterial = 0;
		String mNames;
	};

	struct Title {
		u32 mID = 0;
		String mPrefix;
		String mSuffix;
	};

	struct TaskObjective {
		u32 mIndex = 0;
		u32 mType = 0;
		u32 mZoneID = 0; // 0 = ALL
		u32 mRequired = 1;
		bool mOptional = false;
		bool mHidden = false;
		String mTextA;
		String mTextB;
		String mTextC; // This overrides mTextA / mTextB

		u32 mNPCTypeID = 0;
		u32 mItemID = 0;
	};

	struct TaskStage {
		u32 mIndex = 0;
		std::list<TaskObjective*> mObjectives;
	};

	struct Task {
		u32 mID = 0;
		u32 mType = TaskType::Quest;
		u8 mRewardType = TaskRewardType::Simple;
		u32 mCurrencyReward = 0; // Reward in copper pieces.
		i32 mPointsReward = 0;
		u32 mDuration = 0; // Seconds.
		String mTitle;
		String mDescription;
		String mRewardText;
		bool mRepeatable = true;

		TaskStagesData mStages;
	};
}
