#pragma once

#include <array>
#include "SpellContants.h"
#include "Payload.h"
#include "Bonuses.h"

class BuffController : public Bonuses {
public:
	BuffController() : Bonuses("Buffs") {};
	inline std::array<Payload::Zone::Buff, MaxBuffs>& getData() { return mData; }

	// Returns whether the buff was added successfully.
	bool add(const u32 pSpellID, const u32 pDuration);
	void remove();
private:
	std::array<Payload::Zone::Buff, MaxBuffs> mData;
};
