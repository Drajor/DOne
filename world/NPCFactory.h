#pragma once

#include "Types.h"
#include <list>

namespace Data {
	struct NPCAppearance;
	struct NPCType;
}

class LogContext;
class DataStore;
class ItemFactory;
class ShopDataStore;
class NPC;
class HateControllerFactory;

class NPCFactory {
public:
	NPCFactory();
	~NPCFactory();
	const bool initialise(DataStore* pDataStore, ItemFactory* pItemFactory, ShopDataStore* pShopDataStore);

	NPC* create(const u32 pTypeID);
	NPC* createInvisibleMan();

	// Finds a NPCType by ID.
	Data::NPCType* findType(const u32 pID) const;
	inline std::list<Data::NPCType*> getNPCTypes() { return mNPCTypes; }

	// Finds an NPCAppearance by ID.
	Data::NPCAppearance* findAppearance(const u32 pID) const;
	inline std::list<Data::NPCAppearance*> getNPCAppearances() { return mNPCAppearances; }

private:

	bool mInitialised = false;
	LogContext* mLog = nullptr;
	DataStore* mDataStore = nullptr;
	ItemFactory* mItemFactory = nullptr;
	ShopDataStore* mShopDataStore = nullptr;
	HateControllerFactory* mHateControllerFactory = nullptr;

	// NPC Appearances
	const bool calculateAppearanceData();
	const bool _resolveAppearanceData(Data::NPCAppearance* pAppearance);
	
	std::list<Data::NPCAppearance*> mNPCAppearances;

	// NPC Types
	const bool validateNPCTypeData();
	std::list<Data::NPCType*> mNPCTypes;

	const bool initialiseMerchant(NPC* pNPC, Data::NPCType* pType);
};