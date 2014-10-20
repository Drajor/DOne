#pragma once

class Actor;
class Character;
class NPC;

class CombatSystem {
public:
	// Character vs NPC
	static void primaryMeleeAttack(Character* pAttacker, NPC* pDefender);
	static void seconaryMeleeAttack(Character* pAttacker, NPC* pDefender);
	static void rangedAttack(Character* pAttacker, NPC* pDefender);

	// Character vs Character
	static void primaryMeleeAttack(Character* pAttacker, Character* pDefender);
	static void seconaryMeleeAttack(Character* pAttacker, Character* pDefender);
	static void rangedAttack(Character* pAttacker, Character* pDefender);

	// NPC vs Character
	static void primaryMeleeAttack(NPC* pAttacker, Character* pDefender);
	static void seconaryMeleeAttack(NPC* pAttacker, Character* pDefender);
	static void rangedAttack(NPC* pAttacker, Character* pDefender);
private:
};