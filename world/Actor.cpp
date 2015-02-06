#include "Actor.h"
#include "Utility.h"
#include "LootController.h"

Actor::Actor() {
	mLootController = new LootController();
}

Actor::~Actor() {
	// Check: Actor has been cleaned up correctly before delete.
	if (mTarget != nullptr) { Log::error("[Actor] Target not null on destruction."); }
	if (mLootController->getLooter() != nullptr) { Log::error("[Actor] Looter not null on destruction."); }
	if (mVisibleTo.empty() == false) { Log::error("[Actor] VisibleTo not empty on destruction."); }
	if (mTargeters.empty() == false) { Log::error("[Actor] Targeters not empty on destruction."); }

	if (mHaters.empty() == false) { Log::error("[Actor] Haters not empty on destruction."); }
	
	delete mLootController;
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
	result -= sizeof(mActorData.mSurname);
	result -= sizeof(mActorData.mTitle);
	result -= sizeof(mActorData.mSuffix);

	// Add variable strings
	result += strlen(mActorData.mName) + 1;
	result += strlen(mActorData.mSurname) + 1;
	
	if (hasTitle())
		result += strlen(mActorData.mTitle) + 1;

	if (hasSuffix())
		result += strlen(mActorData.mSuffix) + 1;

	if (!sendsEquipment()) {
		result -= sizeof(mActorData.mColours);
		result -= sizeof(Payload::ActorData::Equipment) * 6;
	}

	return result;
}

const bool Actor::copyData(Utility::MemoryWriter& pWriter) {
	_onCopy();

	// Write name.
	pWriter.writeString(String(mActorData.mName));

	// Chunk Zero.
	std::size_t chunk0 = (unsigned int)&(mActorData.mSurname) - (unsigned int)&(mActorData.mSpawnID);
	pWriter.writeChunk((void*)&(mActorData.mSpawnID), chunk0);

	// Write last name.
	pWriter.writeString(String(mActorData.mSurname));

	// Check One.
	std::size_t chunk1Size = (unsigned int)&(mActorData.mColours) - (unsigned int)&(mActorData.mAAtitle);
	pWriter.writeChunk((void*)&(mActorData.mAAtitle), chunk1Size);

	if (sendsEquipment()) {
		// Write colours.
		pWriter.writeChunk((void*)&(mActorData.mColours), sizeof(mActorData.mColours));
		// Write equipment.
		pWriter.writeChunk((void*)&(mActorData.mEquipment), sizeof(mActorData.mEquipment));
	}
	else {
		pWriter.write<uint32>(0);
		pWriter.write<uint32>(0);
		pWriter.write<uint32>(0);

		pWriter.write<Payload::ActorData::Equipment>(mActorData.mEquipment[MaterialSlot::Primary]);
		pWriter.write<Payload::ActorData::Equipment>(mActorData.mEquipment[MaterialSlot::Secondary]);
	}

	// Write optional title.
	if (hasTitle())
		pWriter.writeString(String(mActorData.mTitle));

	// Write optional suffix.
	if (hasSuffix())
		pWriter.writeString(String(mActorData.mSuffix));

	pWriter.write<uint32>(mActorData.__Unknown16);
	pWriter.write<uint32>(mActorData.__Unknown17);
	pWriter.write<uint8>(mActorData.mIsMercenary);

	pWriter.writeChunk((void*)&(mActorData.mUnknowns), sizeof(mActorData.mUnknowns));

	if (pWriter.check() == false) {
		Log::error("[Actor] Bad Write: Written: " + std::to_string(pWriter.getBytesWritten()) + " Size: " + std::to_string(pWriter.getSize()));
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

const bool Actor::sendsEquipment() const {
	if (getActorType() == AT_PLAYER)
		return true;

	switch (mActorData.mRace) {
	case PlayableRaceIDs::Human:
	case PlayableRaceIDs::Barbarian:
	case PlayableRaceIDs::Erudite:
	case PlayableRaceIDs::WoodElf:
	case PlayableRaceIDs::HighElf:
	case PlayableRaceIDs::DarkElf:
	case PlayableRaceIDs::HalfElf:
	case PlayableRaceIDs::Dwarf:
	case PlayableRaceIDs::Troll:
	case PlayableRaceIDs::Ogre:
	case PlayableRaceIDs::Halfling:
	case PlayableRaceIDs::Gnome:
	case PlayableRaceIDs::Iksar:
	case PlayableRaceIDs::Vahshir:
	case PlayableRaceIDs::Froglok:
	case PlayableRaceIDs::Drakkin:
		return true;
	default:
		return false;
	}

}

void Actor::damage(const int32 pAmount) {
	mCurrentHP -= pAmount;

	if (mCurrentHP <= 0) {
 		setCurrentHP(0);
		setHPPercent(0);
		setIsDead(true);
	}
		
	else
		setHPPercent(_calcHPPercent());
}

void Actor::_clearPrimary() {
	setPrimaryDamage(0);
	setPrimaryFireDamage(0);
	setPrimaryColdDamage(0);
	setPrimaryPoisonDamage(0);
	setPrimaryDiseaseDamage(0);
	setPrimaryChromaticDamage(0);
	setPrimaryPrismaticDamage(0);
	setPrimaryPhysicalDamage(0);
	setPrimaryCorruptionDamage(0);

	setPrimaryAttackAnimation(Animation::ANIM_HAND2HAND);
}
