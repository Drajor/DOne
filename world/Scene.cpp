#include "Scene.h"
#include "Utility.h"
#include "Zone.h"
#include "Actor.h"
#include "Character.h"
#include "NPC.h"

template <typename C>
inline void _diff(C& pA, C& pB, C& pResult) {
	for (auto i : pA) {
		auto found = false;
		for (auto j : pB) {
			if (j == i) {
				found = true;
				break;
			}
		}

		if (!found) pResult.push_back(i);
	}
}

template <typename C>
inline void diff(C& pNew, C& pOld, C& pAdded, C& pRemoved) {
	// Diff new to old to determine what was added.
	_diff(pNew, pOld, pAdded);
	// Diff old to new to determine what was removed.
	_diff(pOld, pNew, pRemoved);
}

Scene::Scene(Zone* pZone) : mZone(pZone) {
	EXPECTED(mZone);
}

const bool Scene::add(Actor* pActor) {
	EXPECTED_BOOL(pActor);
	
	// Character being added to Scene.
	if (pActor->isCharacter()){
		Character* character = Actor::cast<Character*>(pActor);
		mCharacters.push_back(character);
		onCharacterAdded(character);
		return true;
	}

	// NPC being added to Scene.
	if (pActor->isNPC()) {
		NPC* npc = Actor::cast<NPC*>(pActor);
		mNPCs.push_back(npc);
		onNPCAdded(npc);
		return true;
	}

	// Unknown Actor type.
	return false;
}

const bool Scene::remove(Actor* pActor) {
	EXPECTED_BOOL(pActor);

	// Character being removed from Scene.
	if (pActor->isCharacter()) {
		Character* character = Actor::cast<Character*>(pActor);
		mCharacters.remove(character);
		onCharacterRemoved(character);
		return true;
	}

	// NPC being removed from Scene.
	if (pActor->isNPC()) {
		mNPCs.remove(Actor::cast<NPC*>(pActor));
		return true;
	}

	// Unknown Actor type.
	return false;
}

void Scene::onCharacterAdded(Character* pCharacter) {
	EXPECTED(pCharacter);
	std::list<Character*> visible;
	queryCharacters(pCharacter, visible);

	for (auto i : visible) {
		i->addVisibleTo(pCharacter);
		pCharacter->addVisibleTo(i);

		// Notify Zone that these Characters can see each other.
		mZone->handleVisibilityAdd(pCharacter, i);
		mZone->handleVisibilityAdd(i, pCharacter);
	}
}

void Scene::onCharacterRemoved(Character* pCharacter) {
	EXPECTED(pCharacter);
}

void Scene::onNPCAdded(NPC* pNPC) {
	// Query for Characters near npc

	// NOTE: Brute force for now, the fancy stuff comes later.
	for (auto i : mCharacters) {
		const float squareVisibility = i->getVisibleRange() * i->getVisibleRange();
		if (i->squareDistanceTo(pNPC) <= squareVisibility){
			pNPC->addVisibleTo(i); // NPC pNPC is now visible to Character i
			i->addVisibleNPC(pNPC);
			mZone->handleVisibilityAdd(i, pNPC);
		}
	}
}

void Scene::onNPCRemoved(NPC* pNPC)
{

}


void Scene::queryCharacters(Character* pCharacter, std::list<Character*>& pCharacters) {
	EXPECTED(pCharacter);
	EXPECTED(pCharacters.empty());

	const float squareVisibility = pCharacter->getVisibleRange() * pCharacter->getVisibleRange();

	// NOTE: Brute force for now, the fancy stuff comes later.
	for (auto i : mCharacters) {
		if (i->squareDistanceTo(pCharacter) <= squareVisibility && i != pCharacter){
			pCharacters.push_back(i);
		}
	}
}

void Scene::queryNPCs(Character* pCharacter, std::list<NPC*>& pNPCs) {
	EXPECTED(pCharacter);
	EXPECTED(pNPCs.empty());

	const float squareVisibility = pCharacter->getVisibleRange() * pCharacter->getVisibleRange();

	// NOTE: Brute force for now, the fancy stuff comes later.
	for (auto i : mNPCs) {
		if (i->squareDistanceTo(pCharacter) <= squareVisibility){
			pNPCs.push_back(i);
		}
	}
}

const bool Scene::update(Actor* pActor) {
	// Character
	if (pActor->isCharacter()) {
		_updateCharacter(Actor::cast<Character*>(pActor));
		return true;
	}

	// NPC
	if (pActor->isNPC()) {
		_updateNPC(Actor::cast<NPC*>(pActor));
		return true;
	}

	// Unknown Actor type.
	return false;
}

void Scene::_updateCharacter(Character* pCharacter) {
	// Characters.
	std::list<Character*> visibleNew;
	queryCharacters(pCharacter, visibleNew);
	std::list<Character*> added;
	std::list<Character*> removed;

	diff(visibleNew, pCharacter->getVisibleTo(), added, removed);

	for (auto i : added) {
		i->addVisibleTo(pCharacter);
		pCharacter->addVisibleTo(i);

		// Notify Zone that these Characters can see each other.
		mZone->handleVisibilityAdd(pCharacter, i);
		mZone->handleVisibilityAdd(i, pCharacter);
	}

	for (auto i : removed) {
		i->removeVisibleTo(pCharacter);
		pCharacter->removeVisibleTo(i);

		// Notify Zone that these Characters no longer see each other.
		mZone->handleVisibilityRemove(pCharacter, i);
		mZone->handleVisibilityRemove(i, pCharacter);
	}

	// NPCs.
	std::list<NPC*> visibleNewNPC;
	queryNPCs(pCharacter, visibleNewNPC);
	std::list<NPC*> addedNPC;
	std::list<NPC*> removedNPC;

	diff(visibleNewNPC, pCharacter->getVisibleNPCs(), addedNPC, removedNPC);

	for (auto i : addedNPC) {
		i->addVisibleTo(pCharacter);
		pCharacter->addVisibleNPC(i);

		mZone->handleVisibilityAdd(pCharacter, i);

	}
	for (auto i : removedNPC) {
		i->removeVisibleTo(pCharacter);
		pCharacter->removeVisibleNPC(i);

		mZone->handleVisibilityRemove(pCharacter, i);
	}
}

void Scene::_updateNPC(NPC* pNPC)
{

}
