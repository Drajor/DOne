#include "ExperienceController.h"
#include "Data.h"
#include "Utility.h"

using namespace Experience;

std::function<u32(u8)>* Controller::mRequiredExperienceFunction = nullptr;
std::function<u32(u32)>* Controller::mRequiredAAExperienceFunction = nullptr;

static std::function<u32(u8)> defaultExperienceFunction = [](u8 pLevel) { return 5; };
static std::function<u32(u32)> defaultAAExperienceFunction = [](u32 pPoints) { return 100; };

void Controller::_initialise() {
	setRequiredExperienceFunction(&defaultExperienceFunction);
	setRequiredAAExperienceFunction(&defaultAAExperienceFunction);
}

const u32 Controller::getExperienceForLevel(const u8 pLevel) {
	EXPECTED_VAR(mRequiredExperienceFunction, std::numeric_limits<u32>::max());
	return (*mRequiredExperienceFunction)(pLevel);
}

const u32 Controller::getAAExperienceForPoint(const u32 pTotalPoints) {
	EXPECTED_VAR(mRequiredAAExperienceFunction, std::numeric_limits<u32>::max());
	return (*mRequiredAAExperienceFunction)(pTotalPoints);
}

const bool Experience::Controller::initialise(const u8 pLevel, const u8 pMaximumLevel, const u32 pExperience, const u32 pAAExperience, const u32 pSpentAA, const u32 pMaximumSpentAA, const u32 pUnspentAA, const u32 pMaximumUnspentAA, const u32 pExperienceToAA) {
	if (mInitialised) return false;
	if (!mRequiredExperienceFunction) return false;
	if (!mRequiredAAExperienceFunction) return false;

	// Check: Level is not 0 (invalid).
	if (pLevel == 0) return false;

	// Check: Level is not greater than the maximum.
	if (pLevel > pMaximumLevel) return false;

	// Check: Experience towards AA is not greater than 100%.
	if (pExperienceToAA > 100) return false;

	// Check: Number of unspent AA is not greater than the maximum
	if (pUnspentAA > pMaximumUnspentAA) return false;

	// Check: Number of spent AA is not greater than the maximum.
	if (pSpentAA > pMaximumSpentAA) return false;

	// Normal experience.
	mLevel = pLevel;
	mMaximumLevel = pMaximumLevel;
	mExperience = pExperience;

	// Alternate Advanced experience.
	mExperienceToAA = pExperienceToAA;
	mAAExperience = pAAExperience;
	mUnspentAA = pUnspentAA;
	mMaximumUnspentAA = pMaximumUnspentAA;
	mSpentAA = pSpentAA;
	mMaximumSpentAA = pMaximumSpentAA;

	mInitialised = true;
	return true;
}

bool Controller::onLoad(Data::Experience* pData) {
	if (mInitialised) return false;
	if (!pData) return false;
	if (!mRequiredExperienceFunction) return false;
	if (!mRequiredAAExperienceFunction) return false;
	
	mData = pData;
	if (mData->mLevel == 0) return false;
	if (mData->mLevel > mData->mMaximumLevel) return false;
	if (mData->mExperienceToAA > 100) return false;
	if (mData->mUnspentAAPoints > mData->mMaximumUnspentAA) return false;
	if (mData->mSpentAAPoints > mData->mMaximumSpentAA) return false;

	// Normal experience.
	mLevel = mData->mLevel;
	mMaximumLevel = mData->mMaximumLevel;
	mExperience = mData->mExperience;

	// Alternate Advanced experience.
	mExperienceToAA = mData->mExperienceToAA;
	mAAExperience = mData->mAAExperience;
	mUnspentAA = mData->mUnspentAAPoints;
	mMaximumUnspentAA = mData->mMaximumUnspentAA;
	mSpentAA = mData->mSpentAAPoints;
	mMaximumSpentAA = mData->mMaximumSpentAA;

	mInitialised = true;
	return true;
}

bool Experience::Controller::onSave(Data::Experience* pData) const {
	pData->mLevel = getLevel();
	pData->mMaximumLevel = getMaximumLevel();
	pData->mExperience = getExperience();
	pData->mExperienceToAA = getExperienceToAA();
	pData->mAAExperience = getAAExperience();
	pData->mUnspentAAPoints = getUnspentAA();
	pData->mMaximumUnspentAA = getMaximumUnspentAA();
	pData->mSpentAAPoints = getSpentAA();
	pData->mMaximumSpentAA = getMaximumSpentAA();
	return true;
}

void Controller::add(GainResult& pResult, const u32 pExperience, const u32 pAAExperience) {
	// Add normal experience.
	if (pExperience) {
		const auto preLevel = getLevel();
		addExperience(pExperience);
		pResult.mLevels = getLevel() - preLevel;
	}

	// Add Alternate Advancement experience.
	if (pAAExperience) {
		const auto prePoints = getUnspentAA();
		addAAExperience(pAAExperience);
		pResult.mAAPoints = getUnspentAA() - prePoints;
	}
}

const bool Controller::canGainExperience() const {
	if (getLevel() < getMaximumLevel()) return true;
	return getExperience() < getExperienceCap();
}

const bool Controller::canGainAAExperience() const {
	if (getUnspentAA() < getMaximumUnspentAA()) return true;
	return getAAExperience() < getAAExperienceForNextPoint() - 1;
}

void Controller::setLevel(const u8 pLevel) {
	mLevel = Utility::clamp<u8>(pLevel, 1, getMaximumLevel());
	mExperience = 0;
	mSaveNeeded = true;
}

void Controller::addExperience(const u32 pExperience) {
	EXPECTED(canGainExperience());
	
	// Add experience and level(s).
	mExperience += pExperience;
	while (canGainExperience() && mExperience >= getExperienceForNextLevel()) {
		mExperience -= getExperienceForNextLevel();
		mLevel++;
		mSaveNeeded = true;
	}
	
	// Special case: Experience cap reached.
	if (canGainExperience() == false) {
		// Set experience to cap.
		mExperience = getExperienceCap();
		mSaveNeeded = true;
		return;
	}
}

const u8 Controller::removeExperience(const u32 pExperience) {
	return 0;
}

const u32 Controller::getExperienceRatio() const {
	return static_cast<u32>(330.0f * (getExperience() / static_cast<float>(getExperienceForNextLevel())));
}
const u32 Controller::getAAExperienceRatio() const {
	return static_cast<u32>(330.0f * (getAAExperience() / static_cast<float>(getAAExperienceForNextPoint())));
}

void Controller::addAAExperience(const u32 pAAExperience) {
	EXPECTED(canGainAAExperience());

	// Add AA experience and points(s).
	mAAExperience += pAAExperience;
	while (canGainAAExperience() && mAAExperience >= getAAExperienceForNextPoint()) {
		mAAExperience -= getAAExperienceForNextPoint();
		mUnspentAA++;
		mSaveNeeded = true;
	}

	// Special case: AA experience cap reached.
	if (canGainAAExperience() == false) {
		// Set AA experience to cap.
		mAAExperience = getAAExperienceForNextPoint() - 1;
		mSaveNeeded = true;
		return;
	}
}

void Controller::setExperienceToAA(const u32 pToAA) {
	mExperienceToAA = Utility::clamp<u32>(pToAA, 0, 100);
	mSaveNeeded = true;
}

void Controller::setUnspentAAPoints(const u32 pPoints) {
	mUnspentAA = Utility::clamp<u8>(pPoints, 0, getMaximumUnspentAA());
	mAAExperience = 0;
	mSaveNeeded = true;
}