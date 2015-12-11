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
		}
		u8 mLevels = 0;
		u32 mAAPoints = 0;
	};

	class Controller {
	public:
		static void _initialise();

		inline void saved() { mSaveNeeded = false; }
		inline const bool saveNeeded() const { return mSaveNeeded; }

		const bool initialise(const u8 pLevel, const u8 pMaximumLevel, const u32 pExperience, const u32 pAAExperience, const u32 pSpentAA, const u32 pMaximumSpentAA, const u32 pUnspentAA, const u32 pMaximumUnspentAA, const u32 pExperienceToAA);

		// Initialises the ExperienceController.
		bool onLoad(Data::Experience* pData);
		bool onSave(Data::Experience* pData) const;

		void add(GainResult& pResult, const u32 pExperience, const u32 pAAExperience);

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
		inline const u32 getUnspentAA() const { return mUnspentAA; }

		// Sets the current unspent AA points.
		void setUnspentAAPoints(const u32 pPoints);

		// Returns the maximum number of unspent AA points.
		inline const u32 getMaximumUnspentAA() const { return mMaximumUnspentAA; }

		// Returns the spent AA points.
		inline const u32 getSpentAA() const { return mSpentAA; }

		// Returns the maximum number of spent AA points.
		inline const u32 getMaximumSpentAA() const { return mMaximumSpentAA; }

		// Adds unspent AA points.
		inline void addUnspentAAPoints(const u32 pAAPoints) { mUnspentAA += pAAPoints; }

		// Adds spent AA points.
		inline void addSpentAAPoints(const u32 pPoints) { mSpentAA += pPoints; }

		// Returns the total number of AA points.
		inline const u32 getTotalAAPoints() const { return getSpentAA() + getUnspentAA(); }

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

	private:
		bool mInitialised = false;
		bool mSaveNeeded = false;
		Data::Experience* mData = nullptr;

		static std::function<u32(u8)>* mRequiredExperienceFunction;
		static std::function<u32(u32)>* mRequiredAAExperienceFunction;

		// Normal Experience.
		u8 mLevel = 1;
		u8 mMaximumLevel = 1;
		u32 mExperience = 0;

		// Alternate Advancement Experience.
		u32 mExperienceToAA = 0; // 0 - 100
		u32 mSpentAA = 0;
		u32 mUnspentAA = 0;
		u32 mAAExperience = 0;
		u32 mMaximumUnspentAA = 0;
		u32 mMaximumSpentAA = 0;
	};
}