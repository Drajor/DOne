#pragma once

#include "../common/types.h"
#include "../common/timer.h"
#include <string>
class EQStreamInterface;
class ZoneClientConnection;
struct PlayerProfile_Struct;
struct ExtendedProfile_Struct;

class Character {
public:
	Character();
	~Character();
	bool initialise(PlayerProfile_Struct* pProfile, ExtendedProfile_Struct* pExtendedProfile);
	void update();

	void setStanding(bool pStanding);

	std::string getName() { return mName; }
	PlayerProfile_Struct* getProfile() { return mProfile; }
	ExtendedProfile_Struct* getExtendedProfile() { return mExtendedProfile; }
	void startCamp();
private:
	friend ZoneClientConnection;
	uint32 mID;
	std::string mName;
	int32 mAnimation;
	bool mStanding;
	Timer mCampTimer; // 30 seconds.

	PlayerProfile_Struct* mProfile;
	ExtendedProfile_Struct* mExtendedProfile;
};