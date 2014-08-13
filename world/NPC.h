#pragma once

#include "Actor.h"

class NPC : public Actor {
public:
	inline bool isNPC() { return true; }
private:
};