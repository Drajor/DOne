#pragma once

#include "Constants.h"
#include "Singleton.h"
#include "ItemData.h"

class ItemDataStore : public Singleton<ItemDataStore> {
private:
	friend class Singleton<ItemDataStore>;
	ItemDataStore();;
	~ItemDataStore() {};
	ItemDataStore(ItemDataStore const&); // Do not implement.
	void operator=(ItemDataStore const&); // Do not implement.
public:
	const bool initialise();
	ItemData* getNew();
	ItemData* get(const uint32 pID);

	const uint32 getNextSerial();
private:
	void _bootstrap();
	bool mInitialised = false;
	std::vector<ItemData> mItemData;

	inline ItemData* _getNext();
	uint32 mNextID = 10000;
	const uint32 getNextID();
	uint32 mNextSerial = 1;
	
};