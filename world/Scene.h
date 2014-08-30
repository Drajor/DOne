#pragma once

#include "Constants.h"

class Actor;
class Character;
class NPC;
class Zone;

class Scene {
public:
	Scene(Zone* pZone);
	const bool add(Actor* pActor);
	const bool remove(Actor* pActor);
	const bool update(Actor* pActor); // NOTE: Called when an Actor moves

	// Search (whole Scene) for Characters within pRange of pCharacter.
	void queryRange(Character* pCharacter, std::list<Character*>& pResult, const float pRange) {};
	// Search Characters visible to pCharacter within pRange of pCharacter
	void queryRangeVisible(Character* pCharacter, std::list<Character*>& pResult, const float pRange) {};
private:
	void _updateCharacter(Character* pCharacter);
	void _updateNPC(NPC* pNPC);
	void onCharacterAdded(Character* pCharacter);
	void onCharacterRemoved(Character* pCharacter);
	void onNPCAdded(NPC* pNPC);
	void onNPCRemoved(NPC* pNPC);
	void queryCharacters(Character* pCharacter, std::list<Character*>& pCharacters);
	Zone* mZone;
	std::list<Character*> mCharacters;
	std::list<NPC*> mNPCs;
};