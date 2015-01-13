#include "Scene.h"
#include "LogSystem.h"
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

Scene::~Scene() {
	if (mLog) {
		delete mLog;
		mLog = nullptr;
	}
}

const bool Scene::initialise(Zone* pZone, ILog* pLog) {
	if (mInitialised) return false;
	if (!pZone) return false;
	if (!pLog) return false;

	mLog = pLog;
	mZone = pZone;

	StringStream ss;
	ss << "[Scene (ID: " << mZone->getID() << " InstanceID: " << mZone->getInstanceID() << ")]";
	mLog->setContext(ss.str());
	

	mLog->status("Finished initialising.");
	mInitialised = true;
	return true;
}

const bool Scene::add(Actor* pActor) {
	if (!pActor) return false;
	
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
	if (!pActor) return false;

	// Character being removed from Scene.
	if (pActor->isCharacter()) {
		Character* character = Actor::cast<Character*>(pActor);
		mCharacters.remove(character);
		onCharacterRemoved(character);
		return true;
	}

	// NPC being removed from Scene.
	if (pActor->isNPC()) {
		NPC* npc = Actor::cast<NPC*>(pActor);
		mNPCs.remove(npc);
		onNPCRemoved(npc);
		return true;
	}

	// Unknown Actor type.
	return false;
}

void Scene::onCharacterAdded(Character* pCharacter) {
	if (!pCharacter) return;
	mLog->info("Adding Character (" + pCharacter->getName() + ")");

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
	if (!pCharacter) return;
	mLog->info("Removing Character (" + pCharacter->getName() + ")");

	// Remove pCharacter from everyone who can see pCharacter.
	for (auto i : pCharacter->getVisibleTo()) {
		mZone->handleVisibilityRemove(i, pCharacter);
		i->removeVisibleTo(pCharacter);
	}

	// Remove all visible NPCs from Character.
	pCharacter->clearVisibleNPCs();
}

void Scene::onNPCAdded(NPC* pNPC) {
	if (!pNPC) return;

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

void Scene::onNPCRemoved(NPC* pNPC) {

	// Update any Character that can see pNPC.
	for (auto i : pNPC->getVisibleTo()) {
		mZone->handleVisibilityRemove(i, pNPC);
		i->removeVisibleNPC(pNPC);
	}
}


void Scene::queryCharacters(Character* pCharacter, std::list<Character*>& pCharacters) {
	if (!pCharacter) return;
	if (pCharacters.empty()) return;

	const float squareVisibility = pCharacter->getVisibleRange() * pCharacter->getVisibleRange();

	// NOTE: Brute force for now, the fancy stuff comes later.
	for (auto i : mCharacters) {
		if (i->squareDistanceTo(pCharacter) <= squareVisibility && i != pCharacter){
			pCharacters.push_back(i);
		}
	}
}

void Scene::queryNPCs(Character* pCharacter, std::list<NPC*>& pNPCs) {
	if (!pCharacter) return;
	if (pNPCs.empty()) return;

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
