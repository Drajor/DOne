#pragma once

#include "Types.h"

class DataStore;
class Zone;

namespace Data {
	struct Spell;
}

class SpellDataStore {
public:

	const bool initialise(DataStore* pDataStore);
	const Data::Spell* getData(const u16 pSpellID);

private:

	bool mInitialised = false;
	DataStore* mDataStore = nullptr;
	Data::Spell* mSpellData = nullptr;
};

namespace Spell {
	const bool canClassUse(const Data::Spell* pSpell, const u8 pClassID, const u8 pLevel);
	const bool zoneAllowed(const Data::Spell* pSpell, const Zone* pZone);
}