#pragma once

#include "Constants.h"
#include "Singleton.h"
#include "ItemData.h"

class StaticItemData : public Singleton<StaticItemData> {
private:
	friend class Singleton<StaticItemData>;
	StaticItemData();
	~StaticItemData();
	StaticItemData(StaticItemData const&); // Do not implement.
	void operator=(StaticItemData const&); // Do not implement.
public:
	const bool initialise();
private:
	bool mInitialised = false;
	ItemData* mItemData = nullptr;
};