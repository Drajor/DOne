#pragma once

#include "../common/types.h"
#include <string>
class EQStreamInterface;
struct PlayerProfile_Struct;
struct ExtendedProfile_Struct;

class Character {
public:
	Character();
	~Character();
	bool initialise(PlayerProfile_Struct* pProfile, ExtendedProfile_Struct* pExtendedProfile);
	void update();

	std::string getName() { return mName; }
	PlayerProfile_Struct* getProfile() { return mProfile; }
	ExtendedProfile_Struct* getExtendedProfile() { return mExtendedProfile; }
private:
	uint32 mID;
	std::string mName;

	PlayerProfile_Struct* mProfile;
	ExtendedProfile_Struct* mExtendedProfile;
};