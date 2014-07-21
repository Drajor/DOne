#pragma once

#include "../common/types.h"
#include "../common/timer.h"
#include <string>

class Zone;
class EQStreamInterface;
class ZoneClientConnection;
struct PlayerProfile_Struct;
struct ExtendedProfile_Struct;

struct Vector3 {
	Vector3() : x(0.0f), y(0.0f), z(0.0f) {};
	Vector3(float pX, float pY, float pZ) : x(pX), y(pY), z(pZ) {};
	float x;
	float y;
	float z;
};
struct Vector4 {
	Vector4() : x(0.0f), y(0.0f), z(0.0f), h(0.0f) {};
	Vector4(float pX, float pY, float pZ, float pH) : x(pX), y(pY), z(pZ), h(pH) {};
	float x;
	float y;
	float z;
	float h;
};

// [Client Limitation][UF] These values are set by the client.
enum MessageColour {
	MC_White = 0,
	MC_Grey = 1,
	MC_DarkGreen = 2,
	MC_LightBlue = 4,
	MC_Pink = 5,
	MC_Red = 13,
	MC_LightGreen = 14,
	MC_Yellow = 15,
	MC_DarkBlue = 16,
	MC_Aqua = 18
};

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
	void setLoggedOut(bool pLoggedOut) { mLoggedOut = pLoggedOut; }
	bool getLoggedOut() { return mLoggedOut; }
	void setAFK(bool pAFK);
	void setShowHelm(bool pShowHelm);

	void message(uint32 pType, std::string pMessage);
	


	// Position and Heading
	Vector3 getPosition3() { return Vector3(mX, mY, mZ); };
	Vector4 getPosition4() { return Vector4(mX, mY, mZ, mHeading); };
	float getX() { return mX; }
	float getY() { return mY; }
	float getZ() { return mZ; }

	void setPosition(float pX, float pY, float pZ, float pHeading);
	void setPosition(Vector3& pPosition);
	void setPosition(Vector4& pPosition);

	float getHeading() { return mHeading; }
	void setHeading(float pHeading);
	
	// Target Group Buff
	void setTGB(bool pTGB) { mTGB = pTGB; }
	bool getTGB() { return mTGB; }

private:

	float mX;
	float mY;
	float mZ;
	float mHeading;
	void _updateProfilePosition();
	


	const uint32 mCharacterID;
	uint16 mSpawnID;
	std::string mName;
	bool mStanding;
	bool mAFK;
	bool mLoggedOut; // Flag indicating whether this character logged out via /camp
	bool mTGB;
	uint32 mStatus;
	Timer mCampTimer; // 30 seconds.

	Zone* mZone;
	ZoneClientConnection* mConnection;
	PlayerProfile_Struct* mProfile;
	ExtendedProfile_Struct* mExtendedProfile;
};