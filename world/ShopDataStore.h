#pragma once

#include "Types.h"
#include <list>

namespace Data {
	struct Shop;
}

class ILog;
class ILogFactory;
class IDataStore;

class ShopDataStore {
public:

	~ShopDataStore();

	// Initialises the ShopDataStore.
	const bool initialise(IDataStore* pDataStore, ILogFactory* pLogFactory);

	Data::Shop* getShopData(const u32 pID) const;

	// Returns the list of shop data.
	inline std::list<Data::Shop*>& getShopData() { return mData; }

private:

	bool mInitialised = false;
	ILog* mLog = nullptr;
	IDataStore* mDataStore = nullptr;
	
	std::list<Data::Shop*> mData;
};