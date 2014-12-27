#pragma once

#include "Types.h"

struct ExperienceModifier {
	float mModifier = 1.0f;
	float mAAModifer = 1.0f;
	float mLeadershipModifier = 1.0f;
};

struct ExperienceResult {
	u32 mExperience = 0;
	u32 mAAExperience = 0;
	u32 mLeadershipExperience = 0;
};

class ExperienceCalculator {
public:
	inline const float getGlobalModifier() const { return mGlobalModifier.mModifier; }
	inline void setGlobalModifer(const float pValue) { mGlobalModifier.mModifier = pValue; }
	inline const float getGlobalAAModifier() const { return mGlobalModifier.mAAModifer; }
	inline void setGlobalAAModifier(const float pValue) { mGlobalModifier.mAAModifer = pValue; }
	inline const float getGlobalLeadershipModifier() const { return mGlobalModifier.mLeadershipModifier; }
	inline void setGlobalLeadershipModifier(const float pValue) { mGlobalModifier.mLeadershipModifier = pValue; }

	void calculate(ExperienceResult& pResult, const u32 pExperienceToAA, const u8 pKillerLevel, const u8 pOtherLevel, const ExperienceModifier& pKillerModifier, const ExperienceModifier& pKilledModifier, const ExperienceModifier& pZoneModifier) const;

private:

	ExperienceModifier mGlobalModifier;
};
