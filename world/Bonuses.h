#pragma once

#include "Types.h"

class Bonuses {
public:
	Bonuses(const String& pName) : mName(pName) {};
	inline const String& getName() const { return mName; }

	inline const i32 getStrength() const { return mStrength; }
	inline const i32 getStamina() const { return mStamina; }
	inline const i32 getIntelligence() const { return mIntelligence; }
	inline const i32 getWisdom() const { return mWisdom; }
	inline const i32 getAgility() const { return mAgility; }
	inline const i32 getDexterity() const { return mDexterity; }
	inline const i32 getCharisma() const { return mCharisma; }

	inline const i32 getHeroicStrength() const { return mHeroicStrength; }
	inline const i32 getHeroicStamina() const { return mHeroicStamina; }
	inline const i32 getHeroicIntelligence() const { return mHeroicIntelligence; }
	inline const i32 getHeroicWisdom() const { return mHeroicWisdom; }
	inline const i32 getHeroicAgility() const { return mHeroicAgility; }
	inline const i32 getHeroicDexterity() const { return mHeroicDexterity; }
	inline const i32 getHeroicCharisma() const { return mHeroicCharisma; }

	inline const i32 getMagicResist() const { return mMagicResist; }
	inline const i32 getFireResist() const { return mFireResist; }
	inline const i32 getColdResist() const { return mColdResist; }
	inline const i32 getDiseaseResist() const { return mDiseaseResist; }
	inline const i32 getPoisonResist() const { return mPoisonResist; }
	inline const i32 getCorruptionResist() const { return mCorruptionResist; }

	inline const i32 getHeroicMagicResist() const { return mHeroicMagicResist; }
	inline const i32 getHeroicFireResist() const { return mHeroicFireResist; }
	inline const i32 getHeroicColdResist() const { return mHeroicColdResist; }
	inline const i32 getHeroicDiseaseResist() const { return mHeroicDiseaseResist; }
	inline const i32 getHeroicPoisonResist() const { return mHeroicPoisonResist; }
	inline const i32 getHeroicCorruptionResist() const { return mHeroicCorruptionResist; }

	inline const i32 getArmorClass() const { return mArmorClass; }
	inline const i32 getAttack() const { return mAttack; }
	inline const i32 getHealth() const { return mHealth; }
	inline const i32 getMana() const { return mMana; }
	inline const i32 getEndurance() const { return mEndurance; }

	inline const i32 getHealthRegen() const { return mHealthRegen; }
	inline const i32 getManaRegen() const { return mManaRegen; }
	inline const i32 getEnduranceRegen() const { return mEnduranceRegen; }

	// Add

	inline void _addStrength(const i32 pValue) { mStrength += pValue; }
	inline void _addStamina(const i32 pValue) { mStamina += pValue; }
	inline void _addIntelligence(const i32 pValue) { mIntelligence += pValue; }
	inline void _addWisdom(const i32 pValue) { mWisdom += pValue; }
	inline void _addAgility(const i32 pValue) { mAgility += pValue; }
	inline void _addDexterity(const i32 pValue) { mDexterity += pValue; }
	inline void _addCharisma(const i32 pValue) { mCharisma += pValue; }

	inline void _addHeroicStrength(const i32 pValue) { mHeroicStrength += pValue; }
	inline void _addHeroicStamina(const i32 pValue) { mHeroicStamina += pValue; }
	inline void _addHeroicIntelligence(const i32 pValue) { mHeroicIntelligence += pValue; }
	inline void _addHeroicWisdom(const i32 pValue) { mHeroicWisdom += pValue; }
	inline void _addHeroicAgility(const i32 pValue) { mHeroicAgility += pValue; }
	inline void _addHeroicDexterity(const i32 pValue) { mHeroicDexterity += pValue; }
	inline void _addHeroicCharisma(const i32 pValue) { mHeroicCharisma += pValue; }

	inline void _addMagicResist(const i32 pValue) { mMagicResist += pValue; }
	inline void _addFireResist(const i32 pValue) { mFireResist += pValue; }
	inline void _addColdResist(const i32 pValue) { mColdResist += pValue; }
	inline void _addDiseaseResist(const i32 pValue) { mDiseaseResist += pValue; }
	inline void _addPoisonResist(const i32 pValue) { mPoisonResist += pValue; }
	inline void _addCorruptionResist(const i32 pValue) { mCorruptionResist += pValue; }

	inline void _addHeroicMagicResist(const i32 pValue) { mHeroicMagicResist += pValue; }
	inline void _addHeroicFireResist(const i32 pValue) { mHeroicFireResist += pValue; }
	inline void _addHeroicColdResist(const i32 pValue) { mHeroicColdResist += pValue; }
	inline void _addHeroicDiseaseResist(const i32 pValue) { mHeroicDiseaseResist += pValue; }
	inline void _addHeroicPoisonResist(const i32 pValue) { mHeroicPoisonResist += pValue; }
	inline void _addHeroicCorruptionResist(const i32 pValue) { mHeroicCorruptionResist += pValue; }

	inline void _addArmorClass(const i32 pValue) { mArmorClass += pValue; }
	inline void _addAttack(const i32 pValue) { mAttack += pValue; }
	inline void _addHealth(const i32 pValue) { mHealth += pValue; }
	inline void _addMana(const i32 pValue) { mMana += pValue; }
	inline void _addEndurance(const i32 pValue) { mEndurance += pValue; }

	inline void _addHealthRegen(const i32 pValue) { mHealthRegen += pValue; }
	inline void _addManaRegen(const i32 pValue) { mManaRegen += pValue; }
	inline void _addEnduranceRegen(const i32 pValue) { mEnduranceRegen += pValue; }

	// Remove

	inline void _removeStrength(const i32 pValue) { mStrength -= pValue; }
	inline void _removeStamina(const i32 pValue) { mStamina -= pValue; }
	inline void _removeIntelligence(const i32 pValue) { mIntelligence -= pValue; }
	inline void _removeWisdom(const i32 pValue) { mWisdom -= pValue; }
	inline void _removeAgility(const i32 pValue) { mAgility -= pValue; }
	inline void _removeDexterity(const i32 pValue) { mDexterity -= pValue; }
	inline void _removeCharisma(const i32 pValue) { mCharisma -= pValue; }

	inline void _removeHeroicStrength(const i32 pValue) { mHeroicStrength -= pValue; }
	inline void _removeHeroicStamina(const i32 pValue) { mHeroicStamina -= pValue; }
	inline void _removeHeroicIntelligence(const i32 pValue) { mHeroicIntelligence -= pValue; }
	inline void _removeHeroicWisdom(const i32 pValue) { mHeroicWisdom -= pValue; }
	inline void _removeHeroicAgility(const i32 pValue) { mHeroicAgility -= pValue; }
	inline void _removeHeroicDexterity(const i32 pValue) { mHeroicDexterity -= pValue; }
	inline void _removeHeroicCharisma(const i32 pValue) { mHeroicCharisma -= pValue; }

	inline void _removeMagicResist(const i32 pValue) { mMagicResist -= pValue; }
	inline void _removeFireResist(const i32 pValue) { mFireResist -= pValue; }
	inline void _removeColdResist(const i32 pValue) { mColdResist -= pValue; }
	inline void _removeDiseaseResist(const i32 pValue) { mDiseaseResist -= pValue; }
	inline void _removePoisonResist(const i32 pValue) { mPoisonResist -= pValue; }
	inline void _removeCorruptionResist(const i32 pValue) { mCorruptionResist -= pValue; }

	inline void _removeHeroicMagicResist(const i32 pValue) { mHeroicMagicResist -= pValue; }
	inline void _removeHeroicFireResist(const i32 pValue) { mHeroicFireResist -= pValue; }
	inline void _removeHeroicColdResist(const i32 pValue) { mHeroicColdResist -= pValue; }
	inline void _removeHeroicDiseaseResist(const i32 pValue) { mHeroicDiseaseResist -= pValue; }
	inline void _removeHeroicPoisonResist(const i32 pValue) { mHeroicPoisonResist -= pValue; }
	inline void _removeHeroicCorruptionResist(const i32 pValue) { mHeroicCorruptionResist -= pValue; }

	inline void _removeArmorClass(const i32 pValue) { mArmorClass -= pValue; }
	inline void _removeAttack(const i32 pValue) { mAttack -= pValue; }
	inline void _removeHealth(const i32 pValue) { mHealth -= pValue; }
	inline void _removeMana(const i32 pValue) { mMana -= pValue; }
	inline void _removeEndurance(const i32 pValue) { mEndurance -= pValue; }

	inline void _removeHealthRegen(const i32 pValue) { mHealthRegen -= pValue; }
	inline void _removeManaRegen(const i32 pValue) { mManaRegen -= pValue; }
	inline void _removeEnduranceRegen(const i32 pValue) { mEnduranceRegen -= pValue; }

private:

	String mName;

	i32 mStrength = 0;
	i32 mStamina = 0;
	i32 mIntelligence = 0;
	i32 mWisdom = 0;
	i32 mAgility = 0;
	i32 mDexterity = 0;
	i32 mCharisma = 0;

	i32 mHeroicStrength = 0;
	i32 mHeroicStamina = 0;
	i32 mHeroicIntelligence = 0;
	i32 mHeroicWisdom = 0;
	i32 mHeroicAgility = 0;
	i32 mHeroicDexterity = 0;
	i32 mHeroicCharisma = 0;

	i32 mMagicResist = 0;
	i32 mFireResist = 0;
	i32 mColdResist = 0;
	i32 mDiseaseResist = 0;
	i32 mPoisonResist = 0;
	i32 mCorruptionResist = 0;

	i32 mHeroicMagicResist = 0;
	i32 mHeroicFireResist = 0;
	i32 mHeroicColdResist = 0;
	i32 mHeroicDiseaseResist = 0;
	i32 mHeroicPoisonResist = 0;
	i32 mHeroicCorruptionResist = 0;

	i32 mArmorClass = 0;
	i32 mAttack = 0;
	i32 mHealth = 0;
	i32 mMana = 0;
	i32 mEndurance = 0;

	i32 mHealthRegen = 0;
	i32 mManaRegen = 0;
	i32 mEnduranceRegen = 0;
};