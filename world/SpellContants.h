#pragma once

#include "Types.h"

static const auto MaxBuffs = 30;
static const auto MaxSpellID = 35000;
//static const auto NumClasses = 16;
//static const auto NumEffects = 12;
//static const auto NumComponents = 4;
//static const auto NumSpellFields = 215;
//typedef std::array<String, NumFields> Spell;

namespace ResistType {
	enum : u32 {
		None = 0,
	};
}

namespace EnvironmentConstraint {
	enum : i32 {
		None = -1,

	};
}

namespace ZoneContraint {
	enum : u32 {

	};
}

namespace TargetType {
	enum : u32 {
		Foe = 1,
		SingleTarget,
		Self = 6,
		TargetedAE = 8,
		Group = 41,
	};
}

namespace EffectFormula {
	enum : u32 {

	};
}

namespace EffectType {
	enum : u32 {
		Health = 0,
		ArmorClass = 1,
		Attack = 2,
		MovementSpeed = 3,
		Strength = 4,
		Dexterity = 5,
		Agility = 6,
		Stamina = 7,
		Intelligence = 8,
		Wisdom = 9,
		Charisma = 10,
		AttackSpeed = 11,
		Invisibility = 12,
		SeeInvisible = 13,
		Breathing = 14,
		Mana = 15,

		Endurance = 24,

		FireResist = 46,
		ColdResist = 47,
		PoisonResist = 48,
		DiseaseResist = 49,
		MagicResist = 50,

	};
}