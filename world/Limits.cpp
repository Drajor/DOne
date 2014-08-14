#include "Limits.h"

bool Limits::Character::nameLength(const String& pCharacterName) {
	return pCharacterName.length() + 1 >= MIN_CHARACTER_NAME_LENGTH && pCharacterName.length() + 1 <= MAX_CHARACTER_NAME_LENGTH;
}

bool Limits::Character::classID(const ClassID pClassID) {
	switch (pClassID) {
	case ClassIDs::Warrior:
	case ClassIDs::Cleric:
	case ClassIDs::Paladin:
	case ClassIDs::Ranger:
	case ClassIDs::Shadowknight:
	case ClassIDs::Druid:
	case ClassIDs::Monk:
	case ClassIDs::Bard:
	case ClassIDs::Rogue:
	case ClassIDs::Shaman:
	case ClassIDs::Necromancer:
	case ClassIDs::Wizard:
	case ClassIDs::Magician:
	case ClassIDs::Enchanter:
	case ClassIDs::Beastlord:
	case ClassIDs::Berserker:
		return true;
	default:
		break;
	}

	return false;
}

bool Limits::Guild::nameLength(const String& pGuildName) {
	return pGuildName.length() + 1 >= Limits::Guild::MIN_NAME_LENGTH && pGuildName.length() + 1 <= Limits::Guild::MAX_NAME_LENGTH;
}

bool Limits::Guild::MOTDLength(const String& pGuildMOTD){
	return pGuildMOTD.length() + 1 <= Limits::Guild::MAX_MOTD_LENGTH;
}

bool Limits::Guild::ID(const GuildID pGuildID) {
	return pGuildID < Limits::Guild::MAX_GUILDS;
}

bool Limits::Guild::publicNoteLength(const String& pPublicNote) {
	return pPublicNote.length() + 1 <= Limits::Guild::MAX_PUBLIC_NOTE_LENGTH;
}

bool Limits::Guild::personalNoteLength(const String& pPersonalNote) {
	return pPersonalNote.length() + 1 <= Limits::Guild::MAX_PERSONAL_NOTE_LENGTH;
}
