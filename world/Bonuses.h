#pragma once

#include "Constants.h"

class Bonuses {
public:
	inline const int32 getStrength() const { return mStrength; }
	inline const int32 getStamina() const { return mStamina; }
	inline const int32 getIntelligence() const { return mIntelligence; }
	inline const int32 getWisdom() const { return mWisdom; }
	inline const int32 getAgility() const { return mAgility; }
	inline const int32 getDexterity() const { return mDexterity; }
	inline const int32 getCharisma() const { return mCharisma; }

	inline const int32 getHeroicStrength() const { return mHeroicStrength; }
	inline const int32 getHeroicStamina() const { return mHeroicStamina; }
	inline const int32 getHeroicIntelligence() const { return mHeroicIntelligence; }
	inline const int32 getHeroicWisdom() const { return mHeroicWisdom; }
	inline const int32 getHeroicAgility() const { return mHeroicAgility; }
	inline const int32 getHeroicDexterity() const { return mHeroicDexterity; }
	inline const int32 getHeroicCharisma() const { return mHeroicCharisma; }

	inline const int32 getMagicResist() const { return mMagicResist; }
	inline const int32 getFireResist() const { return mFireResist; }
	inline const int32 getColdResist() const { return mColdResist; }
	inline const int32 getDiseaseResist() const { return mDiseaseResist; }
	inline const int32 getPoisonResist() const { return mPoisonResist; }
	inline const int32 getCorruptionResist() const { return mCorruptionResist; }

	inline const int32 getHeroicMagicResist() const { return mHeroicMagicResist; }
	inline const int32 getHeroicFireResist() const { return mHeroicFireResist; }
	inline const int32 getHeroicColdResist() const { return mHeroicColdResist; }
	inline const int32 getHeroicDiseaseResist() const { return mHeroicDiseaseResist; }
	inline const int32 getHeroicPoisonResist() const { return mHeroicPoisonResist; }
	inline const int32 getHeroicCorruptionResist() const { return mHeroicCorruptionResist; }

	inline const int32 getHealth() const { return mHealth; }
	inline const int32 getMana() const { return mMana; }
	inline const int32 getEndurance() const { return mEndurance; }

	inline const int32 getHealthRegen() const { return mHealthRegen; }
	inline const int32 getManaRegen() const { return mManaRegen; }
	inline const int32 getEnduranceRegen() const { return mEnduranceRegen; }

protected:

	// Add

	inline void _addStrength(const int32 pValue) { mStrength += pValue; }
	inline void _addStamina(const int32 pValue) { mStamina += pValue; }
	inline void _addIntelligence(const int32 pValue) { mIntelligence += pValue; }
	inline void _addWisdom(const int32 pValue) { mWisdom += pValue; }
	inline void _addAgility(const int32 pValue) { mAgility += pValue; }
	inline void _addDexterity(const int32 pValue) { mDexterity += pValue; }
	inline void _addCharisma(const int32 pValue) { mCharisma += pValue; }

	inline void _addHeroicStrength(const int32 pValue) { mHeroicStrength += pValue; }
	inline void _addHeroicStamina(const int32 pValue) { mHeroicStamina += pValue; }
	inline void _addHeroicIntelligence(const int32 pValue) { mHeroicIntelligence += pValue; }
	inline void _addHeroicWisdom(const int32 pValue) { mHeroicWisdom += pValue; }
	inline void _addHeroicAgility(const int32 pValue) { mHeroicAgility += pValue; }
	inline void _addHeroicDexterity(const int32 pValue) { mHeroicDexterity += pValue; }
	inline void _addHeroicCharisma(const int32 pValue) { mHeroicCharisma += pValue; }

	inline void _addMagicResist(const int32 pValue) { mMagicResist += pValue; }
	inline void _addFireResist(const int32 pValue) { mFireResist += pValue; }
	inline void _addColdResist(const int32 pValue) { mColdResist += pValue; }
	inline void _addDiseaseResist(const int32 pValue) { mDiseaseResist += pValue; }
	inline void _addPoisonResist(const int32 pValue) { mPoisonResist += pValue; }
	inline void _addCorruptionResist(const int32 pValue) { mCorruptionResist += pValue; }

	inline void _addHeroicMagicResist(const int32 pValue) { mHeroicMagicResist += pValue; }
	inline void _addHeroicFireResist(const int32 pValue) { mHeroicFireResist += pValue; }
	inline void _addHeroicColdResist(const int32 pValue) { mHeroicColdResist += pValue; }
	inline void _addHeroicDiseaseResist(const int32 pValue) { mHeroicDiseaseResist += pValue; }
	inline void _addHeroicPoisonResist(const int32 pValue) { mHeroicPoisonResist += pValue; }
	inline void _addHeroicCorruptionResist(const int32 pValue) { mHeroicCorruptionResist += pValue; }

	inline void _addHealth(const int32 pValue) { mHealth += pValue; }
	inline void _addMana(const int32 pValue) { mMana += pValue; }
	inline void _addEndurance(const int32 pValue) { mEndurance += pValue; }

	inline void _addHealthRegen(const int32 pValue) { mHealthRegen += pValue; }
	inline void _addManaRegen(const int32 pValue) { mManaRegen += pValue; }
	inline void _addEnduranceRegen(const int32 pValue) { mEnduranceRegen += pValue; }

	// Remove

	inline void _removeStrength(const int32 pValue) { mStrength -= pValue; }
	inline void _removeStamina(const int32 pValue) { mStamina -= pValue; }
	inline void _removeIntelligence(const int32 pValue) { mIntelligence -= pValue; }
	inline void _removeWisdom(const int32 pValue) { mWisdom -= pValue; }
	inline void _removeAgility(const int32 pValue) { mAgility -= pValue; }
	inline void _removeDexterity(const int32 pValue) { mDexterity -= pValue; }
	inline void _removeCharisma(const int32 pValue) { mCharisma -= pValue; }

	inline void _removeHeroicStrength(const int32 pValue) { mHeroicStrength -= pValue; }
	inline void _removeHeroicStamina(const int32 pValue) { mHeroicStamina -= pValue; }
	inline void _removeHeroicIntelligence(const int32 pValue) { mHeroicIntelligence -= pValue; }
	inline void _removeHeroicWisdom(const int32 pValue) { mHeroicWisdom -= pValue; }
	inline void _removeHeroicAgility(const int32 pValue) { mHeroicAgility -= pValue; }
	inline void _removeHeroicDexterity(const int32 pValue) { mHeroicDexterity -= pValue; }
	inline void _removeHeroicCharisma(const int32 pValue) { mHeroicCharisma -= pValue; }

	inline void _removeMagicResist(const int32 pValue) { mMagicResist -= pValue; }
	inline void _removeFireResist(const int32 pValue) { mFireResist -= pValue; }
	inline void _removeColdResist(const int32 pValue) { mColdResist -= pValue; }
	inline void _removeDiseaseResist(const int32 pValue) { mDiseaseResist -= pValue; }
	inline void _removePoisonResist(const int32 pValue) { mPoisonResist -= pValue; }
	inline void _removeCorruptionResist(const int32 pValue) { mCorruptionResist -= pValue; }

	inline void _removeHeroicMagicResist(const int32 pValue) { mHeroicMagicResist -= pValue; }
	inline void _removeHeroicFireResist(const int32 pValue) { mHeroicFireResist -= pValue; }
	inline void _removeHeroicColdResist(const int32 pValue) { mHeroicColdResist -= pValue; }
	inline void _removeHeroicDiseaseResist(const int32 pValue) { mHeroicDiseaseResist -= pValue; }
	inline void _removeHeroicPoisonResist(const int32 pValue) { mHeroicPoisonResist -= pValue; }
	inline void _removeHeroicCorruptionResist(const int32 pValue) { mHeroicCorruptionResist -= pValue; }

	inline void _removeHealth(const int32 pValue) { mHealth -= pValue; }
	inline void _removeMana(const int32 pValue) { mMana -= pValue; }
	inline void _removeEndurance(const int32 pValue) { mEndurance -= pValue; }

	inline void _removeHealthRegen(const int32 pValue) { mHealthRegen -= pValue; }
	inline void _removeManaRegen(const int32 pValue) { mManaRegen -= pValue; }
	inline void _removeEnduranceRegen(const int32 pValue) { mEnduranceRegen -= pValue; }

private:

	int32 mStrength = 0;
	int32 mStamina = 0;
	int32 mIntelligence = 0;
	int32 mWisdom = 0;
	int32 mAgility = 0;
	int32 mDexterity = 0;
	int32 mCharisma = 0;

	int32 mHeroicStrength = 0;
	int32 mHeroicStamina = 0;
	int32 mHeroicIntelligence = 0;
	int32 mHeroicWisdom = 0;
	int32 mHeroicAgility = 0;
	int32 mHeroicDexterity = 0;
	int32 mHeroicCharisma = 0;

	int32 mMagicResist = 0;
	int32 mFireResist = 0;
	int32 mColdResist = 0;
	int32 mDiseaseResist = 0;
	int32 mPoisonResist = 0;
	int32 mCorruptionResist = 0;

	int32 mHeroicMagicResist = 0;
	int32 mHeroicFireResist = 0;
	int32 mHeroicColdResist = 0;
	int32 mHeroicDiseaseResist = 0;
	int32 mHeroicPoisonResist = 0;
	int32 mHeroicCorruptionResist = 0;

	int32 mHealth = 0;
	int32 mMana = 0;
	int32 mEndurance = 0;

	int32 mHealthRegen = 0;
	int32 mManaRegen = 0;
	int32 mEnduranceRegen = 0;
};