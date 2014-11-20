#include "Transmutation.h"
#include "Data.h"
#include "Utility.h"
#include "Item.h"
#include "ItemFactory.h"
#include "Random.h"

const bool Transmutation::initialise() {
	_bootstrap();
	return true;
}

void Transmutation::_bootstrap() {
	auto test0 = new TransmutationComponent();
	mComponents.push_back(test0);

	test0->mItemID = ItemID::TestComponentZero;
	test0->mMinimum = 1;
	test0->mMaximum = 5;
	test0->mAttribute = "str";

	auto test1 = new TransmutationComponent();
	mComponents.push_back(test1);

	test1->mItemID = ItemID::TestComponentOne;
	test1->mMinimum = 25;
	test1->mMaximum = 125;
	test1->mAttribute = "hp";
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
