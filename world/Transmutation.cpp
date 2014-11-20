#include "Transmutation.h"
#include "DataStore.h"
#include "Utility.h"
#include "Item.h"
#include "ItemFactory.h"
#include "Random.h"

const bool Transmutation::initialise() {
	EXPECTED_BOOL(mInitialised == false);

	Log::status("[Transmutation] Initialising.");
	EXPECTED_BOOL(DataStore::getInstance().loadTransmutationComponents(mComponents));
	Log::info("[Transmutation] Loaded data for " + std::to_string(mComponents.size()) + " Components.");

	mInitialised = true;
	return true;
}

Item* Transmutation::transmute(std::list<Item*> pItems) {
	EXPECTED_PTR(pItems.empty() == false);

	std::list<TransmutationComponent*> components;
	for (auto i : pItems) {
		// Find matching component.
		auto component = _get(i->getID());
		if (!component)
			return nullptr;

		components.push_back(component);
	}
	
	// TODO: lots of checks ;)

	Item* result = ItemFactory::makeAugment();
	for (auto i : components) {
		if (i->mAttribute == "str") {
			uint32 roll = Random::make(i->mMinimum, i->mMaximum);
			result->setMaxPower(result->getMaxPower() + i->mMaximum);
			result->setPower(result->getPower() + roll);
			result->setStrength(roll);
		}
		else if (i->mAttribute == "hp") {
			uint32 roll = Random::make(i->mMinimum, i->mMaximum);
			result->setMaxPower(result->getMaxPower() + i->mMaximum);
			result->setPower(result->getPower() + roll);
			result->setHealth(Random::make(i->mMinimum, i->mMaximum));
		}
	}

	return result;
}

TransmutationComponent* Transmutation::_get(const uint32 pItemID) const {
	for (auto i : mComponents) {
		if (i->mItemID == pItemID) return i;
	}

	return nullptr;
}
