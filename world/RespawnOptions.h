#pragma once

#include "Types.h"
#include "Vector3.h"
#include <list>

struct RespawnOption {
	u32 mID = 0;
	u16 mZoneID = 0;
	u16 mInstanceID = 0;
	Vector3 mPosition;
	float mHeading = 0.0f;
	String mName;
	u8 mType = 0;
};

class RespawnOptions {
public:
	inline const bool isActive() const { return mActive; }
	inline void setActive(const bool pValue) { mActive = pValue; }

	inline void add(RespawnOption pOption) { mOptions.push_back(pOption); }

	// Returns the number of available options.
	inline const u32 getNumOptions() const { return mOptions.size(); }

	// Returns a reference to RespawnOption list.
	std::list<RespawnOption>& getOptions() { return mOptions; }
private:
	bool mActive = false;
	std::list<RespawnOption> mOptions;
};