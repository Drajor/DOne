#include "CombatSystem.h"

#include "Utility.h"
#include "Actor.h"
#include "Character.h"
#include "NPC.h"
#include "Zone.h"
#include "HateController.h"

#include "Random.h"

// These are special damage values that the client uses to print combat messages.
static const int32 AttackMiss = 0;
static const int32 Invulnerable = -5;

const bool CombatSystem::preAttack(Actor* pAttacker, Actor* pDefender) {
	EXPECTED_BOOL(pAttacker);
	EXPECTED_BOOL(pDefender);
	auto zone = pAttacker->getZone();
	EXPECTED_BOOL(zone);

	if (pAttacker->isDead() || pDefender->isDead()) return false;

	// Attacker animation.
	zone->handleAnimation(pAttacker, pAttacker->getPrimaryAttackAnimation(), 10, true);

	// Check: Defender is invulnerable
	if (pDefender->isInvulnerable()) {
		// Sends then "X tried to hit Y, but Y is INVULNERABLE!"
		zone->handleDamage(pAttacker, pDefender, Invulnerable, pAttacker->getPrimaryDamageType(), 0);
		return false;
	}

	return true;
}

const void CombatSystem::postAttack(Actor* pAttacker, Actor* pDefender, const i32 pDamage, const bool pCritical) {
	auto zone = pAttacker->getZone();
	EXPECTED(zone);

	// Attack missed.
	if (pDamage == 0) {
		zone->handleDamage(pAttacker, pDefender, AttackMiss, pAttacker->getPrimaryDamageType(), 0);
		return;
	}
	
	const u8 preDamagePct = pDefender->getHPPercent();
	
	// Apply damage.
	pDefender->damage(pDamage);

	// Send critical hit message.
	if (pCritical)
		zone->handleCriticalHit(pAttacker, pDamage);

	// Check: Did that hit kill pDefender?
	if (pDefender->isDead()) {
		// Send kill message - "You have slain X!"
		zone->handleDeath(pDefender, pAttacker, pDamage, pAttacker->getPrimaryDamageType());
	}
	else {
		// Send normal damage message - "You punch X for Y points of damage."
		// This also triggers the 'hit' animation on Defender.
		zone->handleDamage(pAttacker, pDefender, pDamage, pAttacker->getPrimaryDamageType(), 0);

		// Check: Send HP update if % hp changed.
		if (preDamagePct != pDefender->getHPPercent()) {
			zone->handleHPChange(pDefender);
		}
	}
}

void CombatSystem::primaryMeleeAttack(Character* pAttacker, NPC* pDefender) {
	if (!preAttack(pAttacker, pDefender)) return;

	auto zone = pAttacker->getZone();
	EXPECTED(zone);
	auto hateController = pDefender->getHateController();
	EXPECTED(hateController);

	int32 damage = 0;
	uint32 hitChance = 0;

	bool hit = Random::make<uint32>(1, 100) <= hitChance;
	if (!hit) {
		damage = AttackMiss;
	}
	else {
		//damage = Random::make<int32>(50, 70);
		damage = Random::make<int32>(1, 5);
	}

	uint32 criticalChance = 50;
	bool criticalHit = Random::make<uint32>(0, 100) <= criticalChance;
	if (criticalHit) {
		damage *= 2;
	}

	postAttack(pAttacker, pDefender, damage, criticalHit);

	if (pDefender->isAlive()) {
		// Add hate.
		hateController->add(pAttacker, damage);
	}
}

void CombatSystem::seconaryMeleeAttack(Character* pAttacker, NPC* pDefender)
{

}

void CombatSystem::rangedAttack(Character* pAttacker, NPC* pDefender)
{

}

void CombatSystem::primaryMeleeAttack(NPC* pAttacker, Character* pDefender) {
	if (!preAttack(pAttacker, pDefender)) return;

	auto zone = pAttacker->getZone();
	EXPECTED(zone);

	int32 damage = 0;
	uint32 hitChance = 80;

	bool hit = Random::make<uint32>(1, 100) <= hitChance;
	if (!hit) {
		damage = AttackMiss;
	}
	else {
		damage = Random::make<int32>(1, 2);
	}

	uint32 criticalChance = 50;
	bool criticalHit = Random::make<uint32>(0, 100) <= criticalChance;
	if (criticalHit) {
		damage *= 2;
	}

	postAttack(pAttacker, pDefender, damage, criticalHit);
}

void CombatSystem::primaryMeleeAttack(Character* pAttacker, Character* pDefender)
{

}

void CombatSystem::primaryMeleeAttack(NPC* pAttacker, NPC* pDefender)
{

}

void CombatSystem::seconaryMeleeAttack(NPC* pAttacker, Character* pDefender)
{

}

void CombatSystem::seconaryMeleeAttack(Character* pAttacker, Character* pDefender)
{

}

void CombatSystem::seconaryMeleeAttack(NPC* pAttacker, NPC* pDefender)
{

}

void CombatSystem::rangedAttack(NPC* pAttacker, Character* pDefender)
{

}

void CombatSystem::rangedAttack(Character* pAttacker, Character* pDefender)
{

}

void CombatSystem::rangedAttack(NPC* pAttacker, NPC* pDefender)
{

}

