#pragma once

#include "../common/types.h"
#include "../common/timer.h"
#include <string>

class Zone;
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

	void setZone(Zone* pZone) { mZone = pZone; }
	void setSpawnID(uint16 pSpawnID) { mSpawnID = pSpawnID; }

	Zone* getZone() { return mZone; }
	void setConnection(ZoneClientConnection* pConnection) { mConnection = pConnection; }
	ZoneClientConnection* getConnection() { return mConnection; }
	
	void update();

	void setStanding(bool pStanding);

	std::string getName() { return mName; }
	uint32 getID() { return mCharacterID; };
	uint16 getSpawnID() { return mSpawnID; }
	uint32 getStatus() { return mStatus; }
	PlayerProfile_Struct* getProfile() { return mProfile; }
	ExtendedProfile_Struct* getExtendedProfile() { return mExtendedProfile; }
	void startCamp();
	void setAFK(bool pAFK);
	void setShowHelm(bool pShowHelm);

	void message(uint32 pType, std::string pMessage);
	void setPosition(float pX, float pY, float pZ);

private:
	
	const uint32 mCharacterID;
	uint16 mSpawnID;
	std::string mName;
	bool mStanding;
	bool mAFK;
	uint32 mStatus;
	Timer mCampTimer; // 30 seconds.

	Zone* mZone;
	ZoneClientConnection* mConnection;
	PlayerProfile_Struct* mProfile;
	ExtendedProfile_Struct* mExtendedProfile;
};