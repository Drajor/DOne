#pragma once

#include "Types.h"
#include <functional>

namespace Data {
	struct Experience;
}

namespace Experience {
	struct GainResult {
		void reset() {
			mLevels = 0;
			mAAPoints = 0;
			mGroupPoints = 0;
			mRaidPoints = 0;
		}
		u8 mLevels = 0;
		u32 mAAPoints = 0;
		u32 mGroupPoints = 0;
		u32 mRaidPoints = 0;
	};

	class Controller {
	public:
		static void _initialise();

		// Initialises the ExperienceController.
		const bool initalise(Data::Experience& pData);

		void add(GainResult& pResult, const u32 pExperience, const u32 pAAExperience, const u32 pGroupExperience, const u32 pRaidExperience);

		/////////////////////////////////////////////////////////////////////
		// Normal Experience.
		/////////////////////////////////////////////////////////////////////

		// Returns whether experience gain is possible.
		const bool canGainExperience() const;

		// Returns the current level.
		inline const u8 getLevel() const { return mLevel; }

		// Sets the current level.
		void setLevel(const u8 pLevel);

		// Returns the maximum level.
		inline const u8 getMaximumLevel() const { return mMaximumLevel; }

		// Returns the current experience.
		inline const u32 getExperience() const { return mExperience; }

		// Adds experience.
		void addExperience(const u32 pExperience);

		// Removes experience and returns the number of level lost.
		inline const u8 removeExperience(const u32 pExperience);

		// 0 to 330
		const u32 getExperienceRatio() const;

		// 0 to 330
		const u32 getAAExperienceRatio() const;

		// Returns the percentage of experience that will go to AA.
		inline const u32 getExperienceToAA() const { return mExperienceToAA; }

		// Sets the percentage of experience that will go to AA.
		void setExperienceToAA(const u32 pToAA);

		// Returns whether AA experience is on.
		inline const bool isAAOn() const { return getExperienceToAA() > 0; }

		/////////////////////////////////////////////////////////////////////
		// Alternate Advancement Experience.
		/////////////////////////////////////////////////////////////////////

		// Returns whether AA experience gain is possible.
		const bool canGainAAExperience() const;

		// Returns the unspent AA points.
		inline const u32 getUnspentAAPoints() const { return mUnspentAAPoints; }

		// Sets the current unspent AA points.
		void setUnspentAAPoints(const u32 pPoints);

		// Returns the maximum number of unspent AA points.
		inline const u32 getMaximumUnspentAAPoints() const { return mMaximumUnspentAAPoints; }

		// Returns the spent AA points.
		inline const u32 getSpentAAPoints() const { return mSpentAAPoints; }

		// Returns the maximum number of spent AA points.
		inline const u32 getMaximumSpentAAPoints() const { return mMaximumSpentAAPoints; }

		// Adds unspent AA points.
		inline void addUnspentAAPoints(const u32 pAAPoints) { mUnspentAAPoints += pAAPoints; }

		// Adds spent AA points.
		inline void addSpentAAPoints(const u32 pPoints) { mSpentAAPoints += pPoints; }

		// Returns the total number of AA points.
		inline const u32 getTotalAAPoints() const { return getSpentAAPoints() + getUnspentAAPoints(); }

		// Returns the current AA experience.
		inline const u32 getAAExperience() const { return mAAExperience; }

		// Adds AA experience.
		void addAAExperience(const u32 pAAExperience);

		// TODO: Remove AA?

		// Returns the amount of experience required for a specific Level.
		static const u32 getExperienceForLevel(const u8 pLevel);
		inline const u32 getExperienceForNextLevel() const { return getExperienceForLevel(getLevel() + 1); };
		inline const u32 getExperienceCap() const { return getExperienceForLevel(getMaximumLevel() + 1) - 1; }

		// Returns the amount of AA experience required for a specific AA point.
		static const u32 getAAExperienceForPoint(const u32 pTotalPoints);
		inline const u32 getAAExperienceForNextPoint() const { return getAAExperienceForPoint(getTotalAAPoints() + 1); }
		//inline const u32 getAAExperienceCap() const { return get }

		// Sets the function which determine how much experience is required for a specific level.
		inline static void setRequiredExperienceFunction(std::function<u32(u8)>* pFunction) { mRequiredExperienceFunction = pFunction; }

		// Sets the function which calculates how much AA experience is required for a specific point.
		inline static void setRequiredAAExperienceFunction(std::function<u32(u32)>* pFunction) { mRequiredAAExperienceFunction = pFunction; }

		/////////////////////////////////////////////////////////////////////
		// Leadership Experience.
		/////////////////////////////////////////////////////////////////////

		// Returns whether leadership experience is on.
		inline const bool isLeadershipOn() const { return mLeadershipExperienceOn; }

		// Sets whether leadership experience is on or off.
		void setLeadershipExperience(const bool pValue) { mLeadershipExperienceOn = pValue; }

		// 0.0f to 1000.0f
		const double getGroupRatio() const;

		// 0.0f to 2000.0f
		const double getRaidRatio() const;

		/////////////////////////////////////////////////////////////////////
		// Group Leadership Experience.
		/////////////////////////////////////////////////////////////////////

		// Returns whether gaining group leadership experience is possible.
		const bool canGainGroupExperience() const;

		// Sets the function which calculates how much Group Leadership experience is required for a specific point.
		inline static void setRequiredGroupExperienceFunction(std::function<u32(u32)>* pFunction) { mRequiredGroupExperienceFunction = pFunction; }

		// Adds group leadership experience.
		void addGroupExperience(const u32 pExperience);

		// Returns the current amount of group leadership experience.
		inline const u32 getGroupExperience() const { return mGroupExperience; }

		// Returns the current number of unspent group leadership points.
		inline const u32 getGroupPoints() const { return mGroupPoints; }

		// Returns the current number of spent group leadership points.
		inline const u32 getSpentGroupPoints() const { return mSpentGroupPoints; }

		// Returns the total number of group leadership points accumulated.
		inline const u32 getTotalGroupPoints() const { return getGroupPoints() + getSpentGroupPoints(); }

		// Returns the maximum number of unspent group points that can be accumulated.
		const u32 getMaxGroupPoints() const;

		// Returns the amount of group leadership experience required for a specific point.
		static const u32 getGroupExperienceForPoint(const u32 pTotalPoints);

		// Returns the amount of group leadership experience required for the next point.
		inline const u32 getGroupExperienceForNextPoint() const { return getGroupExperienceForPoint(getTotalGroupPoints() + 1); }

		/////////////////////////////////////////////////////////////////////
		// Raid Leadership Experience.
		/////////////////////////////////////////////////////////////////////

		// Returns whether gaining raid leadership experience is possible.
		const bool canGainRaidExperience() const;

		// Sets the function which calculates how much Raid Leadership experience is required for a specific point.
		inline static void setRequiredRaidExperienceFunction(std::function<u32(u32)>* pFunction) { mRequiredRaidExperienceFunction = pFunction; }

		// Adds raid leadership experience.
		void addRaidExperience(const u32 pExperience);

		// Returns the current amount of raid leadership experience.
		inline const u32 getRaidExperience() const { return mRaidExperience; }

		// Returns the current number unspent of raid leadership points.
		inline const u32 getRaidPoints() const { return mRaidPoints; }

		// Returns the total number of spent raid leadership points.
		inline const u32 getSpentRaidPoints() const { return mSpentRaidPoints; }

		// Returns the total number of raid leadership points accumulated.
		inline const u32 getTotalRaidPoints() const { return getRaidPoints() + getSpentRaidPoints(); }

		// Returns the maximum number of unspent raid points that can be accumulated.
		const u32 getMaxRaidPoints() const;

		// Returns the amount of raid leadership experience required for a specific point.
		static const u32 getRaidExperienceForPoint(const u32 pTotalPoints);

		// Returns the amount of raid leadership experience required for the next point.
		inline const u32 getRaidExperienceForNextPoint() const { return getRaidExperienceForPoint(getTotalRaidPoints() + 1); }

	private:
		bool mInitialised = false;
		static std::function<u32(u8)>* mRequiredExperienceFunction;
		static std::function<u32(u32)>* mRequiredAAExperienceFunction;
		static std::function<u32(u32)>* mRequiredGroupExperienceFunction;
		static std::function<u32(u32)>* mRequiredRaidExperienceFunction;

		// Normal Experience.
		u8 mLevel = 1;
		u8 mMaximumLevel = 1;
		u32 mExperience = 0;

		// Alternate Advancement Experience.
		u32 mExperienceToAA = 0; // 0 - 100
		u32 mSpentAAPoints = 0;
		u32 mUnspentAAPoints = 0;
		u32 mAAExperience = 0;
		u32 mMaximumUnspentAAPoints = 0;
		u32 mMaximumSpentAAPoints = 0;

		// Leadership Experience.
		bool mLeadershipExperienceOn = false;

		u32 mGroupExperience = 0;
		u32 mGroupPoints = 0;
		u32 mSpentGroupPoints = 0;

		u32 mRaidExperience = 0;
		u32 mRaidPoints = 0;
		u32 mSpentRaidPoints = 0;
	};
}