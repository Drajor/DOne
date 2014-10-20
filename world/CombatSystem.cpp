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

	// Check: Defender is already dead. This may have occurred earlier in the update.
	if (pDefender->isDead()) return;

	// Attacker animation.
	zone->handleAnimation(pAttacker, pAttacker->getPrimaryAttackAnimation(), 10, true);

	int32 damage = 0;
	uint32 hitChance = 80;

	// Check: Defender is invulnerable
	if (pDefender->isInvulnerable()) {
		// Sends then "X tried to hit Y, but Y is INVULNERABLE!"
		zone->handleDamage(pAttacker, pDefender, Invulnerable, pAttacker->getPrimaryDamageType(), 0);
		return;
	}

	bool hit = Random::make(1, 100) <= hitChance;
	if (!hit) {
		damage = AttackMiss;
	}
	else {
		damage = Random::make<int32>(5, 15);
	}

	uint32 criticalChance = 50;
	bool criticalHit = Random::make(0, 100) <= criticalChance;
	if (criticalHit) {
		damage *= 2;
	}
	
	// Attacker successfully hit Defender.
	if (damage > 0) {
		// Apply damage.
		const uint8 preDamagePct = pDefender->getHPPercent();
		pDefender->damage(damage);

		// Send critical hit message.
		if (criticalHit && hit)
			zone->handleCriticalHit(pAttacker, damage);

		// Check: Did that hit kill pDefender?
		if (pDefender->isDead()) {
			// Send kill message - "You have slain X!"
			zone->handleDeath(pDefender, pAttacker, damage, pAttacker->getPrimaryDamageType());
		}
		else {
			// Send normal damage message - "You punch X for Y points of damage."
			// This also triggers the 'hit' animation on Defender.
			zone->handleDamage(pAttacker, pDefender, damage, pAttacker->getPrimaryDamageType(), 0);

			// Check: Send HP update if % hp changed.
			if (preDamagePct != pDefender->getHPPercent()) {
				zone->handleHPChange(pDefender);
			}
		}
	}
	// Attacker missed Defender
	else {
		zone->handleDamage(pAttacker, pDefender, AttackMiss, pAttacker->getPrimaryDamageType(), 0);
	}
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

