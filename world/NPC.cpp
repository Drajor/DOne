#include "NPC.h"
#include "Character.h"
#include "Zone.h"
#include "Utility.h"
#include "ItemGenerator.h"
#include "Item.h"
#include "ItemFactory.h"
#include "LootController.h"
#include "HateController.h"
#include "CombatSystem.h"

NPC::NPC(HateController* pHateController) {
	
	mHateController = pHateController;
	if (!mHateController) {
		Log::error("Got null HateController in NPC constructor!");
	}
	else {
		mHateController->setOwner(this);
	}

	setRunSpeed(0.7f);
	setWalkSpeed(0.35f);
	setBodyType(BT_Humanoid);
	setActorType(AT_NPC);

	setCurrency(4, 5, 6, 7);
}

NPC::~NPC() {
	// Check: NPC has been cleaned up correctly before delete.
	if (mShoppers.empty() == false) { Log::error("[NPC] Shoppers not empty on destruction."); }
	if (mShopItems.empty() == false) { Log::error("[NPC] Shop Items not empty on destruction."); }
	if (mLootItems.empty() == false) { Log::error("[NPC] Loot Items not empty on destruction."); }
	if (mHateController->hasAttackers()) { Log::error("[NPC] HaterController not empty on destruction."); }
}

const bool NPC::initialise() {
	EXPECTED_BOOL(mZone);
	EXPECTED_BOOL(getSpawnID() == 0);
	setSpawnID(mZone->getNextSpawnID());
	addNimbus(405);

	return true;
}

const bool NPC::onDeath() {
	setHPPercent(0);
	setActorType(AT_NPC_CORPSE);
	setName(getName() + "'s corpse");

	mDecayTimer.setStep(DEFAULT_CORPSE_ROT_TIME * 1000);
	mDecayTimer.start();

	mOpenTimer.setStep(DEFAULT_CORPSE_OPEN_TIME * 1000);
	mOpenTimer.start();

	// Configure the LootController.
	auto lootController = getLootController();
	lootController->configure(mHateController);

	clearHate();

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

	// Check: Is NPC in combat?
	//if (hasHaters()) {
	if (mHateController->hasAttackers()) {
		// Check: 
		if (!mAttacking) {
			onCombatBegin();
		}
		if (!hasTarget()) {
			setTarget(mHateController->select());
		}
		if (hasTarget() && mPrimaryAttackTimer.check()) {
			// Attacking an NPC.
			if (getTarget()->isNPC())
				CombatSystem::primaryMeleeAttack(this, Actor::cast<NPC*>(getTarget()));
			// Attacking a Character.
			else if (getTarget()->isCharacter())
				CombatSystem::primaryMeleeAttack(this, Actor::cast<Character*>(getTarget()));
		}
	}

	// Check: Is NPC out of combat?
	if (!mHateController->hasAttackers() && mAttacking) {
		onCombatEnd();
	}

	return true;
}

void NPC::onDestroy() {
	// Remove target references.
	clearTargeters();

	// Check: NPC currently being looted.
	auto lootController = getLootController();
	if (lootController->hasLooter()) {
		auto looter = lootController->getLooter();
		lootController->clearLooter();
		looter->setLootingCorpse(nullptr);
	}

	// TODO: Clean up any Character shopping
	if (isMerchant()) {

	}
	clearHate();
}

void NPC::onLootBegin() {
	// Remove any null items from mLootItems
	// These occur from previous looting.
	mLootItems.erase(std::remove(begin(mLootItems), end(mLootItems), nullptr), end(mLootItems));
}

const bool NPC::isShopOpen() const {
	EXPECTED_BOOL(isMerchant());

	if (getDestroy()) return false; // NPC will be destroyed this update or next update.
	if (hasHaters()) return false; // NPC in combat.

	// TODO: Other conditions which would affect this.

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

void NPC::clearHate() {

	auto attackers = mHateController->getAttackers();
	for (auto i : attackers) {
		i.mActor->removeHater(this);
	}
	mHateController->clear();
	clearHaters();
}

void NPC::onCombatBegin() {
	mAttacking = true;
	mPrimaryAttackTimer.setStep(300);
	mPrimaryAttackTimer.start();
}

void NPC::onCombatEnd() {
	mAttacking = false;
}
