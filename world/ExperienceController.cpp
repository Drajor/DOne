#include "ExperienceController.h"
#include "Data.h"
#include "Utility.h"

using namespace Experience;

std::function<u32(u8)>* Controller::mRequiredExperienceFunction = nullptr;
std::function<u32(u32)>* Controller::mRequiredAAExperienceFunction = nullptr;
std::function<u32(u32)>* Controller::mRequiredGroupExperienceFunction = nullptr;
std::function<u32(u32)>* Controller::mRequiredRaidExperienceFunction = nullptr;

static std::function<u32(u8)> defaultExperienceFunction = [](u8 pLevel) { return 5; };
static std::function<u32(u32)> defaultAAExperienceFunction = [](u32 pPoints) { return 100; };
static std::function<u32(u32)> defaultGroupExperienceFunction = [](u32 pPoints) { return 1000; };
static std::function<u32(u32)> defaultRaidExperienceFunction = [](u32 pPoints) { return 2000; };

void Controller::_initialise() {
	setRequiredExperienceFunction(&defaultExperienceFunction);
	setRequiredAAExperienceFunction(&defaultAAExperienceFunction);
	setRequiredGroupExperienceFunction(&defaultGroupExperienceFunction);
	setRequiredRaidExperienceFunction(&defaultRaidExperienceFunction);
}

const u32 Controller::getExperienceForLevel(const u8 pLevel) {
	EXPECTED_VAR(mRequiredExperienceFunction, std::numeric_limits<u32>::max());
	return (*mRequiredExperienceFunction)(pLevel);
}

const u32 Controller::getAAExperienceForPoint(const u32 pTotalPoints) {
	EXPECTED_VAR(mRequiredAAExperienceFunction, std::numeric_limits<u32>::max());
	return (*mRequiredAAExperienceFunction)(pTotalPoints);
}

const u32 Controller::getGroupExperienceForPoint(const u32 pTotalPoints) {
	EXPECTED_VAR(mRequiredGroupExperienceFunction, std::numeric_limits<u32>::max());
	return (*mRequiredGroupExperienceFunction)(pTotalPoints);
}

const u32 Controller::getRaidExperienceForPoint(const u32 pTotalPoints) {
	EXPECTED_VAR(mRequiredRaidExperienceFunction, std::numeric_limits<u32>::max());
	return (*mRequiredRaidExperienceFunction)(pTotalPoints);
}

const bool Controller::initalise(Data::Experience& pData) {
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

void Controller::add(GainResult& pResult, const u32 pExperience, const u32 pAAExperience, const u32 pGroupExperience, const u32 pRaidExperience) {
	// Add normal experience.
	if (pExperience) {
		const auto preLevel = getLevel();
		addExperience(pExperience);
		pResult.mLevels = getLevel() - preLevel;
	}

	// Add Alternate Advancement experience.
	if (pAAExperience) {
		const auto prePoints = getUnspentAAPoints();
		addAAExperience(pAAExperience);
		pResult.mAAPoints = getUnspentAAPoints() - prePoints;
	}

	// Add Group Leadership experience.
	if (pGroupExperience) {
		const auto prePoints = getGroupPoints();
		addGroupExperience(pGroupExperience);
		pResult.mGroupPoints = getGroupPoints() - prePoints;
	}

	// Add Raid Leadership experience.
	if (pRaidExperience) {
		const auto prePoints = getRaidPoints();
		addRaidExperience(pRaidExperience);
		pResult.mRaidPoints = getRaidPoints() - prePoints;
	}
}

const bool Controller::canGainExperience() const {
	if (getLevel() < getMaximumLevel()) return true;
	return getExperience() < getExperienceCap();
}

const bool Controller::canGainAAExperience() const {
	if (getUnspentAAPoints() < getMaximumUnspentAAPoints()) return true;
	return getAAExperience() < getAAExperienceForNextPoint() - 1;
}

const bool Controller::canGainGroupExperience() const {
	if (getGroupPoints() < getMaxGroupPoints()) return true;
	return getGroupExperience() < getGroupExperienceForNextPoint() - 1;
}

const bool Controller::canGainRaidExperience() const {
	if (getRaidPoints() < getMaxRaidPoints()) return true;
	return getRaidExperience() < getRaidExperienceForNextPoint() - 1;
}

void Controller::setLevel(const u8 pLevel) {
	mLevel = Utility::clamp<u8>(pLevel, 1, getMaximumLevel());
	mExperience = 0;
}

void Controller::addExperience(const u32 pExperience) {
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

const u8 Controller::removeExperience(const u32 pExperience) {
	return 0;
}

const u32 Controller::getExperienceRatio() const {
	return static_cast<u32>(330.0f * (getExperience() / static_cast<float>(getExperienceForNextLevel())));
}
const u32 Controller::getAAExperienceRatio() const {
	return static_cast<u32>(330.0f * (getAAExperience() / static_cast<float>(getAAExperienceForNextPoint())));
}

const double Controller::getGroupRatio() const {
	return static_cast<double>(1000.0f * (getGroupExperience() / static_cast<double>(getGroupExperienceForNextPoint())));
}

const double Controller::getRaidRatio() const {
	return static_cast<double>(2000.0f * (getRaidExperience() / static_cast<double>(getRaidExperienceForNextPoint())));
}

void Controller::addAAExperience(const u32 pAAExperience) {
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

void Controller::setExperienceToAA(const u32 pToAA) {
	mExperienceToAA = Utility::clamp<u32>(pToAA, 0, 100);
}

void Controller::setUnspentAAPoints(const u32 pPoints) {
	mUnspentAAPoints = Utility::clamp<u8>(pPoints, 0, getMaximumUnspentAAPoints());
	mAAExperience = 0;
}

void Controller::addGroupExperience(const u32 pExperience) {
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

void Controller::addRaidExperience(const u32 pExperience) {
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

const u32 Controller::getMaxRaidPoints() const {
	// [Client Limitation]
	// NOTE: This limitation is cosmetic. Underfoot can go past this number.
	if (mLevel < 45) return 6;
	if (mLevel < 55) return 8;
	return 10;
}

const u32 Controller::getMaxGroupPoints() const {
	// [Client Limitation]
	// NOTE: This limitation is cosmetic. Underfoot can go past this number.
	if (mLevel < 35) return 4;
	if (mLevel < 51) return 6;
	return 8;
}
