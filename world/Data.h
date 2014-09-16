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
	struct CharacterData {
		String mName = "<none>";
		uint32 mLevel = 0;
		ClassID mClass = 0;
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
	}
	String mName = "";
	uint32 mLevel = 1;
	ClassID mClass = ClassIDs::Warrior;
	uint32 mExperience = 0;
	String mLastName = "";
	String mTitle = "";
	String mSuffix = "";
	bool mGM = false;
	uint8 mStatus = 0;

	ZoneID mZoneID = 0; // TODO: Choose a good default.
	InstanceID mInstanceID = 0;
	float mX = 0.0f;
	float mY = 0.0f;
	float mZ = 0.0f;
	float mHeading = 0.0f;

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
	uint32 mPlatinumCharacter = 0;
	uint32 mPlatinumBank = 0;
	uint32 mPlatinumCursor = 0;
	uint32 mGoldCharacter = 0;
	uint32 mGoldBank = 0;
	uint32 mGoldCursor = 0;
	uint32 mSilverCharacter = 0;
	uint32 mSilverBank = 0;
	uint32 mSilverCursor = 0;
	uint32 mCopperCharacter = 0;
	uint32 mCopperBank = 0;
	uint32 mCopperCursor = 0;

	// Stats
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

	std::vector<uint32> mSpellBook;
	std::vector<uint32> mSpellBar;
};

struct SpawnGroupData {
	uint32 mID = 0;
};

struct SpawnPointData {
	Vector3 mPosition;
	float mHeading = 0.0f;
	uint32 mSpawnGroupID = 0;
	uint32 mRespawnTime = DEFAULT_RESPAWN_TIME;
};

struct NPCAppearanceData {
	uint32 mID = 0;
	uint32 mParentID = 0;

	uint32 mRaceID = 0;
	uint8 mGender = Gender::G_FEMALE;
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
};