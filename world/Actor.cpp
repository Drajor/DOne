#include "Actor.h"

Actor::Actor() {
	// Until MS fixes initializers...
	mSpawnData.mName[0] = '\0';
	mSpawnData.mLastName[0] = '\0';
	mSpawnData.mTitle[0] = '\0';
	mSpawnData.mSuffix[0] = '\0';

	for (uint8& i : mSpawnData.mUnknown5) i = 0;

	mSpawnData.deltaHeading = 0;
	mSpawnData.x = 0;
	mSpawnData.padding0054 = 0;
	mSpawnData.y = 0;
	mSpawnData.animation = 0;
	mSpawnData.padding0058 = 0;
	mSpawnData.z = 0;
	mSpawnData.deltaY = 0;
	mSpawnData.deltaX = 0;
	mSpawnData.heading = 0;
	mSpawnData.padding0066 = 0;
	mSpawnData.deltaZ = 0;
	mSpawnData.padding0070 = 0;

	for (uint8& i : mSpawnData.mUnknown6) i = 0;

	for (uint8& i : mSpawnData.mUnknown7) i = 0;

	for (uint8& i : mSpawnData.mUnknown8) i = 0;

	for (uint8& i : mSpawnData.mUnknown9) i = 0;

	for (uint8& i : mSpawnData.mUnknown10) i = 0;

	for (uint32& i : mSpawnData.mEquipmentMaterials) i = 0;

	for (uint8& i : mSpawnData.set_to_0xFF) i = 0xFF;

	for (uint8& i : mSpawnData.mUnknown14) i = 0;

	for (uint8& i : mSpawnData.mUnknown15) i = 0;

	for (Colour& i : mSpawnData.mEquipmentColours) i.mColour = 0;

	mSpawnData.DestructibleModel[0] = '\0';
	mSpawnData.DestructibleName2[0] = '\0';
	mSpawnData.DestructibleString[0] = '\0';

	//mSpawnData.mUnknown5[0] = 1;
	//mSpawnData.mUnknown5[1] = 1;
	//mSpawnData.mUnknown5[2] = 1;
}
