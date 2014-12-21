#pragma once

#include "Types.h"
#include <list>

class Character;
class HateController;

class LootController {
public:

	void configure(HateController* pHateController);

	// Adds a looter to the 'allowed' list.
	void addLooter(Character* pLooter);

	// Removes a looter from the 'allowed' list.
	void removeLooter(Character* pLooter);

	// Clears the 'allowed' list.
	void clear();

	// Returns whether a Character is on the 'allowed' list.
	const bool isLooter(Character* pCharacter) const;
	
	// Returns whether loot is open.
	const bool isOpen() const { return mOpen; }

	// Sets whether loot is open.
	void setOpen(const bool pValue) { mOpen = pValue; }

	// Returns whether or not there is currently a looter.
	inline const bool hasLooter() const { return mLooter != nullptr; }

	// Returns the current looter.
	inline Character* getLooter() const { return mLooter; }

	// Sets the current looter.
	inline void setLooter(Character* pCharacter) { mLooter = pCharacter; }

	// Clears the current looter.
	inline void clearLooter() { mLooter = nullptr; }

	// Returns whether a Character is allowed to loot.
	const bool canLoot(Character* pLooter) const;
private:
	// Flag indicating whether loot is 'open', i.e. Anyone can loot.
	bool mOpen = false;
	// TODO: Locked?

	// List of Characters that are allowed to loot.
	std::list<Character*> mLooters;

	// Character currently looting.
	Character* mLooter = nullptr;
};