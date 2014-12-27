#pragma once

#include "Types.h"

struct ExperienceModifier {
	float mNormal = 1.0f;
	float mAA = 1.0f;
	float mGroup = 1.0f;
	float mGroupLeadership = 1.0f;
	float mRaid = 1.0f;
	float mRaidLeadership = 1.0f;
};

struct ExperienceResult {
	u32 mNormal = 0;
	u32 mAA = 0;
	u32 mGroupLeadership = 0;
	u32 mRaidLeadership = 0;
};

class ExperienceCalculator {
public:
	inline const float getGlobalModifier() const { return mGlobalModifier.mNormal; }
	inline void setGlobalModifer(const float pValue) { mGlobalModifier.mNormal = pValue; }
	inline const float getGlobalAAModifier() const { return mGlobalModifier.mAA; }
	inline void setGlobalAAModifier(const float pValue) { mGlobalModifier.mAA = pValue; }
	//inline const float getGlobalLeadershipModifier() const { return mGlobalModifier.mLeadershipModifier; }
	//inline void setGlobalLeadershipModifier(const float pValue) { mGlobalModifier.mLeadershipModifier = pValue; }

	void calculate(ExperienceResult& pResult, const u32 pExperienceToAA, const u8 pKillerLevel, const u8 pOtherLevel, const ExperienceModifier& pKillerModifier, const ExperienceModifier& pKilledModifier, const ExperienceModifier& pZoneModifier) const;

private:

	ExperienceModifier mGlobalModifier;
};
