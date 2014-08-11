#include "Limits.h"

bool Limits::characterNameLength(const String& pCharacterName) {
	return pCharacterName.length() >= MIN_CHARACTER_NAME_LENGTH && pCharacterName.length() <= MAX_CHARACTER_NAME_LENGTH;
}

bool Limits::guildNameLength(const String& pGuildName) {
	return pGuildName.length() >= MIN_GUILD_NAME_LENGTH && pGuildName.length() <= MAX_GUILD_NAME_LENGTH;
}
