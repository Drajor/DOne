#include "Profile.h"
#include <windows.h>
#include "LogSystem.h"

Profile::Profile(String pName) :
mName(pName),
mStart(0),
mFrequency(0.0f)
{
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

	StringStream ss;
	ss << "[Profile " << mName << "] Completed in " << ms << " ms";
	Log::info(ss.str());
}

