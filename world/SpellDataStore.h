#pragma once

#include "SpellContants.h"
#include "Types.h"
#include <array>

namespace Data {
	struct Spell;
}

class ILog;
class ILogFactory;
class IDataStore;
class Zone;

typedef std::array<Data::Spell, MaxSpellID> SpellDataArray;

class SpellDataStore {
public:

	~SpellDataStore();

	// Initialises the SpellDataStore.
	const bool initialise(IDataStore* pDataStore, ILogFactory* pLogFactory);
	const Data::Spell* getData(const u16 pSpellID);

private:

	bool mInitialised = false;
	ILog* mLog = nullptr;
	IDataStore* mDataStore = nullptr;

	Data::Spell* mData = nullptr;
};

namespace Spell {
	const bool canClassUse(const Data::Spell* pSpell, const u8 pClassID, const u8 pLevel);
	const bool zoneAllowed(const Data::Spell* pSpell, const Zone* pZone);
}