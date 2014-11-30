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
private:
	bool mInitialised = false;
	// NPC Appearances
	const bool calculateAppearanceData();
	const bool _resolveAppearanceData(NPCAppearanceData* pAppearance);
	NPCAppearanceData* _findAppearance(const uint32 pID);
	std::list<NPCAppearanceData*> mNPCAppearanceData;

	// NPC Types
	const bool validateNPCTypeData();
	NPCTypeData* _findType(const uint32 pID);
	std::list<NPCTypeData*> mNPCTypeData;

	const bool initialiseMerchant(NPC* pNPC, NPCTypeData* pTypeData);
};