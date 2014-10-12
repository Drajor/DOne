#include "Limits.h"

std::size_t stringLength(const String& pString) { return pString.length() + 1; }

const bool Limits::Character::nameLength(const String& pCharacterName) {
	return stringLength(pCharacterName) >= Limits::Character::MIN_NAME_LENGTH && stringLength(pCharacterName) <= Limits::Character::MAX_NAME_LENGTH;
}

const bool Limits::Character::nameInputLength(const String& pCharacterName) {
	return stringLength(pCharacterName) >= Limits::Character::MIN_INPUT_LENGTH && stringLength(pCharacterName) <= Limits::Character::MAX_INPUT_LENGTH;
}

bool Limits::Character::surnameLengthPayload(const String& pSurname) {
	return stringLength(pSurname) <= Limits::Character::MAX_LAST_NAME_LENGTH;
}

bool Limits::Character::surnameLengthClient(const String& pSurname) {
	return stringLength(pSurname) <= Limits::Character::MAX_LAST_NAME_CLIENT_LENGTH;
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

bool Limits::Guild::rankValid(const GuildRank pRank) {
	switch (pRank) {
	case GuildRanks::Member:
	case GuildRanks::Officer:
	case GuildRanks::Leader:
		return true;
	default:
		break;
	}

	return false;
}

bool Limits::LoginServer::accountNameLength(const String& pAccountName) {
	return stringLength(pAccountName) <= Limits::LoginServer::MAX_ACCOUNT_NAME_LENGTH;
}

bool Limits::World::shortNameLength(const String& pShortName) {
	return true;
	// TODO: Find exact values.
}

bool Limits::World::longNameLength(const String& pLongName) {
	return true;
	// TODO: Find exact values.
}

bool Limits::World::lsAccountNameLength(const String& pAccountName) {
	return true;
	// TODO: Find exact length.
}

bool Limits::World::lsAccountPasswordLength(const String& pAccountPassword) {
	return true;
	// TODO: Find exact length.
}

const bool Limits::SpellBook::slotValid(const uint16 pSlot) {
	return pSlot < Limits::SpellBook::MAX_SLOTS;
}

const bool Limits::SpellBook::spellIDValid(const uint32 pSpellID) {
	return true; // TODO:
}

const bool Limits::SpellBar::slotValid(const uint16 pSlot) {
	return pSlot < Limits::SpellBar::MAX_SLOTS;
}

const bool Limits::SpellBar::spellIDValid(const uint32 pSpellID) {
	return true; // TODO:
}

const bool Limits::Spells::spellIDValid(const uint16 pSpellID) {
	return true;
}

const bool Limits::Skills::validID(const uint32 pSkillID) {
	return pSkillID < Skills::MAX_ID;
}

const bool Limits::Languages::validID(const uint32 pLanguageID) {
	return pLanguageID < Languages::MAX_ID;
}

const bool Limits::General::moneySlotIDValid(const uint32 pSlot) {
	switch (pSlot) {
	case MoneySlotID::CURSOR:
	case MoneySlotID::PERSONAL:
	case MoneySlotID::BANK:
	case MoneySlotID::SHARED_BANK:
		return true;
	default:
		return false;
	}
}

const bool Limits::General::moneyTypeValid(const uint32 pType) {
	switch (pType) {
	case MoneyType::COPPER:
	case MoneyType::SILVER:
	case MoneyType::GOLD:
	case MoneyType::PLATINUM:
		return true;
	default:
		return false;
	}
}
