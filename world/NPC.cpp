#include "NPC.h"
#include "Character.h"
#include "Zone.h"
#include "Utility.h"
#include "ItemGenerator.h"
#include "Item.h"
#include "ItemFactory.h"

NPC::NPC() {
	setRunSpeed(0.7f);
	setWalkSpeed(0.35f);
	setBodyType(BT_Humanoid);
	setActorType(AT_NPC);
	//setIsNPC(true);

	setCurrency(4, 5, 6, 7);
}

const bool NPC::initialise() {
	EXPECTED_BOOL(mZone);
	EXPECTED_BOOL(getSpawnID() == 0);
	setSpawnID(mZone->getNextSpawnID());

	//String baseName = "Bob";
	//setName(baseName+std::to_string(getSpawnID()));
	//setRaceID(6);
	//setClass(ClassIDs::Cleric);
	setClass(41);

	auto a = ItemGenerator::makeTwoHandBlunt(1, Rarity::Common);
	//auto a = ItemFactory::make(ItemID::StartFood);
	a->setShopQuantity(-1);
	a->setShopPrice(1234);
	addShopItem(a);

	auto b = ItemFactory::make(ItemID::StartDrink);
	b->setShopQuantity(-1);
	b->setShopPrice(2345);
	addShopItem(b);

	auto c = ItemFactory::make(ItemID::StartContainer);
	c->setShopQuantity(-1);
	c->setShopPrice(1);
	addShopItem(c);

	return true;
}

const bool NPC::onDeath() {
	setHPPercent(0);
	setActorType(AT_NPC_CORPSE);
	setName(getName() + "'s corpse");
	mDecayTimer.setStep(DEFAULT_CORPSE_ROT_TIME * 1000);
	mDecayTimer.start();
	return true;
}

const bool NPC::update() {
	// Check: Is NPC flagged for destruction?
	if (getDestroy()) return false;

	// Check: Is NPC dead?
	if (isCorpse()) {
		// Corpse is about to decay, clean up if someone is looting.
		if (mDecayTimer.check()) {
			return false;
		}
		return true;
	}
	return true;
}

void NPC::onDestroy() {
	// Remove target references.
	clearTargeters();

	// Remove looting reference.
	Character* looter = getLooter();
	if (looter) {
		looter->setLootingCorpse(nullptr);
		setLooter(nullptr);
	}

	// TODO: Clean up any Character shopping
	if (isMerchant()) {

	}
}

void NPC::onLootBegin() {
	// Remove any null items from mLootItems
	// These occur from previous looting.
	mLootItems.erase(std::remove(begin(mLootItems), end(mLootItems), nullptr), end(mLootItems));
}

const bool NPC::isShopOpen() const {
	EXPECTED_BOOL(isMerchant());

	// TODO: Fighting or other conditions which would affect this.

	return true;
}

const bool NPC::addShopper(Character* pCharacter) {
	// Ensure Character is not already a shopper.
	auto i = std::find(mShoppers.begin(), mShoppers.end(), pCharacter);
	EXPECTED_BOOL(i == mShoppers.end());

	mShoppers.push_back(pCharacter);
	return true;
}

const bool NPC::removeShopper(Character* pCharacter) {
	// Ensure Character is a shopper.
	auto i = std::find(mShoppers.begin(), mShoppers.end(), pCharacter);
	EXPECTED_BOOL(i != mShoppers.end());

	mShoppers.remove(pCharacter);
	return true;
}

Item* NPC::getShopItem(const uint32 pInstanceID) {
	for (auto i : mShopItems) {
		if (i->getInstanceID() == pInstanceID) return i;
	}

	return nullptr;
}
