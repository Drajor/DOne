#include "SpellDataStore.h"
#include "DataStore.h"
#include "Utility.h"
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

