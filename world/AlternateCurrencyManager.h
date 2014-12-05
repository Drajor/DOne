#pragma once

#include "Types.h"
#include "Singleton.h"
#include <list>

namespace Data {
	struct AlternateCurrency;
}

class AlternateCurrencyManager : public Singleton<AlternateCurrencyManager>{
private:
	friend class Singleton<AlternateCurrencyManager>;
	AlternateCurrencyManager() {};
	~AlternateCurrencyManager() {};
	AlternateCurrencyManager(AlternateCurrencyManager const&); // Do not implement.
	void operator=(AlternateCurrencyManager const&); // Do not implement.
public:

	// Initialises the AlternateCurrencyManager.
	const bool initialise();

	// Returns the list of alternate currencies.
	inline std::list<Data::AlternateCurrency*>& getCurrencies() { return mCurrencies; }

	// Returns the Item ID associated with a currency ID. Returns zero if not found.
	const u32 getItemID(const u32 pCurrencyID) const;
	
	// Returns the currency ID associated with an Item ID. Returns zero if not found.
	const u32 getCurrencyID(const u32 pItemID) const;

private:

	bool mInitialised = false;
	std::list<Data::AlternateCurrency*> mCurrencies;
};