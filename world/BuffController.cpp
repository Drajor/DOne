#include "BuffController.h"

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

	return true;
}
