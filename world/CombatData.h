#pragma once

#include "Types.h"
#include <map>
#include <list>

class Actor;

/*

Each Actor has a DefenderCombatData which stores information about other Actors that have attacked them.
Each Actor has an AttackerCombatData which stores a list of Actors which that Actor has attacked.

*/
struct AttackerData {
	AttackerData(const u64 pHate, const u64 pDamage) : mHate(pHate), mDamage(pDamage) {};
	u64 mHate = 0;
	u64 mDamage = 0;
};

class DefenderCombatData {
public:

	void add(Actor* pActor, const u32 pHate, const u32 pDamage);
	void remove(Actor* pActor);

	// Returns a copy of the attacker list.
	std::map<Actor*, AttackerData> getAttackers() { return mAttackers; }

	// Clears all attackers.
	inline void clear() { mAttackers.clear(); }
	inline const bool hasAttackers() const { return !mAttackers.empty(); }
private:
	AttackerData* find(Actor* pActor);
	
	std::map<Actor*, AttackerData> mAttackers; // List of attackers.
};

class AttackerCombatData {
public:
	void add(Actor* pActor);
	void remove(Actor* pActor);
	const bool search(Actor* pActor) const;

	// Returns a copy of the defender list.
	inline std::list<Actor*> getDefenders() { return mDefenders; }

	// Clears all defenders.
	inline void clear() { mDefenders.clear(); }
	inline const bool hasDefenders() const { return !mDefenders.empty(); }
private:
	std::list<Actor*> mDefenders; // List of defenders.
};