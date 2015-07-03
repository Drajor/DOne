#pragma once

#include "LogSystem.h"
#include "Poco/Stopwatch.h"

class Profile {
public:
	Profile(const String& pName, ILog* pLog) : mLog(pLog) {
		mText = "[Profile (" + pName + ")] @ ";
		mStopwatch.start();
	}
	~Profile() {
		mLog->info(mText + toString(mStopwatch.elapsed() / 1000.0f) + " ms");
	}
private:
	String mText;
	ILog* mLog = nullptr;
	Poco::Stopwatch mStopwatch;
};