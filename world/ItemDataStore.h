#pragma once

#include "Types.h"
#include "ItemData.h"

#include <vector>

class IDataStore;

class ItemDataStore {
public:

	ItemDataStore();
	const bool initialise(IDataStore* pDataStore);
	ItemData* getNew();
	ItemData* get(const u32 pID);
	const u32 getNextSerial();

private:

	bool mInitialised = false;
	IDataStore* mDataStore = nullptr;

	void _bootstrap();
	const bool _TransmutationComponentTest(const u32 pItemID, const String& pName);
	std::vector<ItemData> mItemData;

	inline ItemData* _getNext();
	u32 mNextID = 10000;
	const u32 getNextID();
	u32 mNextSerial = 1;
	
};