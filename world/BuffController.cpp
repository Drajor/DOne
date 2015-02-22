#include "BuffController.h"
#include "Character.h"
#include "ZoneConnection.h"
#include "Payload.h"

void BuffController::update() {
	for (auto i = 0; i < MaxBuffs; i++) {
		if (mData[i].mSpellID != 0xFFFFFFFF)
			continue;

		auto& buff = mData[i];
		//buff.
	}
}

void BuffController::tick() {

}

bool BuffController::add(const u32 pSpellID, const u32 pDuration) {
	// Find a free slot.
	auto slotID = -1;
	for (auto i = 0; i < MaxBuffs; i++) {
		if (mData[i].mSpellID != 0xFFFFFFFF)
			continue;

		slotID = i;
		break;
	}

	// No free slot found.
	if (slotID == -1) return false;

	auto& buff = mData[slotID];
	buff.mSpellID = pSpellID;
	buff.mDuration = pDuration;
	buff.mUnknown1 = 0x3f800000; // Magic!
	buff.mType = 2; // Magic!
	buff.mCasterSpawnID = 0x000717fd;

	// Add buff icon to Character.
	if (mOwner->isCharacter()) {
		auto character = Actor::cast<Character*>(mOwner);

		auto packet = Payload::updateBuffIcon(character->getSpawnID(), slotID, buff);
		character->getConnection()->sendPacket(packet);
		delete packet;
	}

	return true;
}

void BuffController::remove(const u32 pSlotID) {
	// Check: Slot range is valid.
	if (pSlotID >= MaxBuffs) {
		return;
	}

	Character* character = nullptr;

	if (mOwner->isCharacter())
		character = Actor::cast<Character*>(mOwner);

	// Remove buff from Character.
	if (character) {
		auto packet = Payload::Zone::UpdateBuff::construct(character->getSpawnID(), 2, mData[pSlotID].mSpellID, pSlotID, 1);
		character->getConnection()->sendPacket(packet);
		delete packet;
	}

	// Clear
	_clear(pSlotID);

	// Remove buff icon from Character.
	if (character) {
		auto packet = Payload::updateBuffIcon(character->getSpawnID(), pSlotID, mData[pSlotID]);
		character->getConnection()->sendPacket(packet);
		delete packet;
	}
}

void BuffController::_clear(const u32 pSlot) {
	auto& buff = mData[pSlot];
	buff.mSpellID = 0xFFFFFFFF;
	buff.mDuration = 0;
	buff.mUnknown1 = 0;
	buff.mType = 0;
	buff.mCasterSpawnID = 0;
}
