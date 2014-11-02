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
	ItemData* getNew();
	ItemData* get(const uint32 pID);
private:
	std::vector<ItemData> mItemData;

	inline ItemData* _getNext();
	uint32 mNextID = 10000;
	const uint32 getNextID();
	uint32 mNextSerial = 1;
	const uint32 getNextSerial();
};