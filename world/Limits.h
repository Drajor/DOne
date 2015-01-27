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
	namespace General {
		const bool moneySlotIDValid(const uint32 pSlot);
		const bool moneyTypeValid(const uint32 pType);
	}
	namespace Character {
		const bool nameLength(const String& pCharacterName);
		const bool nameInputLength(const String& pCharacterName);
		bool surnameLengthPayload(const String& pSurname);
		bool surnameLengthClient(const String& pSurname);

		// Checks whether pClassID is valid for Character creation.
		const bool classID(const uint8 pClassID);

		// Checks whether pRaceID is valid for Character creation.
		const bool raceID(const uint32 pRaceID);

		// Checks whether pGenderID is valid for Character creation.
		const bool genderID(const u8 pGenderID);

		// Checks whether pDeityID is valid for Character creation.
		const bool deityID(const u32 pDeityID);
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
		bool ID(const u32 pGuildID);
		bool MOTDLength(const String& pGuildMOTD);
		bool nameLength(const String& pGuildName);
		const bool nameValid(const String& pGuildName);
		bool publicNoteLength(const String& pPublicNote);
		bool urlLength(const String& pURL);
		bool channelLength(const String& pChannel);
		bool rankValid(const u8 pRank);
	}
	namespace Shop {
		const bool quantityValid(i32 pQuantity);
	}
}