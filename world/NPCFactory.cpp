#include "NPCFactory.h"
#include "Utility.h"
#include "DataStore.h"
#include "NPC.h"

const bool NPCFactory::initialise() {
	EXPECTED_BOOL(mInitialised == false);

	EXPECTED_BOOL(DataStore::getInstance().loadNPCAppearanceData(mNPCAppearanceData));
	EXPECTED_BOOL(calculateAppearanceData());

	EXPECTED_BOOL(DataStore::getInstance().loadNPCTypeData(mNPCTypeData));
	EXPECTED_BOOL(validateNPCTypeData());

	mInitialised = true;
	return true;
}

const bool NPCFactory::calculateAppearanceData() {
	for (auto i : mNPCAppearanceData) {
		if (!_resolveAppearanceData(i))
			return false;
	}

	return true;
}

const bool NPCFactory::_resolveAppearanceData(NPCAppearanceData* pAppearance) {
	EXPECTED_BOOL(pAppearance);

	if (pAppearance->mResolved) return true;
	if (pAppearance->mParentID == 0) return true;

	uint32 parentID = pAppearance->mParentID;
	NPCAppearanceData* parent = _findAppearance(parentID);
	EXPECTED_BOOL(parent);

	// Parent needs to be resolved.
	if (!parent->mResolved) {
		if (!_resolveAppearanceData(parent))
			return false;
	}

	// Copy data from parent.
	if (pAppearance->mOverrides[NPCAppearanceData::Attributes::RaceID] == 0)
		pAppearance->mRaceID = parent->mRaceID;

	if (pAppearance->mOverrides[NPCAppearanceData::Attributes::Gender] == 0)
		pAppearance->mGender = parent->mGender;

	if (pAppearance->mOverrides[NPCAppearanceData::Attributes::BodyType] == 0)
		pAppearance->mBodyType = parent->mBodyType;

	if (pAppearance->mOverrides[NPCAppearanceData::Attributes::Size] == 0)
		pAppearance->mSize = parent->mSize;

	if (pAppearance->mOverrides[NPCAppearanceData::Attributes::FaceStyle] == 0)
		pAppearance->mFaceStyle = parent->mFaceStyle;

	if (pAppearance->mOverrides[NPCAppearanceData::Attributes::HairStyle] == 0)
		pAppearance->mHairStyle = parent->mHairStyle;

	if (pAppearance->mOverrides[NPCAppearanceData::Attributes::BeardStyle] == 0)
		pAppearance->mBeardStyle = parent->mBeardStyle;

	if (pAppearance->mOverrides[NPCAppearanceData::Attributes::BeardColour] == 0)
		pAppearance->mBeardColour = parent->mBeardColour;

	if (pAppearance->mOverrides[NPCAppearanceData::Attributes::EyeColourLeft] == 0)
		pAppearance->mEyeColourLeft = parent->mEyeColourLeft;

	if (pAppearance->mOverrides[NPCAppearanceData::Attributes::EyeColourRight] == 0)
		pAppearance->mEyeColourRight = parent->mEyeColourRight;

	if (pAppearance->mOverrides[NPCAppearanceData::Attributes::DrakkinHeritage] == 0)
		pAppearance->mDrakkinHeritage = parent->mDrakkinHeritage;

	if (pAppearance->mOverrides[NPCAppearanceData::Attributes::DrakkinTattoo] == 0)
		pAppearance->mDrakkinTattoo = parent->mDrakkinTattoo;

	if (pAppearance->mOverrides[NPCAppearanceData::Attributes::DrakkinDetails] == 0)
		pAppearance->mDrakkinDetails = parent->mDrakkinDetails;

	if (pAppearance->mOverrides[NPCAppearanceData::Attributes::HelmTexture] == 0)
		pAppearance->mHelmTexture = parent->mHelmTexture;

	if (pAppearance->mOverrides[NPCAppearanceData::Attributes::PrimaryMaterial] == 0)
		pAppearance->mPrimaryMaterial = parent->mPrimaryMaterial;

	if (pAppearance->mOverrides[NPCAppearanceData::Attributes::SecondaryMaterial] == 0)
		pAppearance->mSecondaryMaterial = parent->mSecondaryMaterial;

	pAppearance->mResolved = true;
	return true;
}

NPCAppearanceData* NPCFactory::_findAppearance(const uint32 pID) {
	for (auto i : mNPCAppearanceData) {
		if (i->mID == pID)
			return i;
	}
	return nullptr;
}

const bool NPCFactory::validateNPCTypeData() {
	for (auto i : mNPCTypeData) {
		EXPECTED_BOOL(_findAppearance(i->mAppearanceID));
	}

	return true;
}

NPC* NPCFactory::create(const uint32 pTypeID) {
	NPCTypeData* type = _findType(pTypeID);
	EXPECTED_PTR(type);
	NPCAppearanceData* appearance = _findAppearance(type->mAppearanceID);
	EXPECTED_PTR(appearance);

	NPC* npc = new NPC();
	npc->setName(type->mName);
	npc->setLastName(type->mLastName);
	
	npc->setRace(appearance->mRaceID);
	npc->setGender(appearance->mGender);
	npc->setBodyType(appearance->mBodyType);
	npc->setSize(appearance->mSize);

	npc->setFaceStyle(appearance->mFaceStyle);
	npc->setHairStyle(appearance->mHairStyle);
	npc->setHairColour(appearance->mHairColour);
	npc->setBeardStyle(appearance->mBeardStyle);
	npc->setBeardColour(appearance->mBeardColour);
	npc->setLeftEyeColour(appearance->mEyeColourLeft);
	npc->setRightEyeColour(appearance->mEyeColourRight);
	npc->setDrakkinHeritage(appearance->mDrakkinHeritage);
	npc->setDrakkinTattoo(appearance->mDrakkinTattoo);
	npc->setDrakkinDetails(appearance->mDrakkinDetails);
	//npc->helm Helm Texture
	npc->setMaterial(MaterialSlot::Mat_Primary, appearance->mPrimaryMaterial);
	npc->setMaterial(MaterialSlot::Mat_Secondary, appearance->mSecondaryMaterial);

	return npc;
}

NPCTypeData* NPCFactory::_findType(const uint32 pID) {
	for (auto i : mNPCTypeData) {
		if (i->mID == pID)
			return i;
	}
	return nullptr;
}
