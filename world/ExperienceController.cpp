#include "ExperienceController.h"
#include "Utility.h"

std::function<u32(u8)>* ExperienceController::mRequiredExperienceFunction = nullptr;
std::function<u32(u32)>* ExperienceController::mRequiredAAExperienceFunction = nullptr;

static std::function<u32(u8)> defaultExperienceFunction = [](u8 pLevel) { return pLevel * 10; };
static std::function<u32(u32)> defaultAAExperienceFunction = [](u32 pPoints) { return 100; };

void ExperienceController::_initialise() {
	setRequiredExperienceFunction(&defaultExperienceFunction);
	setRequiredAAExperienceFunction(&defaultAAExperienceFunction);
}

const u32 ExperienceController::getExperienceForLevel(const u8 pLevel) {
	EXPECTED_VAR(mRequiredExperienceFunction, std::numeric_limits<u32>::max());
	return (*mRequiredExperienceFunction)(pLevel);
}

const u32 ExperienceController::getAAExperienceForPoint(const u32 pTotalPoints) {
	EXPECTED_VAR(mRequiredAAExperienceFunction, std::numeric_limits<u32>::max());
	return (*mRequiredAAExperienceFunction)(pTotalPoints);
}

const bool ExperienceController::initalise(const u8 pLevel, const u8 pMaximumLevel, const u32 pExperience, const u32 pExperienceToAA, const u32 pUnspentAAPoints, const u32 pMaximumUnspentAAPoints, const u32 pSpentAAPoints, const u32 pMaximumSpentAAPoints, const u32 pAAExperience) {
	EXPECTED_BOOL(!mInitialised);
	EXPECTED_BOOL(mRequiredExperienceFunction);
	EXPECTED_BOOL(mRequiredAAExperienceFunction);
	EXPECTED_BOOL(pLevel > 0);
	EXPECTED_BOOL(pLevel <= pMaximumLevel);
	EXPECTED_BOOL(pExperienceToAA <= 100);
	EXPECTED_BOOL(pUnspentAAPoints <= pMaximumUnspentAAPoints);
	EXPECTED_BOOL(pSpentAAPoints <= pMaximumSpentAAPoints);

	mLevel = pLevel;
	mMaximumLevel = pMaximumLevel;
	mExperience = pExperience;
	mExperienceToAA = pExperienceToAA;

	mUnspentAAPoints = pUnspentAAPoints;
	mMaximumUnspentAAPoints = pMaximumUnspentAAPoints;
	mSpentAAPoints = pSpentAAPoints;
	mMaximumSpentAAPoints = pMaximumSpentAAPoints;
	mAAExperience = pAAExperience;

	mInitialised = true;
	return true;
}

const bool ExperienceController::canGainExperience() const {
	if (getLevel() < getMaximumLevel()) return true;
	return getExperience() < getExperienceCap();
}

const bool ExperienceController::canGainAAExperience() const {
	if (getUnspentAAPoints() < getMaximumUnspentAAPoints()) return true;
	return getAAExperience() < getAAExperienceForPoint(getTotalAAPoints() + 1) - 1;
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
