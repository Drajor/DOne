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