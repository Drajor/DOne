#pragma once

#include "Singleton.h"
#include <list>

struct ShopData;

class ShopDataStore : public Singleton<ShopDataStore> {
public:
	const bool initialise();
	ShopData* getShopData(const unsigned int pShopID) const;
private:
	bool mInitialised = false;
	std::list<ShopData*> mShopData;
private:
	friend class Singleton<ShopDataStore>;
	ShopDataStore() {};
	~ShopDataStore() {};
	ShopDataStore(ShopDataStore const&); // Do not implement.
	void operator=(ShopDataStore const&); // Do not implement.
};