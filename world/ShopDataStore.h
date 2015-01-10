#pragma once

#include "Types.h"
#include <list>

namespace Data {
	struct Shop;
}

class IDataStore;

class ShopDataStore {
public:
	const bool initialise(IDataStore* pDataStore);
	Data::Shop* getShopData(const u32 pID) const;

	// Returns the list of shop data.
	inline std::list<Data::Shop*>& getShopData() { return mShopData; }

private:

	bool mInitialised = false;
	IDataStore* mDataStore = nullptr;
	std::list<Data::Shop*> mShopData;
};