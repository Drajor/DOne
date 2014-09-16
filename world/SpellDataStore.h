#pragma once

#include "Constants.h"
#include "Singleton.h"
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
private:
	SpellData* mSpellData = nullptr;
};