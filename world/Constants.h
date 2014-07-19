#pragma once

#include "../common/types.h"

enum PlayableRaceIDs {
	Human = 1,
	Barbarian = 2,
	Erudite = 3,
	WoodElf = 4,
	HighElf = 5,
	DarkElf = 6,
	HalfElf = 7,
	Dwarf = 8,
	Troll = 9,
	Ogre = 10,
	Halfling = 11,
	Gnome = 12,
	Iksar = 128,
	Vahshir = 130,
	Froglok = 330,
	Drakkin = 522
};

enum PlayableClassIDs {
	Warrior = 1,
	Cleric = 2,
	Paladin = 3,
	Ranger = 4,
	Shadowknight = 5,
	Druid = 6,
	Monk = 7,
	Bard = 8,
	Rogue = 9,
	Shaman = 10,
	Necromancer = 11,
	Wizard = 12,
	Magician = 13,
	Enchanter = 14,
	Beastlord = 15,
	Berserker = 16
};

enum PlayerDeityIDs {
	Agnostic = 396
};

enum ZoneIDs {
	NorthQeynos = 2
};

enum SpawnAppearanceTypes : int16 {
	Die = 0,
	WhoLevel = 1,
	Invisible = 3,
	PVP = 4,
	Light = 5,
	Animation = 14,
	Sneak = 15,
	SpawnID = 16,
	HP = 17,
	LinkDead = 18,
	Levitate = 19,
	GM = 20,
	Anonymous = 21,
	GuildID = 22,
	GuildRank = 23,
	AFK = 24,
	Pet = 25,
	Split = 28,
	Size = 29,
	NPCName = 31,
	ShowHelm = 43,
	DamageState = 44
};

enum SpawnAppearanceAnimations : int16 {
	Standing = 100,
	Freeze = 102,
	Looting = 105,
	Sitting = 110,
	Crouch = 111,
	Death = 115
};