#pragma once

#include <array>
#include "SpellContants.h"
#include "Payload.h"
#include "Bonuses.h"

class Actor;

class BuffController : public Bonuses {
public:
	BuffController(Actor* pOwner) : Bonuses("Buffs"), mOwner(pOwner) {};
	inline std::array<Payload::Zone::Buff, MaxBuffs>& getData() { return mData; }

	void update();
	void tick();

	// Returns whether the buff was added successfully.
	bool add(const u32 pSpellID, const u32 pDuration);
	void remove(const u32 pSlotID);
private:
	Actor* mOwner = nullptr;
	std::array<Payload::Zone::Buff, MaxBuffs> mData;

	void _clear(const u32 pSlot);
};
