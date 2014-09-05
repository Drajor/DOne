#pragma once

#include "Constants.h"

class Profile {
public:
	Profile(String pName);
	~Profile();
private:
	String mName = "unnamed";
	__int64 mStart = 0;
	double mFrequency = 0.0f;
};