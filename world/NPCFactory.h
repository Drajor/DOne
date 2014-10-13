#pragma once

#include "Constants.h"
#include "Singleton.h"

struct NPCAppearanceData;

class NPCFactory : public Singleton<NPCFactory> {
private:
	friend class Singleton<NPCFactory>;
	NPCFactory() {};
	~NPCFactory() {};
	NPCFactory(NPCFactory const&); // Do not implement.
	void operator=(NPCFactory const&); // Do not implement.
public:
	const bool initialise();
private:
	const bool calculateAppearanceData();
	const bool _resolveAppearanceData(NPCAppearanceData* pAppearance);
	NPCAppearanceData* _findAppearance(const uint32 pID);
	std::list<NPCAppearanceData*> mNPCAppearanceData;
};