#pragma once

#include "Types.h"
#include <list>

namespace Data {
	struct AlternateCurrency;
}
class ILog;
class ILogFactory;
class IDataStore;

class AlternateCurrencyManager {
public:
	~AlternateCurrencyManager();

	// Initialises the AlternateCurrencyManager.
	const bool initialise(IDataStore* pDataStore, ILogFactory* pLogFactory);

	// Returns the list of alternate currencies.
	inline std::list<Data::AlternateCurrency*>& getCurrencies() { return mData; }

	// Returns the Item ID associated with a currency ID. Returns zero if not found.
	const u32 getItemID(const u32 pCurrencyID) const;
	
	// Returns the currency ID associated with an Item ID. Returns zero if not found.
	const u32 getCurrencyID(const u32 pItemID) const;

private:

	bool mInitialised = false;
	IDataStore* mDataStore = nullptr;
	ILog* mLog = nullptr;

	std::list<Data::AlternateCurrency*> mData;
};