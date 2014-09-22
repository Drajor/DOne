#pragma once

#include "Actor.h"

class NPC : public Actor {
public:
	NPC();
	inline const bool isNPC() const { return true; }
	const bool isNPCCorpse() const { return getActorType() == AT_NPC_CORPSE; }
	const bool initialise();
	const bool onDeath();

private:
};