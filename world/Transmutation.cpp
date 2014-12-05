#include "Transmutation.h"
#include "DataStore.h"
#include "Data.h"
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

	// Process components
	Item* result = ItemFactory::makeAugment();
	for (auto i : components) {
		if (i->mAttribute == "str") { result->setStrength(_roll(result, i)); }
		else if (i->mAttribute == "sta") { result->setStamina(_roll(result, i)); }
		else if (i->mAttribute == "agi") { result->setAgility(_roll(result, i)); }
		else if (i->mAttribute == "dex") { result->setDexterity(_roll(result, i)); }
		else if (i->mAttribute == "wis") { result->setWisdom(_roll(result, i)); }
		else if (i->mAttribute == "int") { result->setIntelligence(_roll(result, i)); }
		else if (i->mAttribute == "cha") { result->setCharisma(_roll(result, i)); }
		else if (i->mAttribute == "pr") { result->setPoisonResist(_roll(result, i)); }
		else if (i->mAttribute == "mr") { result->setMagicResist(_roll(result, i)); }
		else if (i->mAttribute == "dr") { result->setDiseaseResist(_roll(result, i)); }
		else if (i->mAttribute == "fr") { result->setFireResist(_roll(result, i)); }
		else if (i->mAttribute == "cr") { result->setColdResist(_roll(result, i)); }
		else if (i->mAttribute == "cor") { result->setCorruptionResist(_roll(result, i)); }

		else {
			delete result;
			return nullptr;
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

const int32 Transmutation::_roll(Item* pItem, TransmutationComponent* pComponent) {
	int32 roll = Random::make(pComponent->mMinimum, pComponent->mMaximum);
	pItem->setMaxPower(pItem->getMaxPower() + pComponent->mMaximum);
	pItem->setPower(pItem->getPower() + roll);

	return roll;
}
