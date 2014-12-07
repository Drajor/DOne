#pragma once

#include "Types.h"
#include "Singleton.h"

class Zone;

namespace Data {
	struct Spell;
}

class SpellDataStore : public Singleton<SpellDataStore> {
private:
	friend class Singleton<SpellDataStore>;
	SpellDataStore() {};
	~SpellDataStore() {};
	SpellDataStore(SpellDataStore const&); // Do not implement.
	void operator=(SpellDataStore const&); // Do not implement.
public:
	const bool initialise();
	const Data::Spell* getData(const uint16 pSpellID);
private:
	Data::Spell* mSpellData = nullptr;
};

namespace Spell {
	const Data::Spell* get(const uint16 pSpellID);
	const bool canClassUse(const Data::Spell* pSpell, const uint8 pClassID, const uint8 pLevel);
	const bool zoneAllowed(const Data::Spell* pSpell, const Zone* pZone);
}