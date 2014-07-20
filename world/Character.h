#pragma once

#include "../common/types.h"
#include "../common/timer.h"
#include <string>
class EQStreamInterface;
class ZoneClientConnection;
struct PlayerProfile_Struct;
struct ExtendedProfile_Struct;

class Character {
	friend ZoneClientConnection;
public:
	Character(uint32 pCharacterID);
	~Character();
	bool initialise(PlayerProfile_Struct* pProfile, ExtendedProfile_Struct* pExtendedProfile);
	void update();

	void setStanding(bool pStanding);

	std::string getName() { return mName; }
	uint32 getID() { return mID; };
	PlayerProfile_Struct* getProfile() { return mProfile; }
	ExtendedProfile_Struct* getExtendedProfile() { return mExtendedProfile; }
	void startCamp();
	void setAFK(bool pAFK);
	void setShowHelm(bool pShowHelm);
private:
	
	const uint32 mID;
	std::string mName;
	bool mStanding;
	bool mAFK;
	Timer mCampTimer; // 30 seconds.

	PlayerProfile_Struct* mProfile;
	ExtendedProfile_Struct* mExtendedProfile;
};