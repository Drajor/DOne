#include "SpellDataStore.h"
#include "ServiceLocator.h"
#include "DataStore.h"
#include "Utility.h"
#include "Limits.h"
#include "Data.h"
#include "Profile.h"

const bool SpellDataStore::initialise(DataStore* pDataStore) {
	EXPECTED_BOOL(mInitialised == false);
	EXPECTED_BOOL(pDataStore);

	Profile p("SpellDataStore::initialise");
	Log::status("[Spell Data Store] Initialising.");

	mDataStore = pDataStore;

	// Allocate block of memory for spells.
	mSpellData = new Data::Spell[Limits::Spells::MAX_SPELL_ID];

	// DataStore to load and populate.
	uint32 numSpellsLoaded = 0;
	EXPECTED_BOOL(mDataStore->loadSpells(mSpellData, numSpellsLoaded));

	Log::info("[Spell Data Store] Loaded data for " + std::to_string(numSpellsLoaded) + " Spells.");

	mInitialised = true;
	return true;
}

const Data::Spell* SpellDataStore::getData(const u16 pSpellID) {
	EXPECTED_PTR(Limits::Spells::spellIDValid(pSpellID));
	
	auto spell = &mSpellData[pSpellID];
	EXPECTED_PTR(spell);
	EXPECTED_PTR(spell->mInUse);

	return spell;
}

namespace Spell {
	const bool canClassUse(const Data::Spell* pSpell, const u8 pClassID, const u8 pLevel) {
		EXPECTED_BOOL(pSpell);
		EXPECTED_BOOL(Limits::Character::classID(pClassID));
		return pSpell->mRequiredClassLevels[pClassID] <= pLevel;
	}

	const bool zoneAllowed(const Data::Spell* pSpell, const Zone* pZone) {
		EXPECTED_BOOL(pSpell);
		EXPECTED_BOOL(pZone);
		// TODO:
		return true;
	}

}