#include "Profile.h"
#include <windows.h>
#include "LogSystem.h"

Profile::Profile(const String& pName, ILog* pLog) : mName(pName), mLog(pLog) {
	LARGE_INTEGER li;
	QueryPerformanceFrequency(&li);
	mFrequency = double(li.QuadPart) / 1000.0;
	QueryPerformanceCounter(&li);
	mStart = li.QuadPart;
}

Profile::~Profile()
{
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	double ms = double(li.QuadPart - mStart) / mFrequency;

	if (mLog) {
		StringStream ss;
		ss << "[Profile (" << mName << ")] @ " << ms << " ms";
		mLog->info(ss.str());
	}
}

