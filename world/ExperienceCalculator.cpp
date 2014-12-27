#include "ExperienceCalculator.h"

void ExperienceCalculator::calculate(ExperienceResult& pResult, const u32 pExperienceToAA, const u8 pKillerLevel, const u8 pOtherLevel, const ExperienceModifier& pKillerModifier, const ExperienceModifier& pKilledModifier, const ExperienceModifier& pZoneModifier) const {
	u32 result = 10;

	pResult.mExperience = result * 100 / pExperienceToAA;
	//pResult.mAAExperience = 
}
