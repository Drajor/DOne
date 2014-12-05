#pragma once

#include "Types.h"
#include "Singleton.h"
#include <list>

namespace Data {
	struct NPCAppearance;
	struct NPCType;
}
class NPC;

class NPCFactory : public Singleton<NPCFactory> {
private:
	friend class Singleton<NPCFactory>;
	NPCFactory() {};
	~NPCFactory() {};
	NPCFactory(NPCFactory const&); // Do not implement.
	void operator=(NPCFactory const&); // Do not implement.
public:
	const bool initialise();
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
	// NPC Appearances
	const bool calculateAppearanceData();
	const bool _resolveAppearanceData(Data::NPCAppearance* pAppearance);
	
	std::list<Data::NPCAppearance*> mNPCAppearances;

	// NPC Types
	const bool validateNPCTypeData();
	std::list<Data::NPCType*> mNPCTypes;

	const bool initialiseMerchant(NPC* pNPC, Data::NPCType* pType);
};