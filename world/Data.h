#pragma once

#include "Constants.h"

struct AccountData {
	uint32 mAccountID = 0;		// Provided by Login Server
	String mAccountName = "";	// Provided by Login Server
	AccountStatus mStatus = 0;
	uint32 mSuspendedUntil = 0;
	uint32 mLastLogin = 0;
	uint32 mCreated = 0;
	struct CharacterData {
		String mName = "<none>";
		uint32 mLevel = 0;
		ClassID mClass = 0;
		RaceID mRace = 0;
		GenderID mGender = 0;
		DeityID mDeity = 0;
		ZoneID mZoneID = 0;
		FaceID mFace = 0;
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
	bool mCharacterDataLoaded = false;
};

struct CharacterData {
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

	RaceID mRace = 0;
	GenderID mGender = 0;
	FaceID mFace = 0;
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
};

//struct AccountData {
//	uint32 mWorldAccountID;
//	uint32 mLoginServerAccountID;
//	std::string mName;
//	std::string mCharacterName;
//	uint32 mSharedPlatinum;
//	int32 mStatus;
//	bool mGMSpeed;
//	bool mHidden;
//	// mSuspendUntil
//};