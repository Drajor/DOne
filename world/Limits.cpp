#include "Limits.h"
#include "SpellContants.h"
#include "Utility.h"

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

const bool Limits::Character::classID(const uint8 pClassID) {
	switch (pClassID) {
	case ClassID::Warrior:
	case ClassID::Cleric:
	case ClassID::Paladin:
	case ClassID::Ranger:
	case ClassID::Shadowknight:
	case ClassID::Druid:
	case ClassID::Monk:
	case ClassID::Bard:
	case ClassID::Rogue:
	case ClassID::Shaman:
	case ClassID::Necromancer:
	case ClassID::Wizard:
	case ClassID::Magician:
	case ClassID::Enchanter:
	case ClassID::Beastlord:
	case ClassID::Berserker:
		return true;
	default:
		break;
	}

	return false;
}

const bool Limits::Character::raceID(const uint32 pRaceID) {
	return true;
}

const bool Limits::Character::genderID(const u8 pGenderID) {
	return pGenderID == Gender::Male || pGenderID == Gender::Female;
}

const bool Limits::Character::deityID(const u32 pDeityID) {
	return true;
}

bool Limits::Guild::nameLength(const String& pGuildName) {
	return stringLength(pGuildName) >= Limits::Guild::MIN_NAME_LENGTH && stringLength(pGuildName) <= Limits::Guild::MAX_NAME_LENGTH;
}

bool Limits::Guild::MOTDLength(const String& pGuildMOTD){
	return stringLength(pGuildMOTD) <= Limits::Guild::MAX_MOTD_LENGTH;
}

bool Limits::Guild::ID(const u32 pGuildID) {
	return pGuildID < Limits::Guild::MAX_GUILDS;
}

bool Limits::Guild::publicNoteLength(const String& pPublicNote) {
	return stringLength(pPublicNote) <= Limits::Guild::MAX_PUBLIC_NOTE_LENGTH;
}

bool Limits::Guild::urlLength(const String& pURL) {
	return stringLength(pURL) <= Limits::Guild::MAX_URL_LENGTH;
}

bool Limits::Guild::channelLength(const String& pChannel) {
	return stringLength(pChannel) <= Limits::Guild::MAX_CHANNEL_LENGTH;
}

bool Limits::Guild::rankValid(const u8 pRank) {
	switch (pRank) {
	case GuildRank::Member:
	case GuildRank::Officer:
	case GuildRank::Leader:
		return true;
	default:
		break;
	}

	return false;
}

const bool Limits::Guild::nameValid(const String& pGuildName) {
	// Check: Length.
	if (!nameLength(pGuildName)) return false;
	
	//// Check: Has numbers.
	//if (Utility::containsDigits(pGuildName)) return false;

	// Check: Special characters.

	return true;
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
	return pSpellID < MaxSpellID;
}

const bool Limits::Skills::validID(const uint32 pSkillID) {
	return pSkillID < Skills::MAX_ID;
}

const bool Limits::Languages::validID(const uint32 pLanguageID) {
	return pLanguageID < Languages::MAX_ID;
}

const bool Limits::General::moneySlotIDValid(const uint32 pSlot) {
	switch (pSlot) {
	case CurrencySlot::Cursor:
	case CurrencySlot::Personal:
	case CurrencySlot::Bank:
	case CurrencySlot::Trade:
	case CurrencySlot::SharedBank:
		return true;
	default:
		return false;
	}
}

const bool Limits::General::moneyTypeValid(const uint32 pType) {
	switch (pType) {
	case CurrencyType::Copper:
	case CurrencyType::Silver:
	case CurrencyType::Gold:
	case CurrencyType::Platinum:
		return true;
	default:
		return false;
	}
}

const bool Limits::Shop::quantityValid(i32 pQuantity) {
	// Negative 1 or any positive integer is valid.
	return pQuantity == -1 || pQuantity > 0;
}
