#pragma once

#include <string>

class Profile {
public:
	Profile(std::string pName);
	~Profile();
private:
	std::string mName;
	__int64 mStart;
	double mFrequency;
};