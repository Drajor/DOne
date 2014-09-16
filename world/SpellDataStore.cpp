#include "SpellDataStore.h"
#include "DataStore.h"
#include "Utility.h"
#include "Limits.h"
#include "Data.h"
#include "Profile.h"

const bool SpellDataStore::initialise() {
	Profile p("SpellDataStore::initialise");
	Log::status("[Spell Data Store] Initialising.");

	// Allocate block of memory spells
	mSpellData = new SpellData[Limits::Spells::MAX_SPELL_ID];

	// DataStore to load and populate.
	uint32 numSpellsLoaded = 0;
	EXPECTED_BOOL(DataStore::getInstance().loadSpells(mSpellData, numSpellsLoaded));

	Log::info("[Spell Data Store] Loaded data for " + std::to_string(numSpellsLoaded) + " Spells.");
	return true;
}

const SpellData* SpellDataStore::getData(const uint16 pSpellID) {
	EXPECTED_PTR(Limits::Spells::spellIDValid(pSpellID));
	
	auto spell = &mSpellData[pSpellID];
	EXPECTED_PTR(spell);
	EXPECTED_PTR(spell->mInUse);

	return spell;
}

namespace Spell {
	const SpellData* get(const uint16 pSpellID) {
		return SpellDataStore::getInstance().getData(pSpellID);
	}

	const bool canClassUse(const SpellData* pSpell, const uint8 pClassID, const uint8 pLevel) {
		EXPECTED_BOOL(pSpell);
		EXPECTED_BOOL(Limits::Character::classID(pClassID));
		return pSpell->mRequiredClassLevels[pClassID] <= pLevel;
	}

	const bool zoneAllowed(const SpellData* pSpell, const Zone* pZone) {
		EXPECTED_BOOL(pSpell);
		EXPECTED_BOOL(pZone);
		// TODO:
		return true;
	}

}