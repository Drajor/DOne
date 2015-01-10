#pragma once

#include "Types.h"
#include <list>

namespace Data {
	struct AlternateCurrency;
}
class LogContext;
class DataStore;

class AlternateCurrencyManager {
public:
	AlternateCurrencyManager();
	~AlternateCurrencyManager();

	// Initialises the AlternateCurrencyManager.
	const bool initialise(DataStore* pDataStore);

	// Returns the list of alternate currencies.
	inline std::list<Data::AlternateCurrency*>& getCurrencies() { return mCurrencies; }

	// Returns the Item ID associated with a currency ID. Returns zero if not found.
	const u32 getItemID(const u32 pCurrencyID) const;
	
	// Returns the currency ID associated with an Item ID. Returns zero if not found.
	const u32 getCurrencyID(const u32 pItemID) const;

private:

	bool mInitialised = false;
	DataStore* mDataStore = nullptr;
	LogContext* mLog = nullptr;

	std::list<Data::AlternateCurrency*> mCurrencies;
};