#pragma once

#include <stdint.h>
#include <string>
#include <sstream>
#include <list>
#include <vector>
#include <unordered_map>
#include <map>

typedef uint8_t byte;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint16 DeityID;
typedef uint8_t GenderID;
//typedef uint32 ClassID;
typedef uint8_t ClassID;
typedef uint32_t RaceID;
typedef uint32_t ZoneID; // PP=uint16, CharSelect=uint32
typedef uint8_t FaceID; // PP=uint8 CharSelect=uint8
typedef uint16_t InstanceID; // Zone Instance ID
//typedef uint16_t SpawnID;
typedef uint32 SpawnID; // Fucked at the moment, I need to remove this.
typedef std::string String;
typedef std::stringstream StringStream;
typedef uint32_t GuildID;
typedef uint8_t GuildRank;
typedef int8_t AccountStatus;

static const GuildID NO_GUILD = 0xFFFFFFFF;
static const SpawnID NO_TARGET = 0;

enum ResponseID : AccountStatus { ALLOWED = 1, DENIED = 0, SUSPENDED = -1, BANNED = -2, FULL = -3 };

static AccountStatus LOCK_BYPASS_STATUS = 20; // Account status required to bypass login when server is locked.

static const uint32 DEFAULT_RESPAWN_TIME = 5; // In seconds, 5 minutes.

namespace Limits {
	namespace World {
		static const auto Port = 9000; // [Client Limitation] World must use port 9000
	}
	namespace Account {
		static const auto MAX_NUM_CHARACTERS = 18;
		static const auto MAX_EQUIPMENT_SLOTS = 9;
	}
	namespace LoginServer {
		static const auto MAX_ACCOUNT_NAME_LENGTH = 30;
		static const auto MAX_KEY_LENGTH = 30;
	}
	namespace Character {
		static const auto MIN_NAME_LENGTH = 4;
		static const auto MAX_NAME_LENGTH = 64;
		static const auto MAX_TITLE_LENGTH = 32;
		static const auto MAX_SUFFIX_LENGTH = 32;
		static const auto MAX_LAST_NAME_LENGTH = 32;
	}
	namespace Guild {
		static const auto MAX_GUILDS = 1500; // This value is arbitrary.
		static const auto MIN_NAME_LENGTH = 4;
		static const auto MAX_NAME_LENGTH = 64;
		static const auto MAX_MOTD_LENGTH = 512;
		static const auto MAX_PUBLIC_NOTE_LENGTH = 200; // TODO: Determine what the client UI maximum is.
		static const auto MAX_PERSONAL_NOTE_LENGTH = 200; // TODO: Determine what the client UI maximum is.
		static const auto MAX_URL_LENGTH = 512;
		static const auto MAX_CHANNEL_LENGTH = 512;
	}
	namespace Group {
		static const auto MAX_MEMBERS = 6;
	}
	
}

static const String SYS_NAME = "[System]";

enum MaterialSlot : uint8 {
	Mat_Head,
	Mat_Chest,
	Mat_Arms,
	Mat_Wrist,
	Mat_Hands,
	Mat_Legs,
	Mat_Feet,
	Mat_Primary,
	Mat_Secondary
};
static const auto MAX_MATERIAL_SLOTS = 9;
static const auto MAX_ARMOR_DYE_SLOTS = 7;

enum FlyMode : uint8 {
	FM_NONE = 0
};

enum ActorType : uint8 {
	AT_PLAYER,
	AT_NPC,
	AT_PLAYER_CORPSE,
	AT_NPC_CORPSE
};

enum GMStatus : uint8 {
	GM_ON,
	GM_OFF
};

enum AATitle : uint8 {
	NONE,
	GENERAL,
	ARCHETYPE,
	CLASS
};

enum AnonType : uint8 {
	AT_None = 0,
	AT_Roleplay = 1,
	AT_Anonymous = 2
};

enum GuildRanks : GuildRank {
	Member = 0,
	Officer = 1,
	Leader = 2,
	GR_None = 9
};

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

enum ClassIDs : ClassID {
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

enum ZoneIDs : ZoneID {
	NoZone = 0,
	SouthQeynos = 1,
	NorthQeynos = 2,
	TheSurefallGlade = 3,
	TheQeynosHills = 4,
	HighpassHold = 5,
	HighKeep = 6
};

enum class Statistic : uint32 {
	Strength,
	Stamina,
	Charisma,
	Dexterity,
	Intelligence,
	Agility,
	Wisdom
};

enum SpawnAppearanceType : int16 {
	Die = 0,
	WhoLevel = 1,
	Invisible = 3,
	PVP = 4,
	Light = 5,
	Animation = 14,
	Sneak = 15,
	SA_SpawnID = 16,
	HP = 17,
	LinkDead = 18,
	Levitate = 19,
	GM = 20,
	Anonymous = 21,
	SA_GuildID = 22,
	SA_GuildRank = 23,
	AFK = 24,
	Pet = 25,
	Split = 28,
	Size = 29,
	NPCName = 31, // 1 = Trader <PlayerName>
	AutoConsentGroup = 40,
	AutoConsentRaid = 41,
	AutoConsentGuild = 42,
	ShowHelm = 43,
	DamageState = 44,
	EQPlayersAutoUpdate = 45,
};

enum SpawnAppearanceAnimation : int16 {
	Standing = 100,
	Freeze = 102,
	Looting = 105,
	Sitting = 110,
	Crouch = 111,
	Death = 115
};

enum Gender : GenderID {
	G_MALE,
	G_FEMALE
};

enum BodyType : uint8 {
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

enum class ChannelID : uint32 {
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

enum class MessageType : uint32 {
	White = 0,
	Grey = 1,
	DarkGreen = 2,
	LightBlue = 4,
	Pink = 5,
	Red = 13,
	LightGreen = 14,
	Yellow = 15,
	DarkBlue = 16,
	Aqua = 18,
	////////////////////////////////////////////////
	Say = 256,
	Tell = 257,
	Group = 258,
	Guild = 259,
	OOC = 260,
	Auction = 261,
	Shout = 262,
	Emote = 263,
	Spells = 264,
	YouHitOther = 265,
	OtherHitsYou = 266,
	YouMissOther = 267,
	OtherMissesYou = 268,
	Broadcasts = 269,
	Skills = 270,
	Disciplines = 271,
	Unused1 = 272,
	DefaultText = 273,
	Unused2 = 274,
	MerchantOffer = 275,
	MerchantBuySell = 276,
	YourDeath = 277,
	OtherDeath = 278,
	OtherHits = 279,
	OtherMisses = 280,
	Who = 281,
	YellForHelp = 282,
	NonMelee = 283,
	WornOff = 284,
	MoneySplit = 285,
	LootMessages = 286,
	DiceRoll = 287,
	OtherSpells = 288,
	SpellFailure = 289,
	Chat = 290,
	Channel1 = 291,
	Channel2 = 292,
	Channel3 = 293,
	Channel4 = 294,
	Channel5 = 295,
	Channel6 = 296,
	Channel7 = 297,
	Channel8 = 298,
	Channel9 = 299,
	Channel10 = 300,
	CritMelee = 301,
	SpellCrits = 302,
	TooFarAway = 303,
	NPCRampage = 304,
	NPCFlurry = 305,
	NPCEnrage = 306,
	SayEcho = 307,
	TellEcho = 308,
	GroupEcho = 309,
	GuildEcho = 310,
	OOCEcho = 311,
	AuctionEcho = 312,
	ShoutECho = 313,
	EmoteEcho = 314,
	Chat1Echo = 315,
	Chat2Echo = 316,
	Chat3Echo = 317,
	Chat4Echo = 318,
	Chat5Echo = 319,
	Chat6Echo = 320,
	Chat7Echo = 321,
	Chat8Echo = 322,
	Chat9Echo = 323,
	Chat10Echo = 324,
	DoTDamage = 325,
	ItemLink = 326,
	RaidSay = 327,
	MyPet = 328,
	DS = 329,
	Leadership = 330,
	PetFlurry = 331,
	PetCrit = 332,
	FocusEffect = 333,
	Experience = 334,
	System = 335,
	PetSpell = 336,
	PetResponse = 337,
	ItemSpeech = 338,
	StrikeThrough = 339,
	Stun = 340
};

enum class StringID : uint32 {
	SI_NONE = 0,
	GENERIC_9_STRINGS = 1,		//%1 %2 %3 %4 %5 %6 %7 %8 %9
	TARGET_OUT_OF_RANGE = 100,		//Your target is out of range, get closer!
	TARGET_NOT_FOUND = 101,		//Target player not found.
	TRADE_CANCEL_LORE = 104,		//Trade cancelled, duplicated Lore Items would result.
	CANNOT_BIND = 105,		//You cannot form an affinity with this area. Try a city.
	SPELL_DOES_NOT_WORK_HERE = 106,		//This spell does not work here.
	SPELL_DOES_NOT_WORK_PLANE = 107,		//This spell does not work on this plane.
	CANT_SEE_TARGET = 108,		//You cannot see your target.
	MGB_STRING = 113,		//The next group buff you cast will hit all targets in range.
	ABILITY_FAILED = 116,		//Your ability failed. Timer has been reset.
	ESCAPE = 114,		//You escape from combat, hiding yourself from view.
	AA_OFF = 119,		//Alternate Experience is *OFF*.
	AA_ON = 121,		//Alternate Experience is *ON*.
	TARGET_TOO_FAR = 124,		//Your target is too far away, get closer!
	PROC_TOOLOW = 126,		//Your will is not sufficient to command this weapon.
	PROC_PETTOOLOW = 127,		//Your pet's will is not sufficient to command its weapon.
	YOU_FLURRY = 128,		//You unleash a flurry of attacks.
	DOORS_LOCKED = 130,		//It's locked and you're not holding the key.
	DOORS_CANT_PICK = 131,		//This lock cannot be picked.
	DOORS_INSUFFICIENT_SKILL = 132,		//You are not sufficiently skilled to pick this lock.
	DOORS_GM = 133,		//You opened the locked door with your magic GM key.
	ITEMS_INSUFFICIENT_LEVEL = 136,		//You are not sufficient level to use this item.
	GAIN_XP = 138,		//You gain experience!!
	GAIN_GROUPXP = 139,		//You gain party experience!!
	BOW_DOUBLE_DAMAGE = 143,		//Your bow shot did double dmg.
	FORAGE_GRUBS = 150,		//You have scrounged up some fishing grubs.
	FORAGE_WATER = 151,		//You have scrounged up some water.
	FORAGE_FOOD = 152,		//You have scrounged up some food.
	FORAGE_DRINK = 153,		//You have scrounged up some drink.
	FORAGE_NOEAT = 154,		//You have scrounged up something that doesn't look edible.
	FORAGE_FAILED = 155,		//You fail to locate any food nearby.
	ALREADY_FISHING = 156,		//You are already fishing!
	FISHING_NO_POLE = 160,		//You can't fish without a fishing pole, go buy one.
	FISHING_EQUIP_POLE = 161,		//You need to put your fishing pole in your primary hand.
	FISHING_NO_BAIT = 162,		//You can't fish without fishing bait, go buy some.
	FISHING_CAST = 163,		//You cast your line.
	NOT_SCARING = 164,		//You're not scaring anyone.
	FISHING_STOP = 165,		//You stop fishing and go on your way.
	FISHING_LAND = 166,		//Trying to catch land sharks perhaps?
	FISHING_LAVA = 167,		//Trying to catch a fire elemental or something?
	FISHING_FAILED = 168,		//You didn't catch anything.
	FISHING_POLE_BROKE = 169,		//Your fishing pole broke!
	FISHING_SUCCESS = 170,		//You caught, something...
	FISHING_SPILL_BEER = 171,		//You spill your beer while bringing in your line.
	FISHING_LOST_BAIT = 172,		//You lost your bait!
	SPELL_FIZZLE = 173,		//Your spell fizzles!
	MUST_EQUIP_ITEM = 179,		//You cannot use this item unless it is equipped.
	MISS_NOTE = 180,		//You miss a note, bringing your song to a close!
	CANNOT_USE_ITEM = 181,		//Your race, class, or deity cannot use this item.
	ITEM_OUT_OF_CHARGES = 182,		//Item is out of charges.
	TARGET_NO_MANA = 191,		//Your target has no mana to affect
	TARGET_GROUP_MEMBER = 196,		//You must first target a group member.
	SPELL_TOO_POWERFUL = 197,		//Your spell is too powerful for your intended target.
	INSUFFICIENT_MANA = 199,		//Insufficient Mana to cast this spell!
	SAC_TOO_LOW = 203,		//This being is not a worthy sacrifice.
	SAC_TOO_HIGH = 204,		//This being is too powerful to be a sacrifice.
	CANNOT_SAC_SELF = 205,		//You cannot sacrifice yourself.
	SILENCED_STRING = 207,		//You *CANNOT* cast spells, you have been silenced!
	CANNOT_AFFECT_PC = 210,		//That spell can not affect this target PC.
	SPELL_NEED_TAR = 214,		//You must first select a target for this spell!
	ONLY_ON_CORPSES = 221,		//This spell only works on corpses.
	CANT_DRAIN_SELF = 224,		//You can't drain yourself!
	CORPSE_NOT_VALID = 230,		//This corpse is not valid.
	CORPSE_TOO_OLD = 231,		//This player cannot be resurrected. The corpse is too old.
	CAST_OUTDOORS = 234,		//You can only cast this spell in the outdoors.
	SPELL_RECAST = 236,		//Spell recast time not yet met.
	SPELL_RECOVERY = 237,		//Spell recovery time not yet met.
	CANNOT_MEZ = 239,		//Your target cannot be mesmerized.
	CANNOT_MEZ_WITH_SPELL = 240,		//Your target cannot be mesmerized (with this spell).
	IMMUNE_STUN = 241,		//Your target is immune to the stun portion of this effect.
	IMMUNE_ATKSPEED = 242,		//Your target is immune to changes in its attack speed.
	IMMUNE_FEAR = 243,		//Your target is immune to fear spells.
	IMMUNE_MOVEMENT = 244,		//Your target is immune to changes in its run speed.
	ONLY_ONE_PET = 246,		//You cannot have more than one pet at a time.
	CANNOT_CHARM_YET = 248,		//Your target is too high of a level for your charm spell.
	CANNOT_AFFECT_NPC = 251,		//That spell can not affect this target NPC.
	SUSPEND_MINION_HAS_AGGRO = 256,		//Your pet is the focus of something's attention.
	NO_PET = 255,		//You do not have a pet.
	GATE_FAIL = 260,		//Your gate is too unstable, and collapses.
	CORPSE_CANT_SENSE = 262,		//You cannot sense any corpses for this PC in this zone.
	SPELL_NO_HOLD = 263,		//Your spell did not take hold.
	CANNOT_CHARM = 267,		//This NPC cannot be charmed.
	SPELL_NO_EFFECT = 268,		//Your target looks unaffected.
	NO_INSTRUMENT_SKILL = 269,		//Stick to singing until you learn to play this instrument.
	REGAIN_AND_CONTINUE = 270,		//You regain your concentration and continue your casting.
	SPELL_WOULDNT_HOLD = 271,		//Your spell would not have taken hold on your target.
	MISSING_SPELL_COMP = 272,		//You are missing some required spell components.
	INVIS_BEGIN_BREAK = 275,		//You feel yourself starting to appear.
	DISCIPLINE_CONLOST = 278,		//You lose the concentration to remain in your fighting discipline.
	REZ_REGAIN = 289,		//You regain some experience from resurrection.
	DUP_LORE = 290,		//Duplicate lore items are not allowed.
	TGB_ON = 293,		//Target other group buff is *ON*.
	TGB_OFF = 294,		//Target other group buff is *OFF*.
	LDON_SENSE_TRAP1 = 306,		//You do not Sense any traps.
	TRADESKILL_NOCOMBINE = 334,		//You cannot combine these items in this container type!
	TRADESKILL_FAILED = 336,		//You lacked the skills to fashion the items together.
	TRADESKILL_TRIVIAL = 338,		//You can no longer advance your skill from making this item.
	TRADESKILL_SUCCEED = 339,		//You have fashioned the items together to create something new!
	EVADE_SUCCESS = 343,		//You have momentarily ducked away from the main combat.
	EVADE_FAIL = 344,		//Your attempts at ducking clear of combat fail.
	HIDE_FAIL = 345,		//You failed to hide yourself.
	HIDE_SUCCESS = 346,		//You have hidden yourself from view.
	SNEAK_SUCCESS = 347,		//You are as quiet as a cat stalking its prey.
	SNEAK_FAIL = 348,		//You are as quiet as a herd of running elephants.
	MEND_CRITICAL = 349,		//You magically mend your wounds and heal considerable damage.
	MEND_SUCCESS = 350,		//You mend your wounds and heal some damage.
	MEND_WORSEN = 351,		//You have worsened your wounds!
	MEND_FAIL = 352,		//You have failed to mend your wounds.
	LDON_SENSE_TRAP2 = 367,		//You have not detected any traps.
	LOOT_LORE_ERROR = 371,		//You cannot loot this Lore Item. You already have one.
	PICK_LORE = 379,		//You cannot pick up a lore item you already possess.
	CONSENT_DENIED = 390,		//You do not have consent to summon that corpse.
	DISCIPLINE_RDY = 393,		//You are ready to use a new discipline now.
	CONSENT_INVALID_NAME = 397,		//Not a valid consent name.
	CONSENT_NPC = 398,		//You cannot consent NPC\'s.
	CONSENT_YOURSELF = 399,		//You cannot consent yourself.
	SONG_NEEDS_DRUM = 405,		//You need to play a percussion instrument for this song
	SONG_NEEDS_WIND = 406,		//You need to play a wind instrument for this song
	SONG_NEEDS_STRINGS = 407,		//You need to play a stringed instrument for this song
	SONG_NEEDS_BRASS = 408,		//You need to play a brass instrument for this song
	AA_GAIN_ABILITY = 410,		//You have gained the ability "%T1" at a cost of %2 ability %T3.
	AA_IMPROVE = 411,		//You have improved %T1 %2 at a cost of %3 ability %T4.
	AA_REUSE_MSG = 413,		//You can use the ability %B1(1) again in %2 hour(s) %3 minute(s) %4 seconds.
	AA_REUSE_MSG2 = 414,		//You can use the ability %B1(1) again in %2 minute(s) %3 seconds.
	YOU_HEALED = 419,		//%1 has healed you for %2 points of damage.
	BEGINS_TO_GLOW = 422,		//Your %1 begins to glow.
	ALREADY_INVIS = 423,		//%1 tries to cast an invisibility spell on you, but you are already invisible.
	YOU_ARE_PROTECTED = 424,		//%1 tries to cast a spell on you, but you are protected.
	TARGET_RESISTED = 425,		//Your target resisted the %1 spell.
	YOU_RESIST = 426,		//You resist the %1 spell!
	YOU_CRIT_HEAL = 427,		//You perform an exceptional heal! (%1)
	YOU_CRIT_BLAST = 428,		//You deliver a critical blast! (%1)
	SUMMONING_CORPSE = 429,		//Summoning your corpse.
	SUMMONING_CORPSE_OTHER = 430,		//Summoning %1's corpse.
	MISSING_SPELL_COMP_ITEM = 433,		//You are missing %1.
	OTHER_HIT_NONMELEE = 434,		//%1 was hit by non-melee for %2 points of damage.
	SPELL_WORN_OFF_OF = 436,		//Your %1 spell has worn off of %2.
	SPELL_WORN_OFF = 437,		//Your %1 spell has worn off.
	PET_TAUNTING = 438,		//Taunting attacker, Master.
	INTERRUPT_SPELL = 439,		//Your spell is interrupted.
	LOSE_LEVEL = 442,		//You LOST a level! You are now level %1!
	GAIN_ABILITY_POINT = 446,		//You have gained an ability point! You now have %1 ability point%2.
	GAIN_LEVEL = 447,		//You have gained a level! Welcome to level %1!
	LANG_SKILL_IMPROVED = 449,		//Your language skills have improved.
	OTHER_LOOTED_MESSAGE = 466,		//--%1 has looted a %2--
	LOOTED_MESSAGE = 467,		//--You have looted a %1--
	FACTION_WORST = 469,		//Your faction standing with %1 could not possibly get any worse.
	FACTION_WORSE = 470,		//Your faction standing with %1 got worse.
	FACTION_BEST = 471,		//Your faction standing with %1 could not possibly get any better.
	FACTION_BETTER = 472,		//Your faction standing with %1 got better.
	PET_REPORT_HP = 488,		//I have %1 percent of my hit points left.
	PET_NO_TAUNT = 489,		//No longer taunting attackers, Master.
	PET_DO_TAUNT = 490,		//Taunting attackers as normal, Master.
	CORPSE_DECAY1 = 495,		//This corpse will decay in %1 minute(s) %2 seconds.
	DISC_LEVEL_ERROR = 503,		//You must be a level %1 ... to use this discipline.
	DISCIPLINE_CANUSEIN = 504,		//You can use a new discipline in %1 minutes %2 seconds.
	PVP_ON = 552,		//You are now player kill and follow the ways of Discord.
	GENERIC_STRINGID_SAY = 554,		//%1 says '%T2'
	CANNOT_WAKE = 555,		//%1 tells you, 'I am unable to wake %2, master.'
	GUILD_NAME_IN_USE = 711,		//You cannot create a guild with that name, that guild already exists on this server.
	GM_GAINXP = 1002,	//[GM] You have gained %1 AXP and %2 EXP (%3).
	FINISHING_BLOW = 1009,	//%1 scores a Finishing Blow!!
	ASSASSINATES = 1016,	//%1 ASSASSINATES their victim!!
	CRIPPLING_BLOW = 1021,	//%1 lands a Crippling Blow!(%2)
	CRITICAL_HIT = 1023,	//%1 scores a critical hit! (%2)
	DEADLY_STRIKE = 1024,	//%1 scores a Deadly Strike!(%2)
	RESISTS_URGE = 1025,	//%1 resists their urge to flee.
	BERSERK_START = 1027,	//%1 goes into a berserker frenzy!
	DEATH_PACT = 1028,	//%1's death pact has been benevolently fulfilled!
	DIVINE_INTERVENTION = 1029,	//%1 has been rescued by divine intervention!
	BERSERK_END = 1030,	//%1 is no longer berserk.
	GATES = 1031,	//%1 Gates.
	GENERIC_SAY = 1032,	//%1 says '%2'
	OTHER_REGAIN_CAST = 1033,	//%1 regains concentration and continues casting.
	GENERIC_SHOUT = 1034,	//%1 shouts '%2'
	GENERIC_EMOTE = 1036,	//%1 %2
	OTHER_CRIT_HEAL = 1039,	//%1 performs an exceptional heal! (%2)
	OTHER_CRIT_BLAST = 1040,	//%1 delivers a critical blast! (%2)
	NPC_ENRAGE_START = 1042,	//%1 has become ENRAGED.
	NPC_ENRAGE_END = 1043,	//%1 is no longer enraged.
	NPC_RAMPAGE = 1044,	//%1 goes on a RAMPAGE!
	NPC_FLURRY = 1045,	//%1 executes a FLURRY of attacks on %2!
	DISCIPLINE_FEARLESS = 1076,	//%1 becomes fearless.
	DUEL_FINISHED = 1088,	//dont know text
	EATING_MESSAGE = 1091,	//Chomp, chomp, chomp... %1 takes a bite from a %2.
	DRINKING_MESSAGE = 1093,	//Glug, glug, glug... %1 takes a drink from a %2.
	SUCCESSFUL_TAUNT = 1095,	//I'll teach you to interfere with me %3.
	PET_SIT_STRING = 1130,	//Changing position, Master.
	PET_CALMING = 1131,	//Sorry, Master..calming down.
	PET_FOLLOWING = 1132,	//Following you, Master.
	PET_GUARDME_STRING = 1133,	//Guarding you, Master.
	PET_GUARDINGLIFE = 1134,	//Guarding with my life..oh splendid one.
	PET_GETLOST_STRING = 1135,	//As you wish, oh great one.
	PET_LEADERIS = 1136,	//My leader is %3.
	I_FOLLOW_NOONE = 1137,	//I follow no one.
	PET_ON_HOLD = 1138,	//Waiting for your order to attack, Master.
	NOT_LEGAL_TARGET = 1139,	//I beg forgiveness, Master. That is not a legal target.
	MERCHANT_BUSY = 1143,	//I'm sorry, I am busy right now.
	MERCHANT_GREETING = 1144,	//Welcome to my shop, %3.
	MERCHANT_HANDY_ITEM1 = 1145,	//Hello there, %3. How about a nice %4?
	MERCHANT_HANDY_ITEM2 = 1146,	//Greetings, %3. You look like you could use a %4.
	MERCHANT_HANDY_ITEM3 = 1147,	//Hi there %3, just browsing? Have you seen the %4 I just got in?
	MERCHANT_HANDY_ITEM4 = 1148,	//Welcome to my shop, %3. You would probably find a %4 handy.
	AA_POINT = 1197,	//point
	AA_POINTS = 1215,	//points
	SPELL_FIZZLE_OTHER = 1218,	//%1's spell fizzles!
	MISSED_NOTE_OTHER = 1219,	//A missed note brings %1's song to a close!
	SPELL_LEVEL_REQ = 1226,	//This spell only works on people who are level %1 and under.
	CORPSE_DECAY_NOW = 1227,	//This corpse is waiting to expire.
	SURNAME_REJECTED = 1374,	//Your new surname was rejected. Please try a different name.
	DUEL_DECLINE = 1383,	//%1 has declined your challenge to duel to the death.
	DUEL_ACCEPTED = 1384,	//%1 has already accepted a duel with someone else.
	DUEL_CONSIDERING = 1385,	//%1 is considering a duel with someone else.
	PLAYER_REGAIN = 1394,	//You have control of yourself again.
	REZZ_ALREADY_PENDING = 1379,	//You were unable to restore the corpse to life, but you may have success with a later attempt.
	IN_USE = 1406,	//Someone else is using that. Try again later.
	DUEL_FLED = 1408,	//%1 has defeated %2 in a duel to the death! %3 has fled like a cowardly dog!
	MEMBER_OF_YOUR_GUILD = 1429,
	OFFICER_OF_YOUR_GUILD = 1430,
	LEADER_OF_YOUR_GUILD = 1431,
	RECEIVED_PLATINUM = 1452,	//You receive %1 Platinum from %2.
	RECEIVED_GOLD = 1453,	//You receive %1 Gold from %2.
	RECEIVED_SILVER = 1454,	//You receive %1 Silver from %2.
	RECEIVED_COPPER = 1455,	//You receive %1 Copper from %2.
	STRING_FEIGNFAILED = 1456,	//%1 has fallen to the ground.
	DOORS_SUCCESSFUL_PICK = 1457,	//You successfully picked the lock.
	PLAYER_CHARMED = 1461,	//You lose control of yourself!
	TRADER_BUSY = 1468,	//That Trader is currently with a customer. Please wait until their transaction is finished.
	SENSE_CORPSE_DIRECTION = 1563,	//You sense a corpse in this direction.
	QUEUED = 2458, // [queued]
	SUSPEND_MINION_UNSUSPEND = 3267,	//%1 tells you, 'I live again...'
	SUSPEND_MINION_SUSPEND = 3268,	//%1 tells you, 'By your command, master.'
	ONLY_SUMMONED_PETS = 3269,	//3269 This effect only works with summoned pets.
	SUSPEND_MINION_FIGHTING = 3270,	//Your pet must be at peace, first.
	ALREADY_SHIELDED = 3279,	//Either you or your target is already being shielded.
	START_SHIELDING = 3281,	//%1 begins to use %2 as a living shield!
	END_SHIELDING = 3282,	//%1 ceases protecting %2.
	TRADESKILL_MISSING_ITEM = 3455,	//You are missing a %1.
	TRADESKILL_MISSING_COMPONENTS = 3456,	//Sorry, but you don't have everything you need for this recipe in your general inventory.
	TRADESKILL_LEARN_RECIPE = 3457,	//You have learned the recipe %1!
	EXPEDITION_MIN_REMAIN = 3551,	//You only have %1 minutes remaining before this expedition comes to an end.
	REWIND_WAIT = 4059,	//You must wait a bit longer before using the rewind command again.
	CORPSEDRAG_LIMIT = 4061,	//You are already dragging as much as you can!
	CORPSEDRAG_ALREADY = 4062,	//You are already dragging %1.
	CORPSEDRAG_SOMEONE_ELSE = 4063,	//Someone else is dragging %1.
	CORPSEDRAG_BEGIN = 4064,	//You begin to drag %1.
	CORPSEDRAG_STOPALL = 4065,	//You stop dragging the corpses.
	CORPSEDRAG_STOP = 4066,	//You stop dragging the corpse.
	WHOALL_NO_RESULTS = 5029,	//There are no players in EverQuest that match those who filters.
	TELL_QUEUED = 5045, // You told %1 '%T2, %3'
	PLAYER_NOT_ONLINE = 5046, // % 1 is not online at this time.
	PETITION_NO_DELETE = 5053,	//You do not have a petition in the queue.
	PETITION_DELETED = 5054,	//Your petition was successfully deleted.
	GAIN_RAIDEXP = 5085,	//You gained raid experience!
	DUNGEON_SEALED = 5141,	//The gateway to the dungeon is sealed off to you.  Perhaps you would be able to enter if you needed to adventure there.
	ADVENTURE_COMPLETE = 5147,	//You received %1 points for successfully completing the adventure.
	SUCCOR_FAIL = 5169,	//The portal collapes before you can escape!
	PET_ATTACKING = 5501,	//%1 tells you, 'Attacking %2 Master.'
	FATAL_BOW_SHOT = 5745,	//%1 performs a FATAL BOW SHOT!!
	MELEE_SILENCE = 5806,	//You *CANNOT* use this melee ability, you are suffering from amnesia!
	DISCIPLINE_REUSE_MSG = 5807,	//You can use the ability %1 again in %2 hour(s) %3 minute(s) %4 seconds.
	DISCIPLINE_REUSE_MSG2 = 5808,	//You can use the ability %1 again in %2 minute(s) %3 seconds.
	FAILED_TAUNT = 5811,	//You have failed to taunt your target.
	PHYSICAL_RESIST_FAIL = 5817,	//Your target avoided your %1 ability.
	AA_NO_TARGET = 5825,	//You must first select a target for this ability!
	FORAGE_MASTERY = 6012,	//Your forage mastery has enabled you to find something else!
	GUILD_BANK_CANNOT_DEPOSIT = 6097,	// Cannot deposit this item. Containers must be empty, and only one of each LORE and no NO TRADE or TEMPORARY items may be deposited.
	GUILD_BANK_FULL = 6098,	// There is no more room in the Guild Bank.
	GUILD_BANK_TRANSFERRED = 6100,	// '%1' transferred to Guild Bank from Deposits.
	GUILD_BANK_EMPTY_HANDS = 6108,	// You must empty your hands to withdraw from the Guild Bank.
	GENERIC_STRING = 6688,	//%1 (used to any basic message)
	TELL_ECHO = 6719, // You told % 1, '%2'
	SENTINEL_TRIG_YOU = 6724,	//You have triggered your sentinel.
	SENTINEL_TRIG_OTHER = 6725,	//%1 has triggered your sentinel.
	IDENTIFY_SPELL = 6765,	//Item Lore: %1.
	LDON_DONT_KNOW_TRAPPED = 7552,	//You do not know if this object is trapped.
	LDON_HAVE_DISARMED = 7553,	//You have disarmed %1!
	LDON_ACCIDENT_SETOFF = 7554,	//You accidentally set off the trap!
	LDON_HAVE_NOT_DISARMED = 7555,	//You have not disarmed %1.
	LDON_ACCIDENT_SETOFF2 = 7556,	//You accidentally set off the trap!
	LDON_CERTAIN_TRAP = 7557,	//You are certain that %1 is trapped.
	LDON_CERTAIN_NOT_TRAP = 7558,	//You are certain that %1 is not trapped.
	LDON_CANT_DETERMINE_TRAP = 7559,	//You are unable to determine if %1 is trapped.
	LDON_PICKLOCK_SUCCESS = 7560,	//You have successfully picked %1!
	LDON_PICKLOCK_FAILURE = 7561,	//You have failed to pick %1.
	LDON_STILL_LOCKED = 7562,	//You cannot open %1, it is locked.
	LDON_BASH_CHEST = 7563,	//%1 try to %2 %3, but do no damage.
	DOORS_NO_PICK = 7564,	//You must have a lock pick in your inventory to do this.
	LDON_NO_LOCKPICK = 7564,	//You must have a lock pick in your inventory to do this.
	LDON_WAS_NOT_LOCKED = 7565,	//%1 was not locked.
	LDON_WAS_NOT_TRAPPED = 7566,	//%1 was not trapped
	GAIN_GROUP_LEADERSHIP_POINT = 8585,	//
	GAIN_RAID_LEADERSHIP_POINT = 8589,	//
	MAX_GROUP_LEADERSHIP_POINTS = 8584,	//
	MAX_RAID_LEADERSHIP_POINTS = 8591,	//
	LEADERSHIP_EXP_ON = 8653,	//
	LEADERSHIP_EXP_OFF = 8654,	//
	CURRENT_SPELL_EFFECTS = 8757,	//%1's current spell effects:
	GAIN_GROUP_LEADERSHIP_EXP = 8788,	//
	GAIN_RAID_LEADERSHIP_EXP = 8789,	//
	BUFF_MINUTES_REMAINING = 8799,	//%1 (%2 minutes remaining)
	FEAR_TOO_HIGH = 9035,	//Your target is too high of a level for your fear spell.
	SLOW_MOSTLY_SUCCESSFUL = 9029,	//Your spell was mostly successful.
	SLOW_PARTIALLY_SUCCESSFUL = 9030,	// Your spell was partially successful.
	SLOW_SLIGHTLY_SUCCESSFUL = 9031,	//Your spell was slightly successful.
	SPELL_OPPOSITE_EFFECT = 9032,	//Your spell may have had the opposite effect of what you desired.
	HAS_BEEN_AWAKENED = 9037,	//%1 has been awakened by %2.
	YOU_HEAL = 9068,	//You have healed %1 for %2 points of damage.
	YOUR_HIT_DOT = 9072,	//%1 has taken %2 damage from your %3.
	HIT_NON_MELEE = 9073,	//%1 hit %2 for %3 points of non-melee damage.
	SHAKE_OFF_STUN = 9077,
	STRIKETHROUGH_STRING = 9078,	//You strike through your opponent's defenses!
	SPELL_REFLECT = 9082,	//%1's spell has been reflected by %2.
	NEW_SPELLS_AVAIL = 9149,	//You have new spells available to you. Check the merchants near your guild master.
	PET_NOW_FOCUSING = 9254,	//Focusing on one target, Master.
	PET_NOT_FOCUSING = 9263,	//No longer focusing on one target, Master.
	PET_NOT_CASTING = 9264,	//Not casting spells, Master.
	PET_CASTING = 9291,	//Casting spells normally, Master.
	AE_RAMPAGE = 11015,	//%1 goes on a WILD RAMPAGE!
	FACE_ACCEPTED = 12028,	//Facial features accepted.
	SPELL_LEVEL_TO_LOW = 12048,	//You will have to achieve level %1 before you can scribe the %2.
	ATTACKFAILED = 12158,	//%1 try to %2 %3, but %4!
	HIT_STRING = 12183,	//hit
	CRUSH_STRING = 12191,	//crush
	PIERCE_STRING = 12193,	//pierce
	KICK_STRING = 12195,	//kick
	STRIKE_STRING = 12197,	//strike
	BACKSTAB_STRING = 12199,	//backstab
	BASH_STRING = 12201,	//bash
	GUILD_NOT_MEMBER = 12242,	//You are not a member of any guild.
	MEMBER_OF_X_GUILD = 12256,
	OFFICER_OF_X_GUILD = 12257,
	LEADER_OF_X_GUILD = 12258,
	NOT_IN_A_GUILD = 12259,
	TARGET_PLAYER_FOR_GUILD_STATUS = 12260,
	GROUP_INVITEE_NOT_FOUND = 12268,	//You must target a player or use /invite <name> to invite someone to your group.
	GROUP_INVITEE_SELF = 12270,	//12270 You cannot invite yourself.
	NOT_IN_CONTROL = 12368,	//You do not have control of yourself right now.
	ALREADY_CASTING = 12442,	//You are already casting a spell!
	SENSE_CORPSE_NOT_NAME = 12446,	//You don't sense any corpses of that name.
	SENSE_CORPSE_NONE = 12447,	//You don't sense any corpses.
	SCREECH_BUFF_BLOCK = 12448,	//Your immunity buff protected you from the spell %1!
	NOT_HOLDING_ITEM = 12452,	//You are not holding an item!
	SENSE_UNDEAD = 12471,	//You sense undead in this direction.
	SENSE_ANIMAL = 12472,	//You sense an animal in this direction.
	SENSE_SUMMONED = 12473,	//You sense a summoned being in this direction.
	SENSE_NOTHING = 12474,	//You don't sense anything.
	LDON_SENSE_TRAP3 = 12476,	//You don't sense any traps.
	INTERRUPT_SPELL_OTHER = 12478,	//%1's casting is interrupted!
	YOU_HIT_NONMELEE = 12481,	//You were hit by non-melee for %1 damage.
	TRACK_LOST_TARGET = 12681,	//You have lost your tracking target.
	TRACK_STRAIGHT_AHEAD = 12676,
	TRACK_AHEAD_AND_TO = 12677,
	TRACK_TO_THE = 12678,
	TRACK_BEHIND_AND_TO = 12679,
	TRACK_BEHIND_YOU = 12680,
	BEAM_SMILE = 12501,	//%1 beams a smile at %2
	SONG_ENDS_ABRUPTLY = 12686,	//Your song ends abruptly.
	SONG_ENDS = 12687,	//Your song ends.
	SONG_ENDS_OTHER = 12688,	//%1's song ends.
	SONG_ENDS_ABRUPTLY_OTHER = 12689,	//%1's song ends abruptly.
	DIVINE_AURA_NO_ATK = 12695,	//You can't attack while invulnerable!
	TRY_ATTACKING_SOMEONE = 12696,	//Try attacking someone other than yourself, it's more productive.
	BACKSTAB_WEAPON = 12874,	//You need a piercing weapon as your primary weapon in order to backstab
	MORE_SKILLED_THAN_I = 12931,	//%1 tells you, 'You are more skilled than I! What could I possibly teach you?'
	SURNAME_EXISTS = 12939,	//You already have a surname. Operation failed.
	SURNAME_LEVEL = 12940,	//You can only submit a surname upon reaching the 20th level. Operation failed.
	SURNAME_TOO_LONG = 12942,	//Surname must be less than 20 characters in length.
	REPORT_ONCE = 12945,	//You may only submit a report once per time that you zone. Thank you.
	NOW_INVISIBLE = 12950,	//%1 is now Invisible.
	NOW_VISIBLE = 12951,	//%1 is now Visible.
	GUILD_NOT_MEMBER2 = 12966,	//You are not in a guild.
	HOT_HEAL_SELF = 12976,	//You have been healed for %1 hit points by your %2.
	HOT_HEAL_OTHER = 12997,	//You have healed %1 for %2 hit points with your %3.
	HOT_HEALED_OTHER = 12998,	//%1 healed you for %2 hit points by %3.
	DISC_LEVEL_USE_ERROR = 13004,	//You are not sufficient level to use this discipline.
	TOGGLE_ON = 13172,	//Asking server to turn ON your incoming tells.
	TOGGLE_OFF = 13173,	//Asking server to turn OFF all incoming tells for you.
	DUEL_INPROGRESS = 13251,	//You have already accepted a duel with someone else cowardly dog.
	OTHER_HIT_DOT = 13327,	//%1 has taken %2 damage from %3 by %4.
	GENERIC_MISS = 15041,	//%1 missed %2
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
	String mName;
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