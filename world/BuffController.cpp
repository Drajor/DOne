#include "BuffController.h"
#include "Data.h"
#include "Character.h"
#include "ZoneConnection.h"
#include "Payload.h"

BuffController::BuffController(Actor* pOwner) : Bonuses("Buffs"), mOwner(pOwner) {
	for (auto i = 0; i < MaxBuffs; i++)
		mData[i] = new Buff(i);
}

void BuffController::onLoad(std::list<Data::Buff*>& pBuffs) {
	for (auto i : pBuffs) {
		auto buff = mData[i->mSlotIndex];
		buff->mSpellID = i->mSpellID;
		buff->mTicksRemaining = i->mTicksRemaining;
	}
}

void BuffController::onSave(std::list<Data::Buff*>& pBuffs) const {
	for (auto i : mData) {
		if (i->inUse() == false) continue;

		auto buff = new Data::Buff();
		pBuffs.push_back(buff);

		buff->mSlotIndex = i->mSlotIndex;
		buff->mSpellID = i->mSpellID;
		buff->mTicksRemaining = i->mTicksRemaining;
	}
}

void BuffController::update() {
	for (auto i = 0; i < MaxBuffs; i++) {
		if (mData[i]->inUse())
			continue;

		auto buff = mData[i];
		//buff.
	}
}

void BuffController::tick() {

}

bool BuffController::add(const u32 pSpellID, const u32 pDuration) {
	// Find a free slot.
	auto buff = findFirstFree();
	if (!buff) return false;

	//auto buff = mData[slotID];
	//buff.mSpellID = pSpellID;
	//buff.mDuration = pDuration;
	//buff.mUnknown1 = 0x3f800000; // Magic!
	//buff.mType = 2; // Magic!
	//buff.mCasterSpawnID = 0x000717fd;

	buff->mSpellID = pSpellID;
	buff->mTicksRemaining = pDuration;

	// Add buff icon to Character.
	if (mOwner->isCharacter()) {
		auto packet = Payload::updateBuffIcon(mOwner->getSpawnID(), buff);
		Actor::cast<Character*>(mOwner)->getConnection()->sendPacket(packet);
		delete packet;
	}

	return true;
}

void BuffController::remove(const u32 pSlotID) {
	using namespace Payload::Zone;
	// Check: Slot range is valid.
	if (pSlotID >= MaxBuffs) {
		return;
	}

	Character* character = nullptr;

	if (mOwner->isCharacter())
		character = Actor::cast<Character*>(mOwner);

	// Remove buff from Character.
	if (character) {
		auto packet = UpdateBuff::construct(character->getSpawnID(), 2, mData[pSlotID]->mSpellID, pSlotID, 1);
		character->getConnection()->sendPacket(packet);
		delete packet;
	}

	// Clear
	_clear(pSlotID);

	// Remove buff icon from Character.
	if (character) {
		auto packet = Payload::updateBuffIcon(character->getSpawnID(), mData[pSlotID]);
		character->getConnection()->sendPacket(packet);
		delete packet;
	}
}

void BuffController::_clear(const u32 pSlot) {
	auto buff = mData[pSlot];
	buff->mSpellID = 0xFFFFFFFF;
	buff->mTicksRemaining = 0;
	buff->mHitsRemaining = 0;
	//buff.mDuration = 0;
	//buff.mUnknown1 = 0;
	//buff.mType = 0;
	//buff.mCasterSpawnID = 0;
}

Buff* BuffController::findFirstFree() const {
	for (auto i : mData) {
		if (i->inUse() == false) return i;
	}
	
	return nullptr;
}
