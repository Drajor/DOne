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
		const bool nameLength(const String& pCharacterName);
		const bool nameInputLength(const String& pCharacterName);
		bool surnameLengthPayload(const String& pSurname);
		bool surnameLengthClient(const String& pSurname);
		bool classID(const ClassID pClassID);
	}
	namespace SpellBook {
		const bool slotValid(const uint16 pSlot);
		const bool spellIDValid(const uint32 pSpellID);
	}
	namespace SpellBar {
		const bool slotValid(const uint16 pSlot);
		const bool spellIDValid(const uint32 pSpellID);
	}
	namespace Spells {
		const bool spellIDValid(const uint16 pSpellID);
	}
	namespace Skills {
		const bool validID(const uint32 pSkillID);
	}
	namespace Languages {
		const bool validID(const uint32 pLanguageID);
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