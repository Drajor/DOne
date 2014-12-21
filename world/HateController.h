#pragma once

#include "Types.h"
#include <list>

class Actor;

/*

- A HateController is an object that captures information about combat such as damage and spells.
- Derived HateControllers use this information to select a target which the NPC should attack.

*/

struct Hate {
	Hate(Actor* pActor, const u32 pHate) : mActor(pActor), mHate(pHate) {};
	Actor* mActor = nullptr;
	u32 mHate = 0;
};

class HateController {
public:
	virtual ~HateController() {};
	inline void setOwner(Actor* pOwner) { mOwner = pOwner; }
	virtual void add(Actor* pAttacker, const u32 pHate);
	virtual void remove(Actor* pAttacker);
	virtual Actor* select() const = 0;
	inline const std::list<Actor*>::size_type getNumAttackers() { return mAttackers.size(); }
	inline const bool hasAttackers() const { return !mAttackers.empty(); }
	inline void clear(){ mAttackers.clear(); }

	// Returns a copy.
	inline std::list<Hate> getAttackers() { return mAttackers; }
protected:
	Actor* mOwner = nullptr;
	std::list<Hate> mAttackers; // List of attackers in order they were added.
private:
};

/*

- Will never select

*/
class NullHateController : public HateController {
public:
	Actor* select() const { return nullptr; };
private:
};

/*

- Will select the closest available attacker.

*/
class ProximityHateController : public HateController {
public:
	Actor* select() const;
private:
};

/*

- Will select the first available attacker.

*/
class FirstHateController : public HateController {
public:
	Actor* select() const;
private:
};

/*

- Will select the last available attacker.

*/
class LastHateController : public HateController {
public:
	Actor* select() const;
private:
};