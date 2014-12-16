#pragma once

#include <random>

namespace Random {
	static std::mt19937 mRandom;

	static void initialise(const unsigned int pSeed = 0) {
		// Seed provided
		if (pSeed != 0) {
			mRandom.seed(pSeed);
			return;
		}

		std::random_device rd{};
		mRandom.seed(rd());
	}
	
	template <typename T>
	inline const T make(const T pLow, const T pHigh) {
		static std::uniform_int_distribution<T> distribution;
		return distribution(mRandom, { pLow, pHigh });
	}

	//template <typename T, typename D>
	//inline const T _make(const T pLow, const T pHigh, D& pDistribution) {
	//	return D<T>(pLow, pHigh);
	//}

	//template <typename T>
	//inline const T makeIntegral(const T pLow, const T pHigh) { return _make<T, std::uniform_int_distribution>(pLow, pHigh); }

	//template <typename T>
	//inline const T makeFloating(const T pLow, const T pHigh) { return _make<T, std::uniform_real_distribution>(pLow, pHigh); }

	// Returns a random value between 0 and pHigh
	template <typename T>
	inline const T make(const T pHigh) { return make<T>(0, pHigh); }

	template <typename T>
	inline const bool roll(const T pRequired) { return make<T>(99) <= pRequired; }
}