#include "Actor.h"
#include "Utility.h"
#include "CombatData.h"

Actor::Actor() {
	mAttackerCombatData = new AttackerCombatData();
	mDefenderCombatData = new DefenderCombatData();
}

Actor::~Actor() {
	// Check: Actor has been cleaned up correctly before delete.
	if (mTarget != nullptr) { Log::error("[Actor] Target not null on destruction."); }
	if (mLooter != nullptr) { Log::error("[Actor] Looter not null on destruction."); }
	if (mVisibleTo.empty() == false) { Log::error("[Actor] VisibleTo not empty on destruction."); }
	if (mTargeters.empty() == false) { Log::error("[Actor] Targeters not empty on destruction."); }

	if (mAttackerCombatData->getDefenders().empty() == false) { Log::error("[Actor] Defenders not empty on destruction."); }
	if (mDefenderCombatData->getAttackers().empty() == false) { Log::error("[Actor] Attackers not empty on destruction."); }

	delete mAttackerCombatData;
	delete mDefenderCombatData;
	
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

	if (!sendsEquipment()) {
		result -= sizeof(mActorData.mColours);
		result -= sizeof(Payload::ActorData::Equipment) * 6;
	}

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

	// Check One.
	std::size_t chunk1Size = (unsigned int)&(mActorData.mColours) - (unsigned int)&(mActorData.mAAtitle);
	pStructure.writeChunk((void*)&(mActorData.mAAtitle), chunk1Size);

	if (sendsEquipment()) {
		// Write colours.
		pStructure.writeChunk((void*)&(mActorData.mColours), sizeof(mActorData.mColours));
		// Write equipment.
		pStructure.writeChunk((void*)&(mActorData.mEquipment), sizeof(mActorData.mEquipment));
	}
	else {
		pStructure.write<uint32>(0);
		pStructure.write<uint32>(0);
		pStructure.write<uint32>(0);

		pStructure.write<Payload::ActorData::Equipment>(mActorData.mEquipment[MaterialSlot::Primary]);
		pStructure.write<Payload::ActorData::Equipment>(mActorData.mEquipment[MaterialSlot::Secondary]);
	}

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

std::list<Actor*> Actor::getDefenders() { return mAttackerCombatData->getDefenders(); }

std::map<Actor*, AttackerData> Actor::getAttackers() { return mDefenderCombatData->getAttackers(); }

const bool Actor::hasAttackers() const { return mDefenderCombatData->hasAttackers(); }
const bool Actor::hasDefenders() const { return mAttackerCombatData->hasDefenders(); }
