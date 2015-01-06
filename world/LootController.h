#pragma once

#include "Types.h"
#include <list>

class Character;
class Group;
class Raid;
class HateController;

class LootController {
public:

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

	void set(Character* pCharacter) { mCharacter = pCharacter; }
	void set(Group* pGroup) { mGroup = pGroup; }
	void set(Raid* pRaid) { mRaid = pRaid; }

	void clearCharacter() { mCharacter = nullptr; }
	void clearGroup() { mGroup = nullptr; }
	void clearRaid() { mRaid = nullptr; }

private:

	// Flag indicating whether loot is 'open', i.e. Anyone can loot.
	bool mOpen = false;

	// Character that is currently looting.
	Character* mLooter = nullptr;

	// Specific Character that is allowed to loot.
	Character* mCharacter = nullptr;

	// Specific Group that is allowed to loot.
	Group* mGroup = nullptr;

	// Specific Raid that is allowed to loot.
	Raid* mRaid = nullptr;
};