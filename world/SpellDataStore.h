#pragma once

#include "SpellContants.h"
#include "Types.h"
#include <array>
#include <list>

namespace Data {
	struct Spell;
}

// For the #spellsearch command.
struct SpellSearchEntry {
	u32 mID;
	String mName;
};
typedef std::list<SpellSearchEntry> SpellSearchResults;

class ILog;
class ILogFactory;
class IDataStore;
class Zone;

class SpellDataStore {
public:

	~SpellDataStore();

	// Initialises the SpellDataStore.
	const bool initialise(IDataStore* pDataStore, ILogFactory* pLogFactory);
	const Data::Spell* getData(const u16 pSpellID);

	void searchByName(const String& pName, SpellSearchResults& pResults) const;

private:

	bool mInitialised = false;
	ILog* mLog = nullptr;
	IDataStore* mDataStore = nullptr;

	std::array<Data::Spell*, MaxSpellID> mData;
};

namespace Spell {
	const bool canClassUse(const Data::Spell* pSpell, const u8 pClassID, const u8 pLevel);
	const bool zoneAllowed(const Data::Spell* pSpell, const Zone* pZone);
}