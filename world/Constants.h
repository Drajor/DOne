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

enum SpawnAppearanceType : int16 {
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
	NPCName = 31, // 1 = Trader <PlayerName>
	ShowHelm = 43,
	DamageState = 44
};

enum SpawnAppearanceAnimation : int16 {
	Standing = 100,
	Freeze = 102,
	Looting = 105,
	Sitting = 110,
	Crouch = 111,
	Death = 115
};

enum BodyType {
	BT_Humanoid = 1,
	BT_Lycanthrope = 2,
	BT_Undead = 3,
	BT_Giant = 4,
	BT_Construct = 5,
	BT_Extraplanar = 6,
	BT_Magical = 7,	//this name might be a bit off,
	BT_SummonedUndead = 8,
	BT_RaidGiant = 9,
	//					...
	BT_NoTarget = 11,	//no name, can't target this bodytype
	BT_Vampire = 12,
	BT_Atenha_Ra = 13,
	BT_Greater_Akheva = 14,
	BT_Khati_Sha = 15,
	BT_Seru = 16,	//not confirmed....
	BT_Zek = 19,
	BT_Luggald = 20,
	BT_Animal = 21,
	BT_Insect = 22,
	BT_Monster = 23,
	BT_Summoned = 24,	//Elemental?
	BT_Plant = 25,
	BT_Dragon = 26,
	BT_Summoned2 = 27,
	BT_Summoned3 = 28,
	//29
	BT_VeliousDragon = 30,	//might not be a tight set
	//					...
	BT_Dragon3 = 32,
	BT_Boxes = 33,
	BT_Muramite = 34,	//tribal dudes
	//					...
	BT_NoTarget2 = 60,
	//					...
	BT_SwarmPet = 63,	//is this valid, or made up?
	//					...
	BT_InvisMan = 66,	//no name, seen on 'InvisMan', can be /targeted
	BT_Special = 67
};