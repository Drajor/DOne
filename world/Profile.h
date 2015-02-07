#pragma once

#include "Constants.h"

class ILog;

class Profile {
public:
	Profile(const String& pName, ILog* pLog);
	~Profile();
private:
	String mName = "unnamed";
	ILog* mLog = nullptr;
	__int64 mStart = 0;
	double mFrequency = 0.0f;
};