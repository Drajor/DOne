#pragma once

#include "Constants.h"
#include "Singleton.h"

class Zone;
struct SpellData;
class SpellDataStore : public Singleton<SpellDataStore> {
private:
	friend class Singleton<SpellDataStore>;
	SpellDataStore() {};
	~SpellDataStore() {};
	SpellDataStore(SpellDataStore const&); // Do not implement.
	void operator=(SpellDataStore const&); // Do not implement.
public:
	const bool initialise();
	const SpellData* getData(const uint16 pSpellID);
private:
	SpellData* mSpellData = nullptr;
};

namespace Spell {
	const SpellData* get(const uint16 pSpellID);
	const bool canClassUse(const SpellData* pSpell, const uint8 pClassID, const uint8 pLevel);
	const bool zoneAllowed(const SpellData* pSpell, const Zone* pZone);
}