#pragma once

#include "Types.h"
#include <functional>

class ExperienceController {
public:
	static void _initialise();

	// Initialises the ExperienceController.
	const bool initalise(const u8 pLevel, const u8 pMaximumLevel, const u32 pExperience, const u32 pUnspentAAPoints, const u32 pMaximumUnspentAAPoints, const u32 pSpentAAPoints, const u32 pMaximumSpentAAPoints, const u32 pAAExperience);

	// Returns whether experience gain is possible.
	const bool canGainExperience() const;

	// Returns whether AA experience gain is possible.
	const bool canGainAAExperience() const;

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

	const u32 getExperienceRatio() const;
	const u32 getAAExperienceRatio() const;

	// Returns the unspent AA points.
	inline const u32 getUnspentAAPoints() const { return mUnspentAAPoints; }

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

	// Adds AA experience and returns the number of AA points gained.
	const u8 addAAExperience(const u32 pAAExperience);

	// TODO: Remove AA?

	// Returns the amount of experience required for a specific Level.
	static const u32 getExperienceForLevel(const u8 pLevel);
	inline const u32 getExperienceForNextLevel() const { return getExperienceForLevel(getLevel() + 1); };
	inline const u32 getExperienceCap() const { return getExperienceForLevel(getMaximumLevel()) - 1; }

	// Returns the amount of AA experience required for a specific AA point.
	static const u32 getAAExperienceForPoint(const u32 pTotalPoints);
	inline const u32 getAAExperienceForNextPoint() const { return getAAExperienceForPoint(getTotalAAPoints() + 1); }

	// Sets the function which determine how much experience is required for a specific level.
	inline static void setRequiredExperienceFunction(std::function<u32(u8)>* pFunction) { mRequiredExperienceFunction = pFunction; }

	// Sets the function which calculates how much AA experience is required for a specific point.
	inline static void setRequiredAAExperienceFunction(std::function<u32(u32)>* pFunction) { mRequiredAAExperienceFunction = pFunction; }
private:
	bool mInitialised = false;
	static std::function<u32(u8)>* mRequiredExperienceFunction;
	static std::function<u32(u32)>* mRequiredAAExperienceFunction;
	u8 mMaximumLevel = 20;
	u32 mMaximumUnspentAAPoints = 100;
	u32 mMaximumSpentAAPoints = 9000;

	u8 mLevel = 1;
	u32 mExperience = 0;

	u32 mSpentAAPoints = 0;
	u32 mUnspentAAPoints = 0;
	u32 mAAExperience = 0;
};