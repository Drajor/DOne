#pragma once

#include "Constants.h"

namespace Limits {
	namespace World {
		bool shortNameLength(const String& pShortName);
		bool longNameLength(const String& pLongName);
		bool lsAccountNameLength(const String& pAccountName);
		bool lsAccountPasswordLength(const String& pAccountPassword);
	}
	namespace UCS {

	}
	namespace LoginServer {
		bool accountNameLength(const String& pAccountName);
	}
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
		bool urlLength(const String& pURL);
		bool channelLength(const String& pChannel);
		bool rankValid(const GuildRank pRank);
	}
}