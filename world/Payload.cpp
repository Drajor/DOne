#include "Payload.h"
#include "Data.h"

#pragma pack(1)

struct Equipment {
	u32 mMaterial = 0;
	u32 mUnknown = 0; // Elite material maybe?
	u32 itemid = 0;
	u32 mColour = 0;
};
struct CharacterSelectionHeader {
	u32 mCharacterCount = 0;
	u32 mAdditionalSlots = 0;
};
struct CharacterSelectionPartA {
	u8 mLevel = 0;
	u8 mHairStyle = 0;
	u8 mGender = 0;
};

struct CharacterSelectionPartB {
	u8 mBeardStyle = 0;
	u8 mHairColour = 0;
	u8 mFaceStyle = 0;
	Equipment mEquipment[9];
	u32 mPrimary = 0;
	u32 mSecondary = 0;
	u8 mUnknown0 = 0; // Was 0xFF
	u32 mDeity = 0;
	u16 mZoneID = 0;
	u16 mInstanceID = 0;
	u8 mReturnHome = 0; // 0 = Unavailable, 1 = Available.
	u8 mUnknown1 = 0; // Was 0xFF
	u32 mRace = 0;
	u8 mEnterTutorial = 0; // 0 = Unavailable, 1 = Available.
	u8 mClass = 0;
	u8 mLeftEyeColour = 0;
	u8 mBeardColour = 0;
	u8 mRightEyeColour = 0;
	u32 mDrakkinHeritage = 0;
	u32 mDrakkinTattoo = 0;
	u32 mDrakkinDetails = 0;
	u8 mUnknown2 = 0;
};

#pragma pack()

EQApplicationPacket* Payload::makeCharacterSelection(Data::Account* pData) {
	if (!pData) return nullptr;

	const u32 numAccountCharacters = pData->mCharacterData.size();
	u32 size = 0;

	// Add name lengths.
	for (auto i : pData->mCharacterData) {
		size += i->mName.length() + 1;
	}

	size += sizeof(CharacterSelectionHeader);
	size += sizeof(CharacterSelectionPartA) * numAccountCharacters;
	size += sizeof(CharacterSelectionPartB) * numAccountCharacters;

	unsigned char * data = new unsigned char[size];
	Utility::DynamicStructure ds(data, size);

	// Header.
	CharacterSelectionHeader header;
	header.mCharacterCount = numAccountCharacters;
	header.mAdditionalSlots = 0; // Total Slots = 8 + additional slots.
	ds.write<CharacterSelectionHeader>(header);

	for (auto i : pData->mCharacterData) {
		// Part A.
		CharacterSelectionPartA pA;
		pA.mLevel = i->mLevel;
		pA.mHairStyle = i->mHairStyle;
		pA.mGender = i->mGender;
		ds.write<CharacterSelectionPartA>(pA);

		// Name.
		ds.writeString(i->mName);

		// Part B.
		CharacterSelectionPartB pB;
		pB.mBeardStyle = i->mBeardStyle;
		pB.mHairColour = i->mHairColour;
		pB.mFaceStyle = i->mFaceStyle;

		for (auto j = 0; j < 9; j++) {
			pB.mEquipment[j].mMaterial = i->mEquipment[j].mMaterial;
			pB.mEquipment[j].mColour = i->mEquipment[j].mColour;
		}

		pB.mPrimary = i->mPrimary;
		pB.mSecondary = i->mSecondary;
		pB.mUnknown0 = 0;
		pB.mDeity = i->mDeity;
		pB.mZoneID = i->mZoneID;
		pB.mInstanceID = 0;
		pB.mReturnHome = i->mCanReturnHome ? 1 : 0;
		pB.mUnknown1 = 0;
		pB.mRace = i->mRace;
		pB.mEnterTutorial = i->mCanEnterTutorial ? 1 : 0;
		pB.mClass = i->mClass;
		pB.mLeftEyeColour = i->mEyeColourLeft;
		pB.mBeardColour = i->mBeardColour;
		pB.mRightEyeColour = i->mEyeColourRight;
		pB.mDrakkinHeritage = i->mDrakkinHeritage;
		pB.mDrakkinTattoo = i->mDrakkinTattoo;
		pB.mDrakkinDetails = i->mDrakkinDetails;
		pB.mUnknown2 = 0;
		ds.write<CharacterSelectionPartB>(pB);
	}

	auto packet = new EQApplicationPacket(OP_SendCharInfo, data, size);
	return packet;
}
