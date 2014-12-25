#include "ExperienceController.h"
#include "Utility.h"

std::function<u32(u8)>* ExperienceController::mRequiredExperienceFunction = nullptr;
std::function<u32(u32)>* ExperienceController::mRequiredAAExperienceFunction = nullptr;

static std::function<u32(u8)> defaultExperienceFunction = [](u8 pLevel) { return pLevel * 10; };
static std::function<u32(u32)> defaultAAExperienceFunction = [](u32 pPoints) { return 1; };

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

const bool ExperienceController::initalise(const u8 pLevel, const u8 pMaximumLevel, const u32 pExperience, const u32 pUnspentAAPoints, const u32 pMaximumUnspentAAPoints, const u32 pSpentAAPoints, const u32 pMaximumSpentAAPoints, const u32 pAAExperience) {
	EXPECTED_BOOL(!mInitialised);
	EXPECTED_BOOL(mRequiredExperienceFunction);
	EXPECTED_BOOL(mRequiredAAExperienceFunction);
	EXPECTED_BOOL(pLevel > 0);
	EXPECTED_BOOL(pLevel <= pMaximumLevel);
	EXPECTED_BOOL(pUnspentAAPoints <= pMaximumUnspentAAPoints);
	EXPECTED_BOOL(pSpentAAPoints <= pMaximumSpentAAPoints);

	mLevel = pLevel;
	mMaximumLevel = pMaximumLevel;
	mExperience = pExperience;

	mUnspentAAPoints = pUnspentAAPoints;
	mMaximumUnspentAAPoints = pMaximumUnspentAAPoints;
	mSpentAAPoints = pSpentAAPoints;
	mMaximumSpentAAPoints = pMaximumSpentAAPoints;
	mAAExperience = pAAExperience;

	mInitialised = true;
	return true;
}

const bool ExperienceController::canGainExperience() const {
	if (mLevel < mMaximumLevel) return true;
	return mExperience < getExperienceCap();
}

const bool ExperienceController::canGainAAExperience() const {
	if (mUnspentAAPoints < mMaximumUnspentAAPoints) return true;
	// TODO: This needs to be different because there are 2 possible caps.
	return mAAExperience == getAAExperienceForPoint(getTotalAAPoints() + 1) - 1;
}

void ExperienceController::setLevel(const u8 pLevel) {
	mLevel = pLevel;
	mExperience = 0;
}

void ExperienceController::addExperience(const u32 pExperience) {
	EXPECTED(canGainExperience());
	
	const bool willGainLevel = mExperience + pExperience >= getExperienceForNextLevel();

	// Regular case: Experience will not cause a level increase.
	if (!willGainLevel) {
		mExperience += pExperience;
		return;
	}

	// Special case: Experience cap reached.
	if (willGainLevel && mLevel == mMaximumLevel) {
		// Set experience to cap.
		mExperience = getExperienceCap();
		return;
	}
	
	// Add level(s)
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
	return static_cast<u32>(330.0f * (mExperience / static_cast<float>(getExperienceForNextLevel())));
}
const u32 ExperienceController::getAAExperienceRatio() const {
	return 0;
}

const u8 ExperienceController::addAAExperience(const u32 pAAExperience) {
	return 0;
}
