#pragma once

#include <array>
#include <list>
#include "SpellContants.h"
#include "Payload.h"
#include "Bonuses.h"

namespace Data {
	struct Buff;
}

class Actor;

struct Buff {
	Buff(const u32 pSlotIndex) : mSlotIndex(pSlotIndex) {};

	inline const bool inUse() const { return mSpellID != 0xFFFFFFFF; }
	const u32 mSlotIndex;
	u32 mSpellID = 0xFFFFFFFF;
	u32 mTicksRemaining = 0;
	u32 mHitsRemaining = 0;
	u8 mCasterLevel = 0; // The level of the Actor of the caster.
	u8 mActorLevel = 0; // The level of the Actor that owns the buff.
	u8 mInstrumentModifier = 0;
};

class BuffController : public Bonuses {
public:
	BuffController(Actor* pOwner);;
	inline std::array<Buff*, MaxBuffs>& getData() { return mData; }

	void onLoad(std::list<Data::Buff*>& pBuffs);
	void onSave(std::list<Data::Buff*>& pBuffs) const;

	void update();
	void tick();

	// Returns whether the buff was added successfully.
	bool add(const u32 pSpellID, const u32 pDuration);
	void remove(const u32 pSlotID);
private:
	Actor* mOwner = nullptr;
	std::array<Buff*, MaxBuffs> mData;

	Buff* findFirstFree() const;
	void _clear(const u32 pSlot);
};
