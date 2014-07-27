#pragma once

#include "../common/types.h"
#include <string>

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

enum Anonymous{
	ANON_None = 0,
	ANON_Roleplay = 1,
	ANON_Anonymous = 2
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
	CH_GMSAY = 11, // ??
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

enum Animation : uint8 {
	ANIM_KICK = 1, // tested
	ANIM_PIERCING = 2,	// tested
	ANIM_2HSLASHING = 3, // tested
	ANIM_2HWEAPON = 4, // Same animation as 2 but plays a different sound
	ANIM_1HWEAPON = 5, // tested RHS sword swing
	ANIM_DUALWIELD = 6, // tested LSH sword swing
	ANIM_TAILRAKE = 7,	//tested slam & Dpunch too
	ANIM_HAND2HAND = 8, // tested (punch)
	ANIM_SHOOTBOW = 9, // tested
	ANIM_SWIMMING0 = 10, // tested
	ANIM_ROUNDKICK = 11, // tested
	ANIM_TWITCH = 12, // tested
	ANIM_HIT = 13, // tested
	ANIM_TRIP = 14, // tested
	ANIM_DROWNING = 15, // tested
	ANIM_DEATHX = 16, // tested
	ANIM_17 = 17, // Nothing (Tested with Human Female)
	ANIM_18 = 18, // Nothing (Tested with Human Female)
	ANIM_JUMP_HORIZONTAL = 19, // tested
	ANIM_JUMP_VERTICAL = 20, // tested Was previously ANIM_SWARMATTACK
	ANIM_FALLING = 21, // tested
	ANIM_CROUCH_MOVING = 22, // tested
	ANIM_CLIMB = 23, // tested
	ANIM_DUCK = 24, // tested
	ANIM_SWIMMING_IDLE = 25, // tested
	ANIM_IDLE_LOOK = 26, // tested
	ANIM_HAPPY = 27, // tested /cheer /happy
	ANIM_SAD = 28, // tested /cry /frown /mourn
	ANIM_WAVE = 29, // tested /brb /wave
	ANIM_RUDE = 30, // tested /rude /finger /bird /flipoff
	ANIM_YAWN = 31, // tested /bored /yawn
	ANIM_32 = 32, // Nothing (Tested with: Human Female)
	ANIM_SITX = 33, // tested
	ANIM_SHUFFLE0 = 34, // tested The character shuffles their feet a little bit.
	ANIM_SHUFFLE1 = 35, // As above.
	ANIM_BEND_KNEE = 36, // Really looks like a kneel but /kneel uses a different animation.
	ANIM_SWIMMING1 = 37, // tested Same as 10 (ANIM_SWIMMING0)
	ANIM_SIT_IDLE0 = 38, // tested
	ANIM_PLAY_DRUM = 39, // tested
	ANIM_PLAY_FLUTE = 40, // tested
	ANIM_STIR_POT = 41, // tested It really looks like character is stirring a large pot with 2 hands.
	ANIM_CASTING0 = 42, // tested
	ANIM_CASTING1 = 43, // tested
	ANIM_CASTING2 = 44, // tested
	ANIM_FLYING_KICK = 45, // tested (Monk)
	ANIM_TIGER_CLAW = 46, // tested (Monk)
	ANIM_EAGLE_STRIKE = 47, // tested (Monk)
	ANIM_NOD = 48, // tested /nod /agree
	ANIM_GASP = 49, // tested /boggle /gasp
	ANIM_PLEAD = 50, // tested /grovel /plead /apologize
	ANIM_CLAP = 51, // tested /applaud /clap
	ANIM_BLEED = 52, // tested /bleed /hungry
	ANIM_BLUSH = 53, // tested /blush
	ANIM_CHUCKLE = 54, // tested /cackle /chuckle /giggle /snicker
	ANIM_BURP = 55, // tested /burp /cough
	ANIM_CRINGE = 56, // tested /cringe /duck
	ANIM_CURIOUS = 57, // tested /curious /stare /puzzle
	ANIM_DANCE = 58, // tested /dance
	ANIM_BLINK = 59, // tested /blink /veto
	ANIM_GLARE = 60, // tested /glare
	ANIM_PEER = 61, // tested /drool /peer /whistle
	ANIM_KNEEL = 62, // tested /kneel
	ANIM_UNK3 = 63, // tested Character appears disappointed.. maybe there is an emote for it.
	ANIM_POINT = 64, // tested /point
	ANIM_SHRUG = 65, // tested /ponder /shrug
	ANIM_RAISE = 66, // tested /raise /ready
	ANIM_SALUTE = 67, // tested /salute
	ANIM_SHIVER = 68, // tested /shiver
	ANIM_TAP = 69, // tested /tap
	ANIM_BOW = 70, // tested /bow /thank
	ANIM_71 = 71, // Nothing (Tested with Human Female)
	ANIM_72 = 72, // tested Character puts arms out and looks around.
	ANIM_SIT_IDLE1 = 73, // tested Character moves head.
	ANIM_RIDING_IDLE = 74, // tested
	ANIM_75, // Nothing (Tested with Human Female)
	ANIM_76, // Nothing (Tested with Human Female)
	ANIM_SMILE = 77, // tested
	ANIM_RIDING = 78, // tested
	ANIM_79, // Nothing (Tested with Human Female)
	ANIM_CRACK_WHIP0, // tested Will be related to mounts/riding I expect
	ANIM_CRACK_WHIP1 // As Above.
	// Animations appear to loop back to 1 around here.
};

/*
http://everquest.allakhazam.com/history/patches-1999.html
New emotes:
- Here is a list of new emotes that have been added to the game: agree,
amaze, apologize, applaud, plead, bite, bleed, blink, blush, boggle,
bonk, bored, brb, burp, bye, cackle, calm, clap, comfort, congratulate,
cough, cringe, curious, dance, drool, duck, eye, gasp, giggle, glare,
grin, groan, grovel, happy, hungry, introduce, jk (just kidding),
kneel, lost, massage, moan, mourn, peer, point, ponder, puzzle, raise,
ready, roar, salute, shiver, shrug, sigh, smirk, snarl, snicker, stare,
tap, tease, thank, thirsty, veto, welcome, whine, whistle, yawn.
*/

enum WhoType {
	WHO_ZONE,
	WHO_WORLD
};
struct WhoFilter {
	WhoFilter() : mType(WHO_ZONE), mName(""), mRace(0), mClass(0), mMinLevel(0), mMaxLevel(0) {};
	WhoType mType;
	std::string mName;
	uint32 mRace;
	uint8 mClass;
	uint8 mMinLevel;
	uint8 mMaxLevel;
};
//struct Who_All_Struct { // 76 length total
//	/*000*/	char	whom[64];
//	/*064*/	uint32	wrace;		// FF FF = no race
//
//	/*068*/	uint32	wclass;		// FF FF = no class
//	/*072*/	uint32	lvllow;		// FF FF = no numbers
//	/*076*/	uint32	lvlhigh;	// FF FF = no numbers
//	/*080*/	uint32	gmlookup;	// FF FF = not doing /who all gm
//	/*084*/	uint32	guildid;
//	/*088*/	uint8	unknown076[64];
//	/*152*/	uint32	type;		// New for SoF. 0 = /who 3 = /who