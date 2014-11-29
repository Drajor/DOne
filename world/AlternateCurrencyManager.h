#pragma once

#include "Constants.h"
#include "Singleton.h"

struct AlternateCurrency;

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
	inline std::list<AlternateCurrency*>& getCurrencies() { return mCurrencies; }

	// Returns the Item ID associated with a currency ID. Returns zero if not found.
	const uint32 getItemID(const uint32 pCurrencyID) const;
	
	// Returns the currency ID associated with an Item ID. Returns zero if not found.
	const uint32 getCurrencyID(const uint32 pItemID) const;

private:

	bool mInitialised = false;
	std::list<AlternateCurrency*> mCurrencies;
};