#include "Limits.h"

std::size_t stringLength(const String& pString) { return pString.length() + 1; }

bool Limits::Character::nameLength(const String& pCharacterName) {
	return stringLength(pCharacterName) >= Limits::Character::MIN_NAME_LENGTH && stringLength(pCharacterName) <= Limits::Character::MAX_NAME_LENGTH;
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
	return stringLength(pGuildName) >= Limits::Guild::MIN_NAME_LENGTH && stringLength(pGuildName) <= Limits::Guild::MAX_NAME_LENGTH;
}

bool Limits::Guild::MOTDLength(const String& pGuildMOTD){
	return stringLength(pGuildMOTD) <= Limits::Guild::MAX_MOTD_LENGTH;
}

bool Limits::Guild::ID(const GuildID pGuildID) {
	return pGuildID < Limits::Guild::MAX_GUILDS;
}

bool Limits::Guild::publicNoteLength(const String& pPublicNote) {
	return stringLength(pPublicNote) <= Limits::Guild::MAX_PUBLIC_NOTE_LENGTH;
}

bool Limits::Guild::personalNoteLength(const String& pPersonalNote) {
	return stringLength(pPersonalNote) <= Limits::Guild::MAX_PERSONAL_NOTE_LENGTH;
}

bool Limits::Guild::urlLength(const String& pURL) {
	return stringLength(pURL) <= Limits::Guild::MAX_URL_LENGTH;
}

bool Limits::Guild::channelLength(const String& pChannel) {
	return stringLength(pChannel) <= Limits::Guild::MAX_CHANNEL_LENGTH;
}
