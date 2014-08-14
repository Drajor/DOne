#pragma once

#include "Constants.h"

namespace Limits {
	namespace Character {
		bool nameLength(const String& pCharacterName);
		bool classID(const ClassID pClassID);
	}
	namespace Guild {
		bool ID(const GuildID pGuildID);
		bool MOTDLength(const String& pGuildMOTD);
		bool nameLength(const String& pGuildName);
		bool publicNoteLength(const String& pPublicNote);
		bool personalNoteLength(const String& pPersonalNote);
	}
}