#include "Actor.h"
#include "Utility.h"

Actor::Actor() {
	mDecayTimer.Disable();
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
	
	if (hasTitle())
		result += strlen(mActorData.mTitle) + 1;

	if (hasSuffix())
		result += strlen(mActorData.mSuffix) + 1;

	Log::info("Actor: " + getName() + " Size: " + std::to_string(result));
	return result;
}

const bool Actor::copyData(Utility::DynamicStructure& pStructure) {
	_onCopy();

	// Write name.
	pStructure.writeString(String(mActorData.mName));

	// Chunk Zero.
	std::size_t chunk0 = (unsigned int)&(mActorData.mLastName) - (unsigned int)&(mActorData.mSpawnID);
	pStructure.writeChunk((void*)&(mActorData.mSpawnID), chunk0);

	// Write last name.
	pStructure.writeString(String(mActorData.mLastName));

	// Chunk One.
	std::size_t chunk1 = (unsigned int)&(mActorData.mTitle) - (unsigned int)&(mActorData.mAAtitle);
	pStructure.writeChunk((void*)&(mActorData.mAAtitle), chunk1);

	// Write optional title.
	if (hasTitle())
		pStructure.writeString(String(mActorData.mTitle));

	// Write optional suffix.
	if (hasSuffix())
		pStructure.writeString(String(mActorData.mSuffix));

	pStructure.write<uint32>(mActorData.__Unknown16);
	pStructure.write<uint32>(mActorData.__Unknown17);
	pStructure.write<uint8>(mActorData.mIsMercenary);

	pStructure.writeChunk((void*)&(mActorData.mUnknowns), sizeof(mActorData.mUnknowns));

	if (pStructure.check() == false) {
		Log::error("[Actor] Bad Write: Written: " + std::to_string(pStructure.getBytesWritten()) + " Size: " + std::to_string(pStructure.getSize()));
	}

	return true;
}

void Actor::_onCopy() {
	if (hasTitle())
		mActorData.mOtherFlags = mActorData.mOtherFlags | 0x04;

	if (hasSuffix())
		mActorData.mOtherFlags = mActorData.mOtherFlags | 0x08;

	if (isDestructible())
		mActorData.mOtherFlags = mActorData.mOtherFlags | 0xd1;
}