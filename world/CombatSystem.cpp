#include "CombatSystem.h"
#include "Utility.h"
#include "Actor.h"
#include "Character.h"
#include "NPC.h"
#include "Zone.h"

#include "Random.h"

static const int32 AttackMiss = 0;
static const int32 Invulnerable = -5;

void CombatSystem::primaryMeleeAttack(Character* pAttacker, NPC* pDefender) {
	EXPECTED(pAttacker);
	EXPECTED(pDefender);
	Zone* zone = pAttacker->getZone();
	EXPECTED(zone);

	int32 damage = 0;
	uint32 hitChance = 80;

	// Check: Defender is invulnerable
	if (pDefender->isInvulnerable()) {
		// Attacker animation.
		zone->handleAnimation(pAttacker, 10, pAttacker->getPrimaryAttackAnimation(), true);
		// Sends then "X tried to hit Y, but Y is INVULNERABLE!"
		zone->handleDamage(pAttacker, pDefender, Invulnerable, pAttacker->getPrimaryDamageType(), 0);
		return;
	}

	bool hit = Random::make(1, 100) <= hitChance;
	if (!hit) {
		damage = 0;
	}
	else {
		damage = Random::make<int32>(2, 14);
	}

	uint32 criticalChance = 50;
	bool criticalHit = Random::make(0, 100) <= criticalChance;
	if (criticalHit) {
		damage *= 2;
	}
	
	// Attacker animation.
	zone->handleAnimation(pAttacker, 10, pAttacker->getPrimaryAttackAnimation(), true);
	// Defender animation.

	zone->handleDamage(pAttacker, pDefender, damage, pAttacker->getPrimaryDamageType(), 0);
	// TODO: Determine why defender animation is not working.

	if (criticalHit && hit) {
		zone->handleCriticalHit(pAttacker, damage);
	}

	//pDefender->damage(
}

void CombatSystem::seconaryMeleeAttack(Character* pAttacker, NPC* pDefender)
{

}

void CombatSystem::rangedAttack(Character* pAttacker, NPC* pDefender)
{

}

void CombatSystem::primaryMeleeAttack(NPC* pAttacker, Character* pDefender)
{

}

void CombatSystem::primaryMeleeAttack(Character* pAttacker, Character* pDefender)
{

}

void CombatSystem::seconaryMeleeAttack(NPC* pAttacker, Character* pDefender)
{

}

void CombatSystem::seconaryMeleeAttack(Character* pAttacker, Character* pDefender)
{

}

void CombatSystem::rangedAttack(NPC* pAttacker, Character* pDefender)
{

}

void CombatSystem::rangedAttack(Character* pAttacker, Character* pDefender)
{

}

