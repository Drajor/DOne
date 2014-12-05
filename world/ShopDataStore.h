#pragma once

#include "Singleton.h"
#include "Types.h"
#include <list>

namespace Data {
	struct Shop;
}

class ShopDataStore : public Singleton<ShopDataStore> {
public:
	const bool initialise();
	Data::Shop* getShopData(const u32 pID) const;

	// Returns the list of shop data.
	inline std::list<Data::Shop*>& getShopData() { return mShopData; }
private:
	bool mInitialised = false;
	std::list<Data::Shop*> mShopData;
private:
	friend class Singleton<ShopDataStore>;
	ShopDataStore() {};
	~ShopDataStore() {};
	ShopDataStore(ShopDataStore const&); // Do not implement.
	void operator=(ShopDataStore const&); // Do not implement.
};