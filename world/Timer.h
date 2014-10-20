#pragma once

#include "Constants.h"

#include <iostream>

class TTimer {
public:
	// Sets the current time for all QuickTTimer objects.
	static inline void setCurrentTime(const int64 pCurrentTime) { mCurrentTime = pCurrentTime; }
public:
	TTimer() {};
	TTimer(const uint64 pStep) : mStep(pStep) { start(); }
	// Calculates the next value based on current time and step value.
	inline void start() { mNext = mCurrentTime + mStep; }
	// Checks whether the current time is greater than or equal to the next value.
	const bool check(const bool pRestart = true) {
		if (mCurrentTime >= mNext){
			if (pRestart)
				start();
			return true;
		}
		return false;
	};
	// Sets the step value.
	inline void setStep(const uint64 pStep) { mStep = pStep; }

	// Returns the number of milliseconds remaining.
	inline const uint64 remaining() { return mNext - mCurrentTime; }
	// Returns the number of complete seconds remaining.
	inline const uint64 secondsRemaining() { return static_cast<uint64>(std::floor(remaining() / 1000)); }
	// Returns the number of complete minutes remaining.
	inline const uint64 minutesRemaining() { return static_cast<uint64>(std::floor(remaining() / 1000 / 60)); }
	// Returns the number of complete hours remaining.
	inline const uint64 hoursRemaining() { return static_cast<uint64>(std::floor(remaining() / 1000 / 60 / 60)); }
	// Returns the number of complete days remaining.
	inline const uint64 daysRemaining() { return static_cast<uint64>(std::floor(remaining() / 1000 / 60 / 60 / 24)); }
private:
	uint64 mNext = 0;
	uint64 mStep = 0;
	static uint64 mCurrentTime;
};