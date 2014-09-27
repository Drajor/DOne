#pragma once

#include "Constants.h"
#include <random>
#include <chrono>

namespace Random {
	static std::minstd_rand mRandom;
	static void initialise(const unsigned int pSeed = 0) {
		// Seed provided
		if (pSeed != 0) {
			mRandom.seed(pSeed);
			return;
		}

		auto seed = std::chrono::system_clock::now().time_since_epoch().count();
		mRandom.seed(seed);
	}
	
	template <typename T>
	inline const T make(const T pLow, const T pHigh) {
		return std::uniform_int_distribution<T>(pLow, pHigh)(mRandom);
	}
}