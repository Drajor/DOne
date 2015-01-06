#include "ExperienceCalculator.h"
#include "ExperienceController.h"

using namespace Experience;

const u32 Calculator::getBase(const u8 pLevel, const u8 pOtherLevel) const {
	return 4;
}

void Calculator::calculate(CalculationResult& pResult, Context& pContext) const {
	if (pContext.mSoloKill) calculateSolo(pResult, pContext);
	if (pContext.mGroupKill) calculateGroup(pResult, pContext);
	if (pContext.mRaidKill) calculateRaid(pResult, pContext);
}

void Calculator::calculateSolo(CalculationResult& pResult, Context& pContext) const {
	const bool calculateAAExp = pContext.mController->isAAOn() && pContext.mController->canGainAAExperience();

	u32 experience = getBase(pContext.mController->getLevel(), pContext.mNPCLevel);

	float normalModifier = pContext.mNPCMod->mNormal * pContext.mZoneMod->mNormal * mGlobalModifier.mNormal;

	// Calculate Solo modifier.
	float soloModifier = pContext.mNPCMod->mSolo * pContext.mZoneMod->mSolo * mGlobalModifier.mSolo;

	pResult.mNormal = experience;
}

void Calculator::calculateGroup(CalculationResult& pResult, Context& pContext) const {
	// Calculate Group modifier.
	float groupModifier = pContext.mNPCMod->mGroup * pContext.mZoneMod->mGroup * mGlobalModifier.mGroup;
}

void Calculator::calculateRaid(CalculationResult& pResult, Context& pContext) const {
	// Calculate Raid modifier.
	float raidModifier = pContext.mNPCMod->mRaid * pContext.mZoneMod->mRaid * mGlobalModifier.mRaid;
}
