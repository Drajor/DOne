#include "ExperienceController.h"
#include "Data.h"
#include "Utility.h"

std::function<u32(u8)>* ExperienceController::mRequiredExperienceFunction = nullptr;
std::function<u32(u32)>* ExperienceController::mRequiredAAExperienceFunction = nullptr;
std::function<u32(u32)>* ExperienceController::mRequiredGroupExperienceFunction = nullptr;
std::function<u32(u32)>* ExperienceController::mRequiredRaidExperienceFunction = nullptr;

static std::function<u32(u8)> defaultExperienceFunction = [](u8 pLevel) { return 5; };
static std::function<u32(u32)> defaultAAExperienceFunction = [](u32 pPoints) { return 100; };
static std::function<u32(u32)> defaultGroupExperienceFunction = [](u32 pPoints) { return 1000; };
static std::function<u32(u32)> defaultRaidExperienceFunction = [](u32 pPoints) { return 2000; };

void ExperienceController::_initialise() {
	setRequiredExperienceFunction(&defaultExperienceFunction);
	setRequiredAAExperienceFunction(&defaultAAExperienceFunction);
	setRequiredGroupExperienceFunction(&defaultGroupExperienceFunction);
	setRequiredRaidExperienceFunction(&defaultRaidExperienceFunction);
}

const u32 ExperienceController::getExperienceForLevel(const u8 pLevel) {
	EXPECTED_VAR(mRequiredExperienceFunction, std::numeric_limits<u32>::max());
	return (*mRequiredExperienceFunction)(pLevel);
}

const u32 ExperienceController::getAAExperienceForPoint(const u32 pTotalPoints) {
	EXPECTED_VAR(mRequiredAAExperienceFunction, std::numeric_limits<u32>::max());
	return (*mRequiredAAExperienceFunction)(pTotalPoints);
}

const u32 ExperienceController::getGroupExperienceForPoint(const u32 pTotalPoints) {
	EXPECTED_VAR(mRequiredGroupExperienceFunction, std::numeric_limits<u32>::max());
	return (*mRequiredGroupExperienceFunction)(pTotalPoints);
}

const u32 ExperienceController::getRaidExperienceForPoint(const u32 pTotalPoints) {
	EXPECTED_VAR(mRequiredRaidExperienceFunction, std::numeric_limits<u32>::max());
	return (*mRequiredRaidExperienceFunction)(pTotalPoints);
}

const bool ExperienceController::initalise(Data::Experience& pData) {
	EXPECTED_BOOL(!mInitialised);
	EXPECTED_BOOL(mRequiredExperienceFunction);
	EXPECTED_BOOL(mRequiredAAExperienceFunction);
	EXPECTED_BOOL(mRequiredGroupExperienceFunction);
	EXPECTED_BOOL(mRequiredRaidExperienceFunction);
	EXPECTED_BOOL(pData.mLevel > 0);
	EXPECTED_BOOL(pData.mLevel <= pData.mMaximumLevel);
	EXPECTED_BOOL(pData.mExperienceToAA <= 100);
	EXPECTED_BOOL(pData.mUnspentAAPoints <= pData.mMaximumUnspentAA);
	EXPECTED_BOOL(pData.mSpentAAPoints <= pData.mMaximumSpentAA);

	// Normal experience.
	mLevel = pData.mLevel;
	mMaximumLevel = pData.mMaximumLevel;
	mExperience = pData.mExperience;

	// Alternate Advanced experience.
	mExperienceToAA = pData.mExperienceToAA;
	mAAExperience = pData.mAAExperience;
	mUnspentAAPoints = pData.mUnspentAAPoints;
	mMaximumUnspentAAPoints = pData.mMaximumUnspentAA;
	mSpentAAPoints = pData.mSpentAAPoints;
	mMaximumSpentAAPoints = pData.mMaximumSpentAA;

	// Leadership experience.
	mLeadershipExperienceOn = pData.mLeadershipExperienceOn;
	
	mGroupExperience = pData.mGroupExperience;
	mGroupPoints = pData.mGroupPoints;

	mRaidExperience = pData.mRaidExperience;
	mRaidPoints = pData.mRaidPoints;

	mInitialised = true;
	return true;
}

void ExperienceController::add(const u32 pExperience, const u32 pAAExperience, const u32 pGroupExperience, const u32 pRaidExperience) {
	// Add normal experience.
	if (pExperience) addExperience(pExperience);

	// Add Alternate Advancement experience.
	if (pAAExperience) addAAExperience(pAAExperience);

	// Add Group Leadership experience.
	if (pGroupExperience) addGroupExperience(pGroupExperience);

	// Add Raid Leadership experience.
	if (pRaidExperience) addRaidExperience(pRaidExperience);
}

const bool ExperienceController::canGainExperience() const {
	if (getLevel() < getMaximumLevel()) return true;
	return getExperience() < getExperienceCap();
}

const bool ExperienceController::canGainAAExperience() const {
	if (getUnspentAAPoints() < getMaximumUnspentAAPoints()) return true;
	return getAAExperience() < getAAExperienceForNextPoint() - 1;
}

const bool ExperienceController::canGainGroupExperience() const {
	if (getGroupPoints() < getMaxGroupPoints()) return true;
	return getGroupExperience() < getGroupExperienceForNextPoint() - 1;
}

const bool ExperienceController::canGainRaidExperience() const {
	if (getRaidPoints() < getMaxRaidPoints()) return true;
	return getRaidExperience() < getRaidExperienceForNextPoint() - 1;
}

void ExperienceController::setLevel(const u8 pLevel) {
	mLevel = Utility::clamp<u8>(pLevel, 1, getMaximumLevel());
	mExperience = 0;
}

void ExperienceController::addExperience(const u32 pExperience) {
	EXPECTED(canGainExperience());
	
	// Add experience and level(s).
	mExperience += pExperience;
	while (canGainExperience() && mExperience >= getExperienceForNextLevel()) {
		mExperience -= getExperienceForNextLevel();
		mLevel++;
	}
	
	// Special case: Experience cap reached.
	if (canGainExperience() == false) {
		// Set experience to cap.
		mExperience = getExperienceCap();
		return;
	}
}

const u8 ExperienceController::removeExperience(const u32 pExperience) {
	return 0;
}

const u32 ExperienceController::getExperienceRatio() const {
	return static_cast<u32>(330.0f * (getExperience() / static_cast<float>(getExperienceForNextLevel())));
}
const u32 ExperienceController::getAAExperienceRatio() const {
	return static_cast<u32>(330.0f * (getAAExperience() / static_cast<float>(getAAExperienceForNextPoint())));
}

const double ExperienceController::getGroupRatio() const {
	return static_cast<double>(1000.0f * (getGroupExperience() / static_cast<double>(getGroupExperienceForNextPoint())));
}

const double ExperienceController::getRaidRatio() const {
	return static_cast<double>(2000.0f * (getRaidExperience() / static_cast<double>(getRaidExperienceForNextPoint())));
}

void ExperienceController::addAAExperience(const u32 pAAExperience) {
	EXPECTED(canGainAAExperience());

	// Add AA experience and points(s).
	mAAExperience += pAAExperience;
	while (canGainAAExperience() && mAAExperience >= getAAExperienceForNextPoint()) {
		mAAExperience -= getAAExperienceForNextPoint();
		mUnspentAAPoints++;
	}

	// Special case: AA experience cap reached.
	if (canGainAAExperience() == false) {
		// Set AA experience to cap.
		mAAExperience = getAAExperienceForNextPoint() - 1;
		return;
	}
}

void ExperienceController::setExperienceToAA(const u32 pToAA) {
	mExperienceToAA = Utility::clamp<u32>(pToAA, 0, 100);
}

void ExperienceController::setUnspentAAPoints(const u32 pPoints) {
	mUnspentAAPoints = Utility::clamp<u8>(pPoints, 0, getMaximumUnspentAAPoints());
	mAAExperience = 0;
}

void ExperienceController::addGroupExperience(const u32 pExperience) {
	EXPECTED(canGainGroupExperience());

	// Add experience and points(s).
	mGroupExperience += pExperience;
	while (canGainGroupExperience() && mGroupExperience >= getGroupExperienceForNextPoint()) {
		mGroupExperience -= getGroupExperienceForNextPoint();
		mGroupPoints++;
	}

	// Special case: Experience cap reached.
	if (canGainGroupExperience() == false) {
		// Set experience to cap.
		mGroupExperience = getGroupExperienceForNextPoint() - 1;
		return;
	}
}

void ExperienceController::addRaidExperience(const u32 pExperience) {
	EXPECTED(canGainRaidExperience());

	// Add experience and points(s).
	mRaidExperience += pExperience;
	while (canGainRaidExperience() && mRaidExperience >= getRaidExperienceForNextPoint()) {
		mRaidExperience -= getRaidExperienceForNextPoint();
		mRaidPoints++;
	}

	// Special case: Experience cap reached.
	if (canGainRaidExperience() == false) {
		// Set experience to cap.
		mRaidExperience = getRaidExperienceForNextPoint() - 1;
		return;
	}
}

const u32 ExperienceController::getMaxRaidPoints() const {
	// [Client Limitation]
	if (mLevel < 45) return 6;
	if (mLevel < 55) return 8;
	return 10;
}

const u32 ExperienceController::getMaxGroupPoints() const {
	// [Client Limitation]
	if (mLevel < 35) return 4;
	if (mLevel < 51) return 6;
	return 8;
}
