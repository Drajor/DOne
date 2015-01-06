#pragma once

#include "Types.h"
#include <memory>

namespace Experience {
	class Controller;

	struct Modifier {
		float mNormal = 1.0f; // Applied to ALL experience.

		float mAA = 1.0f; // Applied to Alternate Advanced Experience.
		float mLeadership = 1.0f; // Applied to Leadership Experience.

		float mSolo = 1.0f; // Applied when an individual Character kills an NPC.
		float mGroup = 1.0f; // Applied when a Group kills an NPC.
		float mRaid = 1.0f; // Applied when a Raid kills an NPC.
	};

	struct CalculationResult {
		void reset() {
			mNormal = 0;
			mAA = 0;
			mGroupLeadership = 0;
			mRaidLeadership = 0;
		}
		u32 mNormal = 0;
		u32 mAA = 0;
		u32 mGroupLeadership = 0;
		u32 mRaidLeadership = 0;
	};

	struct Context {
		void reset() {
			mNPCLevel = 1;
			mSoloKill = false;
			mGroupKill = false;
			mRaidKill = false;
			mGroupLeader = false;
			mRaidLeader = false;
			mController = nullptr;
			mNPCMod = nullptr;
			mZoneMod = nullptr;
		}
		u8 mNPCLevel = 1;
		bool mSoloKill = false;
		bool mGroupKill = false;
		bool mRaidKill = false;
		bool mGroupLeader = false;
		bool mRaidLeader = false;
		Controller* mController = nullptr;
		Modifier* mNPCMod = nullptr;
		Modifier* mZoneMod = nullptr;
	};

	class Calculator {
	public:
		inline const float getGlobalModifier() const { return mGlobalModifier.mNormal; }
		inline void setGlobalModifer(const float pValue) { mGlobalModifier.mNormal = pValue; }
		inline const float getGlobalAAModifier() const { return mGlobalModifier.mAA; }
		inline void setGlobalAAModifier(const float pValue) { mGlobalModifier.mAA = pValue; }
		inline const float getGlobalLeadershipModifier() const { return mGlobalModifier.mLeadership; }
		inline void setGlobalLeadershipModifier(const float pValue) { mGlobalModifier.mLeadership = pValue; }

		const u32 getBase(const u8 pLevel, const u8 pOtherLevel) const;

		void calculate(CalculationResult& pResult, Context& pContext) const;

	private:

		void calculateSolo(CalculationResult& pResult, Context& pContext) const;
		void calculateGroup(CalculationResult& pResult, Context& pContext) const;
		void calculateRaid(CalculationResult& pResult, Context& pContext) const;

		Modifier mGlobalModifier;
	};
}
