#include "HateController.h"
#include "Actor.h"

void HateController::add(Actor* pAttacker, const u32 pHate) {
	EXPECTED(pAttacker);

	// Existing attacker.
	for (auto i : mAttackers) {
		if (i.mActor == pAttacker) {
			i.mHate += pHate;
			return;
		}
	}

	// New attacker.
	mAttackers.push_back({ pAttacker, pHate });
	// Add hater reference to attacker.
	pAttacker->addHater(mOwner);
}

void HateController::remove(Actor* pAttacker) {
	EXPECTED(pAttacker);

	for (auto i = mAttackers.begin(); i != mAttackers.end(); i++) {
		if (i->mActor == pAttacker) {
			mAttackers.erase(i);
			break;
		}
	}
}


Actor* ProximityHateController::select() const {
	Actor* selected = nullptr;
	float d = std::numeric_limits<float>::max();
	
	for (auto i : mAttackers) {
		// TODO: Check that i can be attacked.
		auto distanceTo = mOwner->squareDistanceTo(i.mActor);
		if (distanceTo < d) {
			d = distanceTo;
			selected = i.mActor;
		}
	}

	return selected;
}

Actor* FirstHateController::select() const {
	Actor* selected = nullptr;
	for (auto i : mAttackers) {
		// TODO: Check that i can be attacked.
		selected = i.mActor;
		break;
	}

	return selected;
}

Actor* LastHateController::select() const {
	Actor* selected = nullptr;
	for (auto i = mAttackers.rbegin(); i != mAttackers.rend(); i++) {
		// TODO: Check that i can be attacked.
		selected = i->mActor;
		break;
	}

	return selected;
}