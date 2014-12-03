#include "SpawnGroup.h"
#include "Random.h"

const u32 SpawnGroup::roll() {
	// TODO: Make weighted rolls.
	auto i = mEntries.begin();
	std::advance(i, Random::make<u32>(0, mEntries.size() - 1));
	return i->first;
}
