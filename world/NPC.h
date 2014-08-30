#pragma once

#include "Actor.h"

class NPC : public Actor {
public:
	NPC();
	inline const bool isNPC() const { return true; }
	const bool initialise();

private:
};