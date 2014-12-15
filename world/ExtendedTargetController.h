#pragma once

#include "Types.h"

class ExtendedTargetController {
public:
	inline const bool getAutoAddHaters() const { return mAutoAddHaters; }
	inline void setAutoAddHaters(const bool pValue) { mAutoAddHaters = pValue; }
private:
	bool mAutoAddHaters = false;
};