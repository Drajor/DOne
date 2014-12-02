#pragma once

#include "Constants.h"
#include "Singleton.h"

struct NPCAppearanceData;
struct NPCTypeData;
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
	NPC* create(const uint32 pTypeID);
	NPC* createInvisibleMan();

	inline std::list<NPCTypeData*> getNPCTypes() { return mNPCTypeData; }

	NPCAppearanceData* getAppearance(const uint32 pID);
	inline std::list<NPCAppearanceData*> getNPCAppearances() { return mNPCAppearanceData; }
private:
	bool mInitialised = false;
	// NPC Appearances
	const bool calculateAppearanceData();
	const bool _resolveAppearanceData(NPCAppearanceData* pAppearance);
	
	std::list<NPCAppearanceData*> mNPCAppearanceData;

	// NPC Types
	const bool validateNPCTypeData();
	NPCTypeData* _findType(const uint32 pID);
	std::list<NPCTypeData*> mNPCTypeData;

	const bool initialiseMerchant(NPC* pNPC, NPCTypeData* pTypeData);
};