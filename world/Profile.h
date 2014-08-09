#pragma once

#include "Constants.h"

class Profile {
public:
	Profile(String pName);
	~Profile();
private:
	String mName;
	__int64 mStart;
	double mFrequency;
};