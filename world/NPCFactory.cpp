#include "NPCFactory.h"
#include "IDataStore.h"
#include "Data.h"
#include "LogSystem.h"
#include "NPC.h"
#include "ShopDataStore.h"
#include "Item.h"
#include "ItemFactory.h"

#include "HateControllerFactory.h"
#include "HateController.h"

NPCFactory::~NPCFactory() {
	mDataStore = nullptr;
	mItemFactory = nullptr;
	mShopDataStore = nullptr;

	if (mLog) {
		delete mLog;
		mLog = nullptr;
	}

	for (auto i : mNPCAppearances)
		delete i;
	mNPCAppearances.clear();

	for (auto i : mNPCTypes)
		delete i;
	mNPCTypes.clear();
}


const bool NPCFactory::initialise(IDataStore* pDataStore, ILogFactory* pLogFactory, ItemFactory* pItemFactory, ShopDataStore* pShopDataStore) {
	if (mInitialised) return false;
	if (!pDataStore) return false;
	if (!pLogFactory) return false;
	if (!pItemFactory) return false;
	if (!pShopDataStore) return false;

	mLog = pLogFactory->make();
	mLog->setContext("[NPCFactory]");
	mLog->status("Initialising.");

	mDataStore = pDataStore;
	mItemFactory = pItemFactory;
	mShopDataStore = pShopDataStore;

	// Load appearance data.
	if (!mDataStore->loadNPCAppearanceData(mNPCAppearances)) {
		mLog->error("Failed to load appearance data.");
		return false;
	}
	mLog->info("Loaded data for " + toString(mNPCAppearances.size()) + " Appearances.");

	// Calculate appearance data.
	if (!calculateAppearanceData()) {
		mLog->error("Failed to calculate appearance data.");
		return false;
	}

	// Load type data.
	if (!mDataStore->loadNPCTypeData(mNPCTypes)) {
		mLog->error("Failed to load type data.");
		return false;
	}
	mLog->info("Loaded data for " + toString(mNPCTypes.size()) + " Types.");

	EXPECTED_BOOLX(validateNPCTypeData(), mLog);
	

	// HateControllerFactory.
	mHateControllerFactory = new HateControllerFactory();
	mHateControllerFactory->set("null", []() { return new NullHateController(); });
	mHateControllerFactory->set("proximity", []() { return new ProximityHateController(); });
	mHateControllerFactory->set("first", []() { return new FirstHateController(); });
	mHateControllerFactory->set("last", []() { return new LastHateController(); });

	mLog->status("Finished initialising.");
	mInitialised = true;
	return true;
}

const bool NPCFactory::calculateAppearanceData() {
	for (auto i : mNPCAppearances) {
		if (!_resolveAppearanceData(i))
			return false;
	}

	return true;
}

const bool NPCFactory::_resolveAppearanceData(Data::NPCAppearance* pAppearance) {
	if (!pAppearance) return false;

	if (pAppearance->mResolved) return true;
	if (pAppearance->mParentID == 0) return true;

	u32 parentID = pAppearance->mParentID;
	auto parent = findAppearance(parentID);
	if (!parent) return false;

	// Parent needs to be resolved.
	if (!parent->mResolved) {
		if (!_resolveAppearanceData(parent))
			return false;
	}

	// Copy data from parent.
	if (pAppearance->mOverrides[Data::NPCAppearance::Attributes::RaceID] == 0)
		pAppearance->mRaceID = parent->mRaceID;

	if (pAppearance->mOverrides[Data::NPCAppearance::Attributes::Gender] == 0)
		pAppearance->mGender = parent->mGender;

	if (pAppearance->mOverrides[Data::NPCAppearance::Attributes::Texture] == 0)
		pAppearance->mTexture = parent->mTexture;

	if (pAppearance->mOverrides[Data::NPCAppearance::Attributes::BodyType] == 0)
		pAppearance->mBodyType = parent->mBodyType;

	if (pAppearance->mOverrides[Data::NPCAppearance::Attributes::Size] == 0)
		pAppearance->mSize = parent->mSize;

	if (pAppearance->mOverrides[Data::NPCAppearance::Attributes::FaceStyle] == 0)
		pAppearance->mFaceStyle = parent->mFaceStyle;

	if (pAppearance->mOverrides[Data::NPCAppearance::Attributes::HairStyle] == 0)
		pAppearance->mHairStyle = parent->mHairStyle;

	if (pAppearance->mOverrides[Data::NPCAppearance::Attributes::BeardStyle] == 0)
		pAppearance->mBeardStyle = parent->mBeardStyle;

	if (pAppearance->mOverrides[Data::NPCAppearance::Attributes::BeardColour] == 0)
		pAppearance->mBeardColour = parent->mBeardColour;

	if (pAppearance->mOverrides[Data::NPCAppearance::Attributes::EyeColourLeft] == 0)
		pAppearance->mEyeColourLeft = parent->mEyeColourLeft;

	if (pAppearance->mOverrides[Data::NPCAppearance::Attributes::EyeColourRight] == 0)
		pAppearance->mEyeColourRight = parent->mEyeColourRight;

	if (pAppearance->mOverrides[Data::NPCAppearance::Attributes::DrakkinHeritage] == 0)
		pAppearance->mDrakkinHeritage = parent->mDrakkinHeritage;

	if (pAppearance->mOverrides[Data::NPCAppearance::Attributes::DrakkinTattoo] == 0)
		pAppearance->mDrakkinTattoo = parent->mDrakkinTattoo;

	if (pAppearance->mOverrides[Data::NPCAppearance::Attributes::DrakkinDetails] == 0)
		pAppearance->mDrakkinDetails = parent->mDrakkinDetails;

	if (pAppearance->mOverrides[Data::NPCAppearance::Attributes::HelmTexture] == 0)
		pAppearance->mHelmTexture = parent->mHelmTexture;

	if (pAppearance->mOverrides[Data::NPCAppearance::Attributes::PrimaryMaterial] == 0)
		pAppearance->mPrimaryMaterial = parent->mPrimaryMaterial;

	if (pAppearance->mOverrides[Data::NPCAppearance::Attributes::SecondaryMaterial] == 0)
		pAppearance->mSecondaryMaterial = parent->mSecondaryMaterial;

	pAppearance->mResolved = true;
	return true;
}

const bool NPCFactory::validateNPCTypeData() {
	for (auto i : mNPCTypes) {
		if (!findAppearance(i->mAppearanceID)) return false;
	}

	return true;
}

NPC* NPCFactory::create(const u32 pTypeID) {
	auto type = findType(pTypeID);
	if (!type) return nullptr;
	auto appearance = findAppearance(type->mAppearanceID);
	if (!appearance) return nullptr;

	
	auto hateController = mHateControllerFactory->make("proximity");

	NPC* npc = new NPC(hateController);
	npc->setName(type->mName);
	npc->setLastName(type->mLastName);
	npc->setClass(type->mClass);
	
	npc->setRace(appearance->mRaceID);
	npc->setGender(appearance->mGender);
	npc->setTexture(appearance->mTexture);
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

	for (int i = 0; i < 7; i++)
		npc->setColour(i, appearance->mColours[i]);

	//npc->helm Helm Texture
	npc->setMaterial(MaterialSlot::Primary, appearance->mPrimaryMaterial);
	npc->setMaterial(MaterialSlot::Secondary, appearance->mSecondaryMaterial);

	// NPC is a merchant.
	if (npc->isMerchant()) {
		initialiseMerchant(npc, type);
	}

	return npc;
}

NPC* NPCFactory::createInvisibleMan() {
	auto hateController = mHateControllerFactory->make("null");
	if (!hateController) return nullptr;

	NPC* npc = new NPC(hateController);
	npc->setRace(127);
	npc->setBodyType(66);
	npc->setTargetable(false);
	npc->setShowName(false);

	return npc;
}

const bool NPCFactory::initialiseMerchant(NPC* pNPC, Data::NPCType* pType) {
	if (!pNPC) return false;
	if (!pType) return false;

	auto shopData = mShopDataStore->getShopData(pType->mShopID);
	if (!shopData) return false;

	// Add shop Items to NPC.
	for (auto i : shopData->mItems) {
		auto item = mItemFactory->make(i.first);
		if (!item) return false;

		item->setShopQuantity(i.second);
		pNPC->addShopItem(item);
	}

	return true;
}

Data::NPCType* NPCFactory::findType(const u32 pID) const {
	for (auto i : mNPCTypes) {
		if (i->mID == pID)
			return i;
	}
	return nullptr;
}

Data::NPCAppearance* NPCFactory::findAppearance(const u32 pID) const {
	for (auto i : mNPCAppearances) {
		if (i->mID == pID)
			return i;
	}
	return nullptr;
}
