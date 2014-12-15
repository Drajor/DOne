#include "CombatData.h"
#include "Actor.h"
#include "LogSystem.h"

void DefenderCombatData::add(Actor* pActor, const u32 pHate, const u32 pDamage) {
	auto record = find(pActor);

	// New entry
	if (!record) {
		AttackerData r(pHate, pDamage);
		mAttackers.insert(std::make_pair(pActor, r));
		Log::info("[DefenderCombatData] Adding " + pActor->getName());
		return;
	}

	// Existing entry.
	record->mHate += pHate;
	record->mDamage += pDamage;
}

void DefenderCombatData::remove(Actor* pActor) {
	Log::info("[DefenderCombatData] Removing " + pActor->getName());
	mAttackers.erase(pActor);
}

AttackerData* DefenderCombatData::find(Actor* pActor) {
	auto r = mAttackers.find(pActor);
	if (r != mAttackers.end())
		return &(*r).second;
	return nullptr;
}

void AttackerCombatData::add(Actor* pActor) {
	if (!search(pActor)) {
		Log::info("[AttackerCombatData] Adding " + pActor->getName());
		mDefenders.push_back(pActor);
	}
}

const bool AttackerCombatData::search(Actor* pActor) const {
	for (auto i : mDefenders) {
		if (i == pActor)
			return true;
	}

	return false;
}

void AttackerCombatData::remove(Actor* pActor) {
	Log::info("[AttackerCombatData] Removing " + pActor->getName());
	mDefenders.remove(pActor);
}