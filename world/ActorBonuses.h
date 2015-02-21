#pragma once

#include "Bonuses.h"
#include <list>

class ActorBonuses {
public:
	inline void add(Bonuses* pBonuses) { mBonuses.push_back(pBonuses); }
	inline std::list<Bonuses*>& getBonuses() { return mBonuses; }

	inline const i32 getCappedStrength() const { const auto v = getStrength(); const auto c = getHeroicStrength(); return v > c ? c : v; }
	inline const i32 getCappedStamina() const { const auto v = getStamina(); const auto c = getHeroicStamina(); return v > c ? c : v; }
	inline const i32 getCappedIntelligence() const { const auto v = getIntelligence(); const auto c = getHeroicIntelligence(); return v > c ? c : v; }
	inline const i32 getCappedWisdom() const { const auto v = getWisdom(); const auto c = getHeroicWisdom(); return v > c ? c : v; }
	inline const i32 getCappedAgility() const { const auto v = getAgility(); const auto c = getHeroicAgility(); return v > c ? c : v; }
	inline const i32 getCappedDexterity() const { const auto v = getDexterity(); const auto c = getHeroicAgility(); return v > c ? c : v; }
	inline const i32 getCappedCharisma() const { const auto v = getCharisma(); const auto c = getHeroicCharisma(); return v > c ? c : v; }

	inline const i32 getCappedMagicResist() const { const auto v = getMagicResist(); const auto c = getHeroicMagicResist(); return v > c ? c : v; }
	inline const i32 getCappedFireResist() const { const auto v = getFireResist(); const auto c = getHeroicFireResist(); return v > c ? c : v; }
	inline const i32 getCappedColdResist() const { const auto v = getColdResist(); const auto c = getHeroicColdResist(); return v > c ? c : v; }
	inline const i32 getCappedDiseaseResist() const { const auto v = getDiseaseResist(); const auto c = getHeroicDiseaseResist(); return v > c ? c : v; }
	inline const i32 getCappedPoisonResist() const { const auto v = getPoisonResist(); const auto c = getHeroicPoisonResist(); return v > c ? c : v; }
	inline const i32 getCappedCorruptionResist() const { const auto v = getCorruptionResist(); const auto c = getHeroicCorruptionResist(); return v > c ? c : v; }

	inline const i32 getStrength() const { i32 count = 0; for (auto i : mBonuses) count += i->getStrength(); return count; }
	inline const i32 getStamina() const { i32 count = 0; for (auto i : mBonuses) count += i->getStamina(); return count; }
	inline const i32 getIntelligence() const { i32 count = 0; for (auto i : mBonuses) count += i->getIntelligence(); return count; }
	inline const i32 getWisdom() const { i32 count = 0; for (auto i : mBonuses) count += i->getWisdom(); return count; }
	inline const i32 getAgility() const { i32 count = 0; for (auto i : mBonuses) count += i->getAgility(); return count; }
	inline const i32 getDexterity() const { i32 count = 0; for (auto i : mBonuses) count += i->getDexterity(); return count; }
	inline const i32 getCharisma() const { i32 count = 0; for (auto i : mBonuses) count += i->getCharisma(); return count; }

	inline const i32 getHeroicStrength() const { i32 count = 0; for (auto i : mBonuses) count += i->getHeroicStrength(); return count; }
	inline const i32 getHeroicStamina() const { i32 count = 0; for (auto i : mBonuses) count += i->getHeroicStamina(); return count; }
	inline const i32 getHeroicIntelligence() const { i32 count = 0; for (auto i : mBonuses) count += i->getHeroicIntelligence(); return count; }
	inline const i32 getHeroicWisdom() const { i32 count = 0; for (auto i : mBonuses) count += i->getHeroicWisdom(); return count; }
	inline const i32 getHeroicAgility() const { i32 count = 0; for (auto i : mBonuses) count += i->getHeroicAgility(); return count; }
	inline const i32 getHeroicDexterity() const { i32 count = 0; for (auto i : mBonuses) count += i->getHeroicDexterity(); return count; }
	inline const i32 getHeroicCharisma() const { i32 count = 0; for (auto i : mBonuses) count += i->getHeroicCharisma(); return count; }

	inline const i32 getMagicResist() const { i32 count = 0; for (auto i : mBonuses) count += i->getMagicResist(); return count; }
	inline const i32 getFireResist() const { i32 count = 0; for (auto i : mBonuses) count += i->getFireResist(); return count; }
	inline const i32 getColdResist() const { i32 count = 0; for (auto i : mBonuses) count += i->getColdResist(); return count; }
	inline const i32 getDiseaseResist() const { i32 count = 0; for (auto i : mBonuses) count += i->getDiseaseResist(); return count; }
	inline const i32 getPoisonResist() const { i32 count = 0; for (auto i : mBonuses) count += i->getPoisonResist(); return count; }
	inline const i32 getCorruptionResist() const { i32 count = 0; for (auto i : mBonuses) count += i->getCorruptionResist(); return count; }

	inline const i32 getHeroicMagicResist() const { i32 count = 0; for (auto i : mBonuses) count += i->getHeroicMagicResist(); return count; }
	inline const i32 getHeroicFireResist() const { i32 count = 0; for (auto i : mBonuses) count += i->getHeroicFireResist(); return count; }
	inline const i32 getHeroicColdResist() const { i32 count = 0; for (auto i : mBonuses) count += i->getHeroicColdResist(); return count; }
	inline const i32 getHeroicDiseaseResist() const { i32 count = 0; for (auto i : mBonuses) count += i->getHeroicDiseaseResist(); return count; }
	inline const i32 getHeroicPoisonResist() const { i32 count = 0; for (auto i : mBonuses) count += i->getHeroicPoisonResist(); return count; }
	inline const i32 getHeroicCorruptionResist() const { i32 count = 0; for (auto i : mBonuses) count += i->getHeroicCorruptionResist(); return count; }

	inline const i32 getArmorClass() const { i32 count = 0; for (auto i : mBonuses) count += i->getArmorClass(); return count; }
	inline const i32 getAttack() const { i32 count = 0; for (auto i : mBonuses) count += i->getAttack(); return count; }
	inline const i32 getHealth() const { i32 count = 0; for (auto i : mBonuses) count += i->getHealth(); return count; }
	inline const i32 getMana() const { i32 count = 0; for (auto i : mBonuses) count += i->getMana(); return count; }
	inline const i32 getEndurance() const { i32 count = 0; for (auto i : mBonuses) count += i->getEndurance(); return count; }

	inline const i32 getHealthRegen() const { i32 count = 0; for (auto i : mBonuses) count += i->getHealthRegen(); return count; }
	inline const i32 getManaRegen() const { i32 count = 0; for (auto i : mBonuses) count += i->getManaRegen(); return count; }
	inline const i32 getEnduranceRegen() const { i32 count = 0; for (auto i : mBonuses) count += i->getEnduranceRegen(); return count; }

private:

	std::list<Bonuses*> mBonuses;
};