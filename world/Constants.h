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

// [Client Limitation][UF] These values are set by the client.
enum MessageColour {
	MC_White = 0,
	MC_Grey = 1,
	MC_DarkGreen = 2,
	MC_LightBlue = 4,
	MC_Pink = 5,
	MC_Red = 13,
	MC_LightGreen = 14,
	MC_Yellow = 15,
	MC_DarkBlue = 16,
	MC_Aqua = 18
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

enum ChannelID : uint32 {
	CH_GUILD = 0, // /gu
	CH_GROUP = 2, // /g
	CH_SHOUT = 3, // /shou
	CH_AUCTION = 4, // /auc
	CH_OOC = 5, // /ooc
	CH_BROADCAST = 6, // ??
	CH_TELL = 7, // /t
	CH_SAY = 8, // /say
	CH_RAID = 15, // /rs
	CH_UCS = 20, // Not sure yet.
	CH_EMOTE = 22 // UF+

};

enum Language : uint32 {
	COMMON_TONGUE_LANG = 0,
	BARBARIAN_LANG = 1,
	ERUDIAN_LANG = 2,
	ELVISH_LANG = 3,
	DARK_ELVISH_LANG = 4,
	DWARVISH_LANG = 5,
	TROLL_LANG = 6,
	OGRE_LANG = 7,
	GNOMISH_LANG = 8,
	HALFLING_LANG = 9,
	THIEVES_CANT_LANG = 10,
	OLD_ERUDIAN_LANG = 11,
	ELDER_ELVISH_LANG = 12,
	FROGLOK_LANG = 13,
	GOBLIN_LANG = 14,
	GNOLL_LANG = 15,
	COMBINE_TONGUE_LANG = 16,
	ELDER_TEIRDAL_LANG = 17,
	LIZARDMAN_LANG = 18,
	ORCISH_LANG = 19,
	FAERIE_LANG = 20,
	DRAGON_LANG = 21,
	ELDER_DRAGON_LANG = 22,
	DARK_SPEECH_LANG = 23,
	VAH_SHIR_LANG = 24,
	ALARAN_LANG = 25,
	HADAL_LANG = 26,
	UNKNOWN_LANG = 27
};