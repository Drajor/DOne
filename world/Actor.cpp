#include "Actor.h"
#include "Utility.h"

Actor::Actor() {
	mDecayTimer.Disable();
	// Until MS fixes initializers...
	//mSpawnData.mName[0] = '\0';
	//mSpawnData.mLastName[0] = '\0';
	//mSpawnData.mTitle[0] = '\0';
	//mSpawnData.mSuffix[0] = '\0';

	//for (uint8& i : mSpawnData.mUnknown5) i = 0;

	//mSpawnData.deltaHeading = 0;
	//mSpawnData.x = 0;
	//mSpawnData.padding0054 = 0;
	//mSpawnData.y = 0;
	//mSpawnData.animation = 0;
	//mSpawnData.padding0058 = 0;
	//mSpawnData.z = 0;
	//mSpawnData.deltaY = 0;
	//mSpawnData.deltaX = 0;
	//mSpawnData.heading = 0;
	//mSpawnData.padding0066 = 0;
	//mSpawnData.deltaZ = 0;
	//mSpawnData.padding0070 = 0;

	//for (uint8& i : mSpawnData.mUnknown6) i = 0;

	//for (uint8& i : mSpawnData.mUnknown7) i = 0;

	//for (uint8& i : mSpawnData.mUnknown8) i = 0;

	//for (uint8& i : mSpawnData.mUnknown9) i = 0;

	//for (uint8& i : mSpawnData.mUnknown10) i = 0;

	//for (uint32& i : mSpawnData.mEquipmentMaterials) i = 0;

	//for (uint8& i : mSpawnData.set_to_0xFF) i = 0xFF;

	//for (uint8& i : mSpawnData.mUnknown14) i = 0;

	//for (uint8& i : mSpawnData.mUnknown15) i = 0;

	//for (Colour& i : mSpawnData.mEquipmentColours) i.mColour = 0;

	//mSpawnData.DestructibleModel[0] = '\0';
	//mSpawnData.DestructibleName2[0] = '\0';
	//mSpawnData.DestructibleString[0] = '\0';

	//mSpawnData.mUnknown5[0] = 1;
	//mSpawnData.mUnknown5[1] = 1;
	//mSpawnData.mUnknown5[2] = 1;
}

void Actor::setTarget(Actor* pActor) {
	EXPECTED(pActor);
	if (mTarget)
		clearTarget();

	mTarget = pActor;
	mTarget->addTargeter(this);		
}

const uint32 Actor::getDataSize() const {
	uint32 result = sizeof(Payload::ActorData);

	// Remove the maximum size of variable sized attributes.
	result -= sizeof(mActorData.mName);
	result -= sizeof(mActorData.mLastName);
	result -= sizeof(mActorData.mTitle);
	result -= sizeof(mActorData.mSuffix);

	// Add variable strings
	result += strlen(mActorData.mName) + 1;
	result += strlen(mActorData.mLastName) + 1;
	result += strlen(mActorData.mTitle) + 1;
	result += strlen(mActorData.mSuffix) + 1;

	Log::info("Actor: " + getName() + " Size: " + std::to_string(result));
	return result;
}

const bool Actor::copyData(Utility::DynamicStructure& pStructure) {

	pStructure.writeString(String(mActorData.mName));

	// Chunk Zero.
	std::size_t chunk0 = (unsigned int)&(mActorData.mLastName) - (unsigned int)&(mActorData.mSpawnID);
	pStructure.writeChunk((void*)&(mActorData.mSpawnID), chunk0);

	pStructure.writeString(String(mActorData.mLastName));

	// Chunk One.
	std::size_t chunk1 = (unsigned int)&(mActorData.mTitle) - (unsigned int)&(mActorData.__Unknown7);
	pStructure.writeChunk((void*)&(mActorData.__Unknown7), chunk1);

	pStructure.writeString(String(mActorData.mTitle));
	pStructure.writeString(String(mActorData.mSuffix));

	pStructure.write<uint32>(mActorData.__Unknown16);
	pStructure.write<uint32>(mActorData.__Unknown17);
	pStructure.write<uint8>(mActorData.mIsMercenary);

	pStructure.writeChunk((void*)&(mActorData.mUnknowns), sizeof(mActorData.mUnknowns));

	return true;
}