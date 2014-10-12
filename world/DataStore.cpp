#include "DataStore.h"
#include "Utility.h"
#include "Limits.h"
#include "Profile.h"
#include "Settings.h"
#include "ItemData.h"

#include <Windows.h>
#include "../common/tinyxml/tinyxml.h"

template <typename T>
inline bool readAttribute(TiXmlElement* pElement, const String& pAttributeName, T& pAttributeValue, bool pRequired = true) {
	if (!pElement) {
		Log::error("null pElement in readRequiredAttribute");
		return false;
	}
	// Try to read attribute.
	const char* attribute = pElement->Attribute(pAttributeName.c_str());
	if (!attribute) {
		// Attribute required.
		if (pRequired) {
			Log::error("attribute not found in readRequiredAttribute");
			return false;
		}
		// Attribute not required.
		return true;
	}
	return Utility::stoSafe(pAttributeValue, String(attribute));
}

inline bool readAttribute(TiXmlElement* pElement, const String& pAttributeName, String& pAttributeValue, bool pRequired = true) {
	if (!pElement) {
		Log::error("null pElement in readRequiredAttribute");
		return false;
	}
	// Try to read attribute.
	const char* attribute = pElement->Attribute(pAttributeName.c_str());
	if (!attribute) {
		// Attribute required.
		if (pRequired) {
			Log::error("attribute not found in readRequiredAttribute");
			return false;
		}
		// Attribute not required.
		return true;
	}
	pAttributeValue = attribute;
	return true;
}

inline bool readAttribute(TiXmlElement* pElement, const String& pAttributeName, bool& pAttributeValue, bool pRequired = true) {
	if (!pElement) {
		Log::error("null pElement in readRequiredAttribute");
		return false;
	}
	// Try to read attribute.
	const char* attribute = pElement->Attribute(pAttributeName.c_str());
	if (!attribute) {
		// Attribute required.
		if (pRequired) {
			Log::error("attribute not found in readRequiredAttribute");
			return false;
		}
		// Attribute not required.
		return true;
	}
	pAttributeValue = String(attribute) == "1";
	return true;
}

bool DataStore::initialise() {

	return true;
}

namespace AccountDataXML {
#define SCA static const auto
	SCA FileLocation = "./data/accounts.xml";
	namespace Tag {
		SCA Accounts = "accounts";
		SCA Account = "account";
	}
	namespace Attribute {
		// Tag::Account
		SCA ID = "id";
		SCA Name = "name";
		SCA Status = "status";
		SCA SuspendedUntil = "suspend_until";
		SCA LastLogin = "last_login";
		SCA Created = "created";
	}
#undef SCA
}

bool DataStore::loadAccounts(std::list<AccountData*>& pAccounts) {
	using namespace AccountDataXML;
	Profile p("DataStore::loadAccounts");
	EXPECTED_BOOL(pAccounts.empty());
	TiXmlDocument document(AccountDataXML::FileLocation);
	EXPECTED_BOOL(document.LoadFile());

	auto accountsElement = document.FirstChildElement(Tag::Accounts);
	EXPECTED_BOOL(accountsElement);
	auto accountElement = accountsElement->FirstChildElement(Tag::Account);
	
	// There are no accounts yet.
	if (!accountElement)
		return true;

	// Iterate over each "account" element.
	while (accountElement) {
		auto accountData = new AccountData();
		pAccounts.push_back(accountData);

		EXPECTED_BOOL(readAttribute(accountElement, Attribute::ID, accountData->mAccountID));
		EXPECTED_BOOL(readAttribute(accountElement, Attribute::Name, accountData->mAccountName));
		EXPECTED_BOOL(Limits::LoginServer::accountNameLength(accountData->mAccountName));
		EXPECTED_BOOL(readAttribute(accountElement, Attribute::Status, accountData->mStatus));
		EXPECTED_BOOL(readAttribute(accountElement, Attribute::SuspendedUntil, accountData->mSuspendedUntil));
		EXPECTED_BOOL(readAttribute(accountElement, Attribute::Created, accountData->mCreated));
		
		accountElement = accountElement->NextSiblingElement(Tag::Account);
	}
	
	return true;
}

bool DataStore::saveAccounts(std::list<AccountData*>& pAccounts) {
	using namespace AccountDataXML;
	Profile p("DataStore::saveAccounts");
	TiXmlDocument document(FileLocation);

	auto accountsElement = new TiXmlElement(Tag::Accounts);
	document.LinkEndChild(accountsElement);

	for (auto i : pAccounts) {
		auto accountElement = new TiXmlElement(Tag::Account);
		accountsElement->LinkEndChild(accountElement);

		accountElement->SetAttribute(Attribute::ID, std::to_string(i->mAccountID).c_str());
		accountElement->SetAttribute(Attribute::Name, i->mAccountName.c_str());
		accountElement->SetAttribute(Attribute::Status, i->mStatus);
		accountElement->SetAttribute(Attribute::SuspendedUntil, std::to_string(i->mSuspendedUntil).c_str());
		accountElement->SetAttribute(Attribute::Created, std::to_string(i->mCreated).c_str());
	}

	EXPECTED_BOOL(document.SaveFile());
	return true;
}

namespace AccountCharacterDataXML {
#define SCA static const auto
	namespace Tag {
		SCA Account = "account";
		SCA Characters = "characters";
		SCA Character = "character";
		SCA Equipment = "equipment";
		SCA Slot = "slot";
	}
	namespace Attribute {
		// Tag::Account
		SCA SharedPlatinum = "shared_platinum";
		// Tag::Character
		SCA Name = "name";
		SCA Race = "race";
		SCA Class = "class";
		SCA Level = "level";
		SCA Gender = "gender";
		SCA Deity = "deity";
		SCA Zone = "zone";
		SCA FaceStyle = "face";
		SCA HairStyle = "hair_style";
		SCA HairColour = "hair_colour";
		SCA BeardStyle = "beard_style";
		SCA BeardColour = "beard_colour";
		SCA LeftEyeColour = "eye_colour1";
		SCA RightEyeColour = "eye_colour2";
		SCA DrakkinHeritage = "drakkin_heritage";
		SCA DrakkinTattoo = "drakkin_tattoo";
		SCA DrakkinDetails = "drakkin_details";
		// Tag::Equipment
		SCA Primary = "primary";
		SCA Secondary = "secondary";
		// Tag::Slot
		SCA Material = "material";
		SCA Colour = "colour";
	}
#undef SCA
}

bool DataStore::loadAccountCharacterData(AccountData* pAccount) {
	using namespace AccountCharacterDataXML;
	Profile p("DataStore::loadAccountCharacterData");
	EXPECTED_BOOL(pAccount);
	TiXmlDocument document(String("./data/accounts/" + pAccount->mAccountName + ".xml").c_str());
	EXPECTED_BOOL(document.LoadFile());

	auto accountElement = document.FirstChildElement(Tag::Account);
	EXPECTED_BOOL(accountElement);
	EXPECTED_BOOL(readAttribute(accountElement, Attribute::SharedPlatinum, pAccount->mPlatinumSharedBank));
	auto charactersElement = accountElement->FirstChildElement(Tag::Characters);
	EXPECTED_BOOL(charactersElement);
	auto characterElement = charactersElement->FirstChildElement(Tag::Character);

	// There are no characters yet.
	if (!characterElement)
		return true;

	// Iterate over each "account" element.
	auto characterSlot = 0;
	while (characterElement && characterSlot < Limits::Account::MAX_NUM_CHARACTERS) {
		auto characterData = new AccountData::CharacterData();
		pAccount->mCharacterData.push_back(characterData);

		// Read the basic/visual information about each character.
		EXPECTED_BOOL(readAttribute(characterElement, Attribute::Name, characterData->mName));
		EXPECTED_BOOL(readAttribute(characterElement, Attribute::Race, characterData->mRace));
		EXPECTED_BOOL(readAttribute(characterElement, Attribute::Class, characterData->mClass));
		EXPECTED_BOOL(readAttribute(characterElement, Attribute::Level, characterData->mLevel));
		EXPECTED_BOOL(readAttribute(characterElement, Attribute::Gender, characterData->mGender));
		EXPECTED_BOOL(readAttribute(characterElement, Attribute::Deity, characterData->mDeity));
		EXPECTED_BOOL(readAttribute(characterElement, Attribute::Zone, characterData->mZoneID));
		EXPECTED_BOOL(readAttribute(characterElement, Attribute::FaceStyle, characterData->mFaceStyle));
		EXPECTED_BOOL(readAttribute(characterElement, Attribute::HairStyle, characterData->mHairStyle));
		EXPECTED_BOOL(readAttribute(characterElement, Attribute::HairColour, characterData->mHairColour));
		EXPECTED_BOOL(readAttribute(characterElement, Attribute::BeardStyle, characterData->mBeardStyle));
		EXPECTED_BOOL(readAttribute(characterElement, Attribute::BeardColour, characterData->mBeardColour));
		EXPECTED_BOOL(readAttribute(characterElement, Attribute::LeftEyeColour, characterData->mEyeColourLeft));
		EXPECTED_BOOL(readAttribute(characterElement, Attribute::RightEyeColour, characterData->mEyeColourRight));
		EXPECTED_BOOL(readAttribute(characterElement, Attribute::DrakkinHeritage, characterData->mDrakkinHeritage));
		EXPECTED_BOOL(readAttribute(characterElement, Attribute::DrakkinTattoo, characterData->mDrakkinTattoo));
		EXPECTED_BOOL(readAttribute(characterElement, Attribute::DrakkinDetails, characterData->mDrakkinDetails));

		// Read the equipment information about each character.
		auto equipmentElement = characterElement->FirstChildElement(Tag::Equipment);
		EXPECTED_BOOL(equipmentElement);
		EXPECTED_BOOL(readAttribute(equipmentElement, Attribute::Primary, characterData->mPrimary)); // IDFile of item in primary slot.
		EXPECTED_BOOL(readAttribute(equipmentElement, Attribute::Secondary, characterData->mSecondary)); // IDFile of item in secondary slot.

		auto slotElement = equipmentElement->FirstChildElement(Tag::Slot);
		auto slotCount = 0;
		EXPECTED_BOOL(slotElement);
		while (slotElement && slotCount < Limits::Account::MAX_EQUIPMENT_SLOTS) {
			EXPECTED_BOOL(readAttribute(slotElement, Attribute::Material, characterData->mEquipment[slotCount].mMaterial));
			EXPECTED_BOOL(readAttribute(slotElement, Attribute::Colour, characterData->mEquipment[slotCount].mColour));
			slotCount++;
			slotElement = slotElement->NextSiblingElement(Tag::Slot);
		}
		EXPECTED_BOOL(slotCount == Limits::Account::MAX_EQUIPMENT_SLOTS); // Check that we read the correct amount of slot data.

		characterSlot++;
		characterElement = characterElement->NextSiblingElement(Tag::Character);
	}

	return true;
}

bool DataStore::saveAccountCharacterData(AccountData* pAccount) {
	using namespace AccountCharacterDataXML;
	Profile p("DataStore::saveAccountCharacterData");
	EXPECTED_BOOL(pAccount);
	TiXmlDocument document(String("./data/accounts/" + pAccount->mAccountName + ".xml").c_str());

	auto accountElement = static_cast<TiXmlElement*>(document.LinkEndChild(new TiXmlElement(Tag::Account)));
	accountElement->SetAttribute(Attribute::SharedPlatinum, pAccount->mPlatinumSharedBank);

	auto charactersElement = accountElement->LinkEndChild(new TiXmlElement(Tag::Characters));
	
	// Write 
	for (auto i : pAccount->mCharacterData) {
		auto characterElement = static_cast<TiXmlElement*>(charactersElement->LinkEndChild(new TiXmlElement(Tag::Character)));

		characterElement->SetAttribute(Attribute::Name, i->mName.c_str());
		characterElement->SetAttribute(Attribute::Race, i->mRace);
		characterElement->SetAttribute(Attribute::Class, i->mClass);
		characterElement->SetAttribute(Attribute::Level, i->mLevel);
		characterElement->SetAttribute(Attribute::Gender, i->mGender);
		characterElement->SetAttribute(Attribute::Deity, i->mDeity);
		characterElement->SetAttribute(Attribute::Zone, i->mZoneID);
		characterElement->SetAttribute(Attribute::FaceStyle, i->mFaceStyle);
		characterElement->SetAttribute(Attribute::Gender, i->mGender);
		characterElement->SetAttribute(Attribute::HairStyle, i->mHairStyle);
		characterElement->SetAttribute(Attribute::HairColour, i->mHairColour);
		characterElement->SetAttribute(Attribute::BeardStyle, i->mBeardStyle);
		characterElement->SetAttribute(Attribute::BeardColour, i->mBeardColour);
		characterElement->SetAttribute(Attribute::LeftEyeColour, i->mEyeColourLeft);
		characterElement->SetAttribute(Attribute::RightEyeColour, i->mEyeColourRight);
		characterElement->SetAttribute(Attribute::DrakkinHeritage, i->mDrakkinHeritage);
		characterElement->SetAttribute(Attribute::DrakkinTattoo, i->mDrakkinTattoo);
		characterElement->SetAttribute(Attribute::DrakkinDetails, i->mDrakkinDetails);

		auto equipmentElement = static_cast<TiXmlElement*>(characterElement->LinkEndChild(new TiXmlElement(Tag::Equipment)));
		equipmentElement->SetAttribute(Attribute::Primary, i->mPrimary);
		equipmentElement->SetAttribute(Attribute::Secondary, i->mSecondary);

		// Write equipment material / colours.
		for (int j = 0; j < Limits::Account::MAX_EQUIPMENT_SLOTS; j++) {
			auto slotElement = static_cast<TiXmlElement*>(equipmentElement->LinkEndChild(new TiXmlElement(Tag::Slot)));
			slotElement->SetAttribute(Attribute::Material, i->mEquipment[j].mMaterial);
			slotElement->SetAttribute(Attribute::Colour, i->mEquipment[j].mColour);
		}
	}

	EXPECTED_BOOL(document.SaveFile());
	return true;
}

namespace CharacterDataXML {
#define SCA static const auto
	namespace Tag {
		SCA Character = "character";
		SCA Stats = "stats";
		SCA Visual = "visual";
		SCA Dyes = "dyes";
		SCA Dye = "dye";
		SCA Guild = "guild";
		SCA Currency = "currency";
		SCA Crystals = "crystals";
		SCA Radiant = "radiant";
		SCA Ebon = "ebon";
		SCA SpellBook = "spellbook";
		SCA SpellBookSlot = "sb_slot";
		SCA SpellBar = "spellbar";
		SCA SpellBarSlot = "sb_slot";
		SCA Skills = "skills";
		SCA Skill = "skill";
		SCA Lanaguages = "languages";
		SCA Language = "language";
	}
	namespace Attribute {
		// Tag::Character
		SCA Name = "name";
		SCA GM = "gm";
		SCA Status = "status";
		SCA Level = "level";
		SCA Class = "class";
		SCA Zone = "zone";
		SCA X = "x";
		SCA Y = "y";
		SCA Z = "z";
		SCA Heading = "heading";
		SCA Experience = "experience";
		SCA LastName = "last_name";
		SCA Title = "title";
		SCA Suffix = "suffix";
		SCA AutoConsentGroup = "auto_consent_group";
		SCA AutoConsentRaid = "auto_consent_raid";
		SCA AutoConsentGuild = "auto_consent_guild";
		// Tag::Stats
		SCA Strength = "strength";
		SCA Stamina = "stamina";
		SCA Charisma = "charisma";
		SCA Dexterity = "dexterity";
		SCA Intelligence = "intelligence";
		SCA Agility = "agility";
		SCA Wisdom = "wisdom";
		// Tag::Visual
		SCA Race = "race";
		SCA Gender = "gender";
		SCA Face = "face";
		SCA HairStyle = "hair_style";
		SCA HairColour = "hair_colour";
		SCA BeardStyle = "beard_style";
		SCA BeardColour = "beard_colour";
		SCA EyeColour1 = "eye_colour1";
		SCA EyeColour2 = "eye_colour2";
		SCA DrakkinHeritage = "drakkin_heritage";
		SCA DrakkinTattoo = "drakkin_tattoo";
		SCA DrakkinDetails = "drakkin_Details";
		// Tag::Dye
		SCA Colour = "colour";
		// Tag::Guild
		SCA GuildID = "id";
		SCA GuildRank = "rank";
		// Tag::Currency
		SCA PlatinumCharacter = "platinum_character";
		SCA PlatinumBank = "platinum_bank";
		SCA PlatinumCursor = "platinum_cursor";
		SCA GoldCharacter = "gold_character";
		SCA GoldBank = "gold_bank";
		SCA GoldCursor = "gold_cursor";
		SCA SilverCharacter = "silver_character";
		SCA SilverBank = "silver_bank";
		SCA SilverCursor = "silver_cursor";
		SCA CopperCharacter = "copper_character";
		SCA CopperBank = "copper_bank";
		SCA CopperCursor = "copper_cursor";
		// Tag::Crystals / Radiant / Ebon
		SCA Current = "current";
		SCA Total = "total";
		// Tag::SpellBook
		SCA SpellBookSlot = "slot";
		SCA SpellBookSpell = "id";
		// Tag::SpellBar
		SCA SpellBarSlot = "slot";
		SCA SpellBarSpell = "id";
		// Tag::Skill
		SCA SkillID = "id";
		SCA SkillValue = "value";
		// Tag::Language
		SCA LanguageID = "id";
		SCA LanguageValue = "value";
	}
#undef SCA
}

const bool DataStore::loadCharacter(const String& pCharacterName, CharacterData* pCharacterData) {
	Profile p("DataStore::loadCharacter");
	using namespace CharacterDataXML;
	EXPECTED_BOOL(pCharacterData);
	TiXmlDocument document(String("./data/characters/" + pCharacterName + ".xml").c_str());
	EXPECTED_BOOL(document.LoadFile());

	// Tag::Character
	auto characterElement = document.FirstChildElement(Tag::Character);
	EXPECTED_BOOL(characterElement);
	EXPECTED_BOOL(readAttribute(characterElement, Attribute::Name, pCharacterData->mName));
	EXPECTED_BOOL(readAttribute(characterElement, Attribute::GM, pCharacterData->mGM));
	EXPECTED_BOOL(readAttribute(characterElement, Attribute::Status, pCharacterData->mStatus));
	EXPECTED_BOOL(readAttribute(characterElement, Attribute::Level, pCharacterData->mLevel));
	EXPECTED_BOOL(readAttribute(characterElement, Attribute::Class, pCharacterData->mClass));
	EXPECTED_BOOL(readAttribute(characterElement, Attribute::Zone, pCharacterData->mZoneID));
	EXPECTED_BOOL(readAttribute(characterElement, Attribute::X, pCharacterData->mX));
	EXPECTED_BOOL(readAttribute(characterElement, Attribute::Y, pCharacterData->mY));
	EXPECTED_BOOL(readAttribute(characterElement, Attribute::Z, pCharacterData->mZ));
	EXPECTED_BOOL(readAttribute(characterElement, Attribute::Heading, pCharacterData->mHeading));
	EXPECTED_BOOL(readAttribute(characterElement, Attribute::Experience, pCharacterData->mExperience));
	EXPECTED_BOOL(readAttribute(characterElement, Attribute::LastName, pCharacterData->mLastName));
	EXPECTED_BOOL(readAttribute(characterElement, Attribute::Title, pCharacterData->mTitle));
	EXPECTED_BOOL(readAttribute(characterElement, Attribute::Suffix, pCharacterData->mSuffix));
	EXPECTED_BOOL(readAttribute(characterElement, Attribute::AutoConsentGroup, pCharacterData->mAutoConsentGroup));
	EXPECTED_BOOL(readAttribute(characterElement, Attribute::AutoConsentRaid, pCharacterData->mAutoConsentRaid));
	EXPECTED_BOOL(readAttribute(characterElement, Attribute::AutoConsentGuild, pCharacterData->mAutoConsentGuild));

	// Tag::Stats
	auto statsElement = characterElement->FirstChildElement(Tag::Stats);
	EXPECTED_BOOL(statsElement);
	EXPECTED_BOOL(readAttribute(statsElement, Attribute::Strength, pCharacterData->mStrength));
	EXPECTED_BOOL(readAttribute(statsElement, Attribute::Stamina, pCharacterData->mStamina));
	EXPECTED_BOOL(readAttribute(statsElement, Attribute::Charisma, pCharacterData->mCharisma));
	EXPECTED_BOOL(readAttribute(statsElement, Attribute::Intelligence, pCharacterData->mIntelligence));
	EXPECTED_BOOL(readAttribute(statsElement, Attribute::Agility, pCharacterData->mAgility));
	EXPECTED_BOOL(readAttribute(statsElement, Attribute::Wisdom, pCharacterData->mWisdom));

	// Tag::Visual
	auto visualElement = characterElement->FirstChildElement(Tag::Visual);
	EXPECTED_BOOL(visualElement);
	EXPECTED_BOOL(readAttribute(visualElement, Attribute::Race, pCharacterData->mRace));
	EXPECTED_BOOL(readAttribute(visualElement, Attribute::Gender, pCharacterData->mGender));
	EXPECTED_BOOL(readAttribute(visualElement, Attribute::Face, pCharacterData->mFaceStyle));
	EXPECTED_BOOL(readAttribute(visualElement, Attribute::HairStyle, pCharacterData->mHairStyle));
	EXPECTED_BOOL(readAttribute(visualElement, Attribute::HairColour, pCharacterData->mHairColour));
	EXPECTED_BOOL(readAttribute(visualElement, Attribute::BeardStyle, pCharacterData->mBeardStyle));
	EXPECTED_BOOL(readAttribute(visualElement, Attribute::BeardColour, pCharacterData->mBeardColour));
	EXPECTED_BOOL(readAttribute(visualElement, Attribute::EyeColour1, pCharacterData->mEyeColourLeft));
	EXPECTED_BOOL(readAttribute(visualElement, Attribute::EyeColour2, pCharacterData->mEyeColourRight));
	EXPECTED_BOOL(readAttribute(visualElement, Attribute::DrakkinHeritage, pCharacterData->mDrakkinHeritage));
	EXPECTED_BOOL(readAttribute(visualElement, Attribute::DrakkinTattoo, pCharacterData->mDrakkinTattoo));

	// Tag::Dyes
	auto dyesElement = characterElement->FirstChildElement(Tag::Dyes);
	EXPECTED_BOOL(dyesElement);
	int slotID = 0;
	auto dyeElement = dyesElement->FirstChildElement(Tag::Dye);
	while (dyeElement) {
		EXPECTED_BOOL(readAttribute(dyeElement, Attribute::Colour, pCharacterData->mDyes[slotID]));
		slotID++;
		dyeElement = dyeElement->NextSiblingElement(Tag::Dye);
	}
	EXPECTED_BOOL(slotID == MAX_ARMOR_DYE_SLOTS); // Check all 7 slots were read.

	// Tag::Skills
	auto skillsElement = characterElement->FirstChildElement(Tag::Skills);
	EXPECTED_BOOL(skillsElement);
	auto skillElement = skillsElement->FirstChildElement(Tag::Skill);
	while (skillElement) {
		// Tag::Skill
		uint32 skillID = 0;
		uint32 skillValue = 0;
		EXPECTED_BOOL(readAttribute(skillElement, Attribute::SkillID, skillID));
		EXPECTED_BOOL(readAttribute(skillElement, Attribute::SkillValue, skillValue));
		EXPECTED_BOOL(Limits::Skills::validID(skillID));
		pCharacterData->mSkills[skillID] = skillValue;

		skillElement = skillElement->NextSiblingElement(Tag::Skill);
	}

	// Tag::Languages
	auto languagesElement = characterElement->FirstChildElement(Tag::Lanaguages);
	EXPECTED_BOOL(languagesElement);
	auto languageElement = languagesElement->FirstChildElement(Tag::Language);
	while (languageElement) {
		// Tag::Language
		uint32 languageID = 0;
		uint32 languageValue = 0;
		EXPECTED_BOOL(readAttribute(languageElement, Attribute::LanguageID, languageID));
		EXPECTED_BOOL(readAttribute(languageElement, Attribute::LanguageValue, languageValue));
		EXPECTED_BOOL(Limits::Languages::validID(languageID));
		pCharacterData->mLanguages[languageID] = languageValue;

		languageElement = languageElement->NextSiblingElement(Tag::Language);
	}
	
	// Tag::Guild
	auto guildElement = characterElement->FirstChildElement(Tag::Guild);
	EXPECTED_BOOL(guildElement);
	EXPECTED_BOOL(readAttribute(guildElement, Attribute::GuildID, pCharacterData->mGuildID));
	EXPECTED_BOOL(readAttribute(guildElement, Attribute::GuildRank, pCharacterData->mGuildRank));

	// Tag::Currency
	auto currencyElement = characterElement->FirstChildElement(Tag::Currency);
	EXPECTED_BOOL(currencyElement);
	EXPECTED_BOOL(readAttribute(currencyElement, Attribute::PlatinumCharacter, pCharacterData->mPlatinumCharacter));
	EXPECTED_BOOL(readAttribute(currencyElement, Attribute::PlatinumBank, pCharacterData->mPlatinumBank));
	EXPECTED_BOOL(readAttribute(currencyElement, Attribute::PlatinumCursor, pCharacterData->mPlatinumCursor));
	EXPECTED_BOOL(readAttribute(currencyElement, Attribute::GoldCharacter, pCharacterData->mGoldCharacter));
	EXPECTED_BOOL(readAttribute(currencyElement, Attribute::GoldBank, pCharacterData->mGoldBank));
	EXPECTED_BOOL(readAttribute(currencyElement, Attribute::GoldCursor, pCharacterData->mGoldCursor));
	EXPECTED_BOOL(readAttribute(currencyElement, Attribute::SilverCharacter, pCharacterData->mSilverCharacter));
	EXPECTED_BOOL(readAttribute(currencyElement, Attribute::SilverBank, pCharacterData->mSilverBank));
	EXPECTED_BOOL(readAttribute(currencyElement, Attribute::SilverCursor, pCharacterData->mSilverCursor));
	EXPECTED_BOOL(readAttribute(currencyElement, Attribute::CopperCharacter, pCharacterData->mCopperCharacter));
	EXPECTED_BOOL(readAttribute(currencyElement, Attribute::CopperBank, pCharacterData->mCopperBank));
	EXPECTED_BOOL(readAttribute(currencyElement, Attribute::CopperCursor, pCharacterData->mCopperCursor));

	// Tag::Crystals
	auto crystalsElement = characterElement->FirstChildElement(Tag::Crystals);
	EXPECTED_BOOL(crystalsElement);
	// Tag::Radiant
	auto radiantElement = crystalsElement->FirstChildElement(Tag::Radiant);
	EXPECTED_BOOL(radiantElement);
	EXPECTED_BOOL(readAttribute(radiantElement, Attribute::Current, pCharacterData->mRadiantCrystals));
	EXPECTED_BOOL(readAttribute(radiantElement, Attribute::Total, pCharacterData->mTotalRadiantCrystals));
	// Tag::Ebon
	auto ebonElement = crystalsElement->FirstChildElement(Tag::Ebon);
	EXPECTED_BOOL(ebonElement);
	EXPECTED_BOOL(readAttribute(ebonElement, Attribute::Current, pCharacterData->mEbonCrystals));
	EXPECTED_BOOL(readAttribute(ebonElement, Attribute::Total, pCharacterData->mTotalEbonCrystals));

	// Caster Only
	if (Utility::isCaster(pCharacterData->mClass)) {
		// Tag::SpellBook
		{
			auto spellbookElement = characterElement->FirstChildElement(Tag::SpellBook);
			EXPECTED_BOOL(spellbookElement);
			auto slotElement = spellbookElement->FirstChildElement(Tag::SpellBookSlot);
			while (slotElement) {
				uint32 spellID = 0;
				uint32 slotID = 0;
				EXPECTED_BOOL(readAttribute(slotElement, Attribute::SpellBookSlot, slotID));
				EXPECTED_BOOL(readAttribute(slotElement, Attribute::SpellBookSpell, spellID));
				EXPECTED_BOOL(Limits::SpellBook::slotValid(slotID));
				EXPECTED_BOOL(Limits::SpellBook::spellIDValid(spellID));

				pCharacterData->mSpellBook[slotID] = spellID;

				slotElement = slotElement->NextSiblingElement(Tag::SpellBookSlot);
			}
		}
		{
			// Tag::SpellBar
			auto spellBarElement = characterElement->FirstChildElement(Tag::SpellBar);
			EXPECTED_BOOL(spellBarElement);
			auto slotElement = spellBarElement->FirstChildElement(Tag::SpellBarSlot);
			while (slotElement) {
				uint32 spellID = 0;
				uint32 slotID = 0;
				EXPECTED_BOOL(readAttribute(slotElement, Attribute::SpellBarSlot, slotID));
				EXPECTED_BOOL(readAttribute(slotElement, Attribute::SpellBarSpell, spellID));
				EXPECTED_BOOL(Limits::SpellBar::slotValid(slotID));
				EXPECTED_BOOL(Limits::SpellBar::spellIDValid(spellID));

				pCharacterData->mSpellBar[slotID] = spellID;

				slotElement = slotElement->NextSiblingElement(Tag::SpellBarSlot);
			}
		}
	}

	return true;
}

const bool DataStore::saveCharacter(const String& pCharacterName, const CharacterData* pCharacterData) {
	Profile p("DataStore::saveCharacter");
	using namespace CharacterDataXML;

	EXPECTED_BOOL(pCharacterData);
	TiXmlDocument document(String("./data/characters/" + pCharacterName + ".xml").c_str());

	// Tag::Character
	auto characterElement = static_cast<TiXmlElement*>(document.LinkEndChild(new TiXmlElement(Tag::Character)));
	characterElement->SetAttribute(Attribute::Name, pCharacterData->mName.c_str());
	characterElement->SetAttribute(Attribute::GM, pCharacterData->mGM);
	characterElement->SetAttribute(Attribute::Status, pCharacterData->mStatus);
	characterElement->SetAttribute(Attribute::Level, pCharacterData->mLevel);
	characterElement->SetAttribute(Attribute::Class, pCharacterData->mClass);
	characterElement->SetAttribute(Attribute::Zone, pCharacterData->mZoneID);
	characterElement->SetAttribute(Attribute::X, pCharacterData->mX);
	characterElement->SetAttribute(Attribute::Y, pCharacterData->mY);
	characterElement->SetAttribute(Attribute::Z, pCharacterData->mZ);
	characterElement->SetAttribute(Attribute::Heading, pCharacterData->mHeading);
	characterElement->SetAttribute(Attribute::Experience, pCharacterData->mExperience);
	characterElement->SetAttribute(Attribute::LastName, pCharacterData->mLastName.c_str());
	characterElement->SetAttribute(Attribute::Title, pCharacterData->mTitle.c_str());
	characterElement->SetAttribute(Attribute::Suffix, pCharacterData->mSuffix.c_str());
	characterElement->SetAttribute(Attribute::AutoConsentGroup, pCharacterData->mAutoConsentGroup);
	characterElement->SetAttribute(Attribute::AutoConsentRaid, pCharacterData->mAutoConsentRaid);
	characterElement->SetAttribute(Attribute::AutoConsentGuild, pCharacterData->mAutoConsentGuild);

	// Tag::Stats
	auto statsElement = static_cast<TiXmlElement*>(characterElement->LinkEndChild(new TiXmlElement(Tag::Stats)));
	statsElement->SetAttribute(Attribute::Strength, pCharacterData->mStrength);
	statsElement->SetAttribute(Attribute::Stamina, pCharacterData->mStamina);
	statsElement->SetAttribute(Attribute::Charisma, pCharacterData->mCharisma);
	statsElement->SetAttribute(Attribute::Intelligence, pCharacterData->mIntelligence);
	statsElement->SetAttribute(Attribute::Agility, pCharacterData->mAgility);
	statsElement->SetAttribute(Attribute::Wisdom, pCharacterData->mWisdom);

	// Tag::Visual
	auto visualElement = static_cast<TiXmlElement*>(characterElement->LinkEndChild(new TiXmlElement(Tag::Visual)));
	visualElement->SetAttribute(Attribute::Race, pCharacterData->mRace);
	visualElement->SetAttribute(Attribute::Gender, pCharacterData->mGender);
	visualElement->SetAttribute(Attribute::Face, pCharacterData->mFaceStyle);
	visualElement->SetAttribute(Attribute::HairStyle, pCharacterData->mHairStyle);
	visualElement->SetAttribute(Attribute::HairColour, pCharacterData->mHairColour);
	visualElement->SetAttribute(Attribute::BeardStyle, pCharacterData->mBeardStyle);
	visualElement->SetAttribute(Attribute::BeardColour, pCharacterData->mBeardColour);
	visualElement->SetAttribute(Attribute::EyeColour1, pCharacterData->mEyeColourLeft);
	visualElement->SetAttribute(Attribute::EyeColour2, pCharacterData->mEyeColourRight);
	visualElement->SetAttribute(Attribute::DrakkinHeritage, pCharacterData->mDrakkinHeritage);
	visualElement->SetAttribute(Attribute::DrakkinTattoo, pCharacterData->mDrakkinTattoo);

	// Tag::Dyes
	auto dyesElement = static_cast<TiXmlElement*>(characterElement->LinkEndChild(new TiXmlElement(Tag::Dyes)));
	for (int i = 0; i < MAX_ARMOR_DYE_SLOTS; i++) {
		// Tag::Dye
		auto dyeElement = static_cast<TiXmlElement*>(dyesElement->LinkEndChild(new TiXmlElement(Tag::Dye)));
		dyeElement->SetAttribute(Attribute::Colour, pCharacterData->mDyes[i]);
	}

	// Tag::Skills
	auto skillsElement = static_cast<TiXmlElement*>(characterElement->LinkEndChild(new TiXmlElement(Tag::Skills)));
	for (int i = 0; i < Limits::Skills::MAX_ID; i++) {
		if (pCharacterData->mSkills[i] == 0) continue;
		// Tag::Skill
		auto skillElement = static_cast<TiXmlElement*>(skillsElement->LinkEndChild(new TiXmlElement(Tag::Skill)));
		skillElement->SetAttribute(Attribute::SkillID, i);
		skillElement->SetAttribute(Attribute::SkillValue, pCharacterData->mSkills[i]);
	}

	// Tag::Languages
	auto LanguagesElement = static_cast<TiXmlElement*>(characterElement->LinkEndChild(new TiXmlElement(Tag::Lanaguages)));
	for (int i = 0; i < Limits::Languages::MAX_ID; i++) {
		if (pCharacterData->mLanguages[i] == 0) continue;
		// Tag::Language
		auto languageElement = static_cast<TiXmlElement*>(LanguagesElement->LinkEndChild(new TiXmlElement(Tag::Language)));
		languageElement->SetAttribute(Attribute::LanguageID, i);
		languageElement->SetAttribute(Attribute::LanguageValue, pCharacterData->mLanguages[i]);
	}

	// Tag::Guild
	auto guildElement = static_cast<TiXmlElement*>(characterElement->LinkEndChild(new TiXmlElement(Tag::Guild)));
	guildElement->SetAttribute(Attribute::GuildID, pCharacterData->mGuildID);
	guildElement->SetAttribute(Attribute::GuildRank, pCharacterData->mGuildRank);

	// Tag::Currency
	auto currencyElement = static_cast<TiXmlElement*>(characterElement->LinkEndChild(new TiXmlElement(Tag::Currency)));
	currencyElement->SetAttribute(Attribute::PlatinumCharacter, pCharacterData->mPlatinumCharacter);
	currencyElement->SetAttribute(Attribute::PlatinumBank, pCharacterData->mPlatinumBank);
	currencyElement->SetAttribute(Attribute::PlatinumCursor, pCharacterData->mPlatinumCursor);
	currencyElement->SetAttribute(Attribute::GoldCharacter, pCharacterData->mGoldCharacter);
	currencyElement->SetAttribute(Attribute::GoldBank, pCharacterData->mGoldBank);
	currencyElement->SetAttribute(Attribute::GoldCursor, pCharacterData->mGoldCursor);
	currencyElement->SetAttribute(Attribute::SilverCharacter, pCharacterData->mSilverCharacter);
	currencyElement->SetAttribute(Attribute::SilverBank, pCharacterData->mSilverBank);
	currencyElement->SetAttribute(Attribute::SilverCursor, pCharacterData->mSilverCursor);
	currencyElement->SetAttribute(Attribute::CopperCharacter, pCharacterData->mCopperCharacter);
	currencyElement->SetAttribute(Attribute::CopperBank, pCharacterData->mCopperBank);
	currencyElement->SetAttribute(Attribute::CopperCursor, pCharacterData->mCopperCursor);

	// Tag::Crystals
	auto crystalsElement = static_cast<TiXmlElement*>(characterElement->LinkEndChild(new TiXmlElement(Tag::Crystals)));
	// Tag::Radiant
	auto radiantElement = static_cast<TiXmlElement*>(crystalsElement->LinkEndChild(new TiXmlElement(Tag::Radiant)));
	radiantElement->SetAttribute(Attribute::Current, pCharacterData->mRadiantCrystals);
	radiantElement->SetAttribute(Attribute::Total, pCharacterData->mTotalRadiantCrystals);
	// Tag::Ebon
	auto ebonElement = static_cast<TiXmlElement*>(crystalsElement->LinkEndChild(new TiXmlElement(Tag::Ebon)));
	ebonElement->SetAttribute(Attribute::Current, pCharacterData->mEbonCrystals);
	ebonElement->SetAttribute(Attribute::Total, pCharacterData->mTotalEbonCrystals);

	// Caster Only
	if (Utility::isCaster(pCharacterData->mClass)){
		// Tag::SpellBook
		auto spellbookElement = static_cast<TiXmlElement*>(characterElement->LinkEndChild(new TiXmlElement(Tag::SpellBook)));
		for (auto i = 0; i < Limits::SpellBook::MAX_SLOTS; i++) {
			if (pCharacterData->mSpellBook[i] == 0)
				continue;
			
			auto slotElement = static_cast<TiXmlElement*>(spellbookElement->LinkEndChild(new TiXmlElement(Tag::SpellBookSlot)));
			slotElement->SetAttribute(Attribute::SpellBookSlot, i);
			slotElement->SetAttribute(Attribute::SpellBookSpell, pCharacterData->mSpellBook[i]);
		}
		// Tag::SpellBar
		auto spellbarElement = static_cast<TiXmlElement*>(characterElement->LinkEndChild(new TiXmlElement(Tag::SpellBar)));
		for (auto i = 0; i < Limits::SpellBar::MAX_SLOTS; i++) {
			if (pCharacterData->mSpellBar[i] == 0)
				continue;

			auto slotElement = static_cast<TiXmlElement*>(spellbarElement->LinkEndChild(new TiXmlElement(Tag::SpellBarSlot)));
			slotElement->SetAttribute(Attribute::SpellBarSlot, i);
			slotElement->SetAttribute(Attribute::SpellBarSpell, pCharacterData->mSpellBar[i]);
		}
	}

	EXPECTED_BOOL(document.SaveFile());
	return true;
}

namespace SettingsDataXML {
#define SCA static const auto
	SCA FileLocation = "./data/settings.xml";
	namespace Tag {
		SCA Settings = "settings";
		SCA World = "world";
		SCA LoginServer = "login_server";
		SCA UCS = "ucs";
	}
	namespace Attribute {
		// Tag::Server
		SCA ShortName = "short_name";
		SCA LongName = "long_name";
		SCA Locked = "locked";
		// Tag::LoginServer
		SCA AccountName = "account_name";
		SCA Password = "password";
		SCA Address = "address";
		SCA Port = "port"; // Tag::UCS uses.
	}
#undef SCA
}

bool DataStore::loadSettings() {
	using namespace SettingsDataXML;
	TiXmlDocument document(SettingsDataXML::FileLocation);
	EXPECTED_BOOL(document.LoadFile());

	// Tag::Settings
	auto settingsElement = document.FirstChildElement(Tag::Settings);
	EXPECTED_BOOL(settingsElement);

	// Tag::Server
	auto worldElement = settingsElement->FirstChildElement(Tag::World);
	EXPECTED_BOOL(worldElement);

	String shortName = "";
	EXPECTED_BOOL(readAttribute(worldElement, Attribute::ShortName, shortName));
	EXPECTED_BOOL(Settings::_setServerShortName(shortName));

	String longName = "";
	EXPECTED_BOOL(readAttribute(worldElement, Attribute::LongName, longName));
	EXPECTED_BOOL(Settings::_setServerLongName(longName));

	bool locked = false;
	EXPECTED_BOOL(readAttribute(worldElement, Attribute::Locked, locked));
	EXPECTED_BOOL(Settings::_setLocked(locked));

	// Tag::LoginServer
	auto loginServerElement = settingsElement->FirstChildElement(Tag::LoginServer);
	EXPECTED_BOOL(loginServerElement);

	String lsAccountName = "";
	EXPECTED_BOOL(readAttribute(loginServerElement, Attribute::AccountName, lsAccountName));
	EXPECTED_BOOL(Settings::_setLSAccountName(lsAccountName));

	String lsPassword = "";
	EXPECTED_BOOL(readAttribute(loginServerElement, Attribute::Password, lsPassword));
	EXPECTED_BOOL(Settings::_setLSPassword(lsPassword));

	String lsAddress = "";
	EXPECTED_BOOL(readAttribute(loginServerElement, Attribute::Address, lsAddress));
	EXPECTED_BOOL(Settings::_setLSAddress(lsAddress));

	uint16 lsPort = 0;
	EXPECTED_BOOL(readAttribute(loginServerElement, Attribute::Port, lsPort));
	EXPECTED_BOOL(Settings::_setLSPort(lsPort));

	// Tag::UCS
	auto ucsElement = settingsElement->FirstChildElement(Tag::UCS);
	EXPECTED_BOOL(ucsElement);

	uint16 ucsPort = 0;
	EXPECTED_BOOL(readAttribute(ucsElement, Attribute::Port, ucsPort));
	EXPECTED_BOOL(Settings::_setUCSPort(ucsPort));

	return true;
}

namespace NPCAppearanceDataXML {
#define SCA static const auto
	SCA FileLocation = "./data/npc/appearances.xml";
	namespace Tag {
		SCA Appearances = "appearances";
		SCA Appearance = "appearance";
	}
	namespace Attribute {
		// Tag::Appearance
		SCA ID = "id";
		SCA Parent = "parent";
		SCA Race = "race";
		SCA Gender = "gender";
		SCA BodyType = "body_type";
		SCA Size = "size";
		SCA FaceStyle = "face_style";
		SCA HairStyle = "hair_style";
		SCA BeardStyle = "beard_style";
		SCA HairColour = "hair_colour";
		SCA BeardColour = "beard_colour";
		SCA LeftEyeColour = "eye_colour_left";
		SCA RightEyeColour = "eye_colour_right";
		SCA DrakkinHeritage = "drakkin_heritage";
		SCA DrakkinTattoo = "drakkin_tattoo";
		SCA DrakkinDetails = "drakkin_details";
		SCA HelmTexture = "helm_texture";
	}
#undef SCA
}

bool DataStore::loadNPCAppearanceData(std::list<NPCAppearanceData*>& pAppearances) {
	using namespace NPCAppearanceDataXML;
	Profile p("DataStore::loadNPCAppearanceData");
	EXPECTED_BOOL(pAppearances.empty());
	TiXmlDocument document(NPCAppearanceDataXML::FileLocation);
	EXPECTED_BOOL(document.LoadFile());

	auto appearancesElement = document.FirstChildElement(Tag::Appearances);
	EXPECTED_BOOL(appearancesElement);
	auto appearanceElement = appearancesElement->FirstChildElement(Tag::Appearance);

	while (appearanceElement) {
		auto d = new NPCAppearanceData();
		pAppearances.push_back(d);

		// Required.
		EXPECTED_BOOL(readAttribute(appearanceElement, Attribute::ID, d->mID));
		EXPECTED_BOOL(readAttribute(appearanceElement, Attribute::Parent, d->mParentID));

		// Optional.
		EXPECTED_BOOL(readAttribute(appearanceElement, Attribute::Race, d->mRaceID, false));
		EXPECTED_BOOL(readAttribute(appearanceElement, Attribute::Gender, d->mGender, false));
		EXPECTED_BOOL(readAttribute(appearanceElement, Attribute::BodyType, d->mBodyType, false));
		EXPECTED_BOOL(readAttribute(appearanceElement, Attribute::Size, d->mSize, false));
		EXPECTED_BOOL(readAttribute(appearanceElement, Attribute::FaceStyle, d->mFaceStyle, false));
		EXPECTED_BOOL(readAttribute(appearanceElement, Attribute::HairStyle, d->mHairStyle, false));
		EXPECTED_BOOL(readAttribute(appearanceElement, Attribute::BeardStyle, d->mBeardStyle, false));
		EXPECTED_BOOL(readAttribute(appearanceElement, Attribute::HairColour, d->mHairColour, false));
		EXPECTED_BOOL(readAttribute(appearanceElement, Attribute::BeardColour, d->mBeardColour, false));
		EXPECTED_BOOL(readAttribute(appearanceElement, Attribute::LeftEyeColour, d->mEyeColourLeft, false));
		EXPECTED_BOOL(readAttribute(appearanceElement, Attribute::RightEyeColour, d->mEyeColourRight, false));
		EXPECTED_BOOL(readAttribute(appearanceElement, Attribute::RightEyeColour, d->mEyeColourRight, false));
		EXPECTED_BOOL(readAttribute(appearanceElement, Attribute::DrakkinHeritage, d->mDrakkinHeritage, false));
		EXPECTED_BOOL(readAttribute(appearanceElement, Attribute::DrakkinTattoo, d->mDrakkinTattoo, false));
		EXPECTED_BOOL(readAttribute(appearanceElement, Attribute::DrakkinDetails, d->mDrakkinDetails, false));
		EXPECTED_BOOL(readAttribute(appearanceElement, Attribute::HelmTexture, d->mHelmTexture, false));

		appearanceElement = appearanceElement->NextSiblingElement(Tag::Appearance);
	}

	return true;
}

const bool DataStore::deleteCharacter(const String& pCharacterName) {
	const String existingFile = "./data/characters/" + pCharacterName + ".xml";
	const String newFile = "./data/characters/deleted/" + std::to_string(Utility::Time::now()) + "_" + pCharacterName + ".xml";
	// Copy the character xml to the deleted directory.
	EXPECTED_BOOL(CopyFile(existingFile.c_str(), newFile.c_str(), true));
	// Delete the character file.
	EXPECTED_BOOL(DeleteFile(existingFile.c_str()));

	return true;
}

namespace SpellDataXML {
#define SCA static const auto
	SCA FileLocation = "./data/spells.xml";
	namespace Tag {
		SCA Spells = "spells";
		SCA Spell = "spell";
	}
	namespace Attribute {
		// Tag::Spell
		SCA ID = "id";
		SCA Name = "name";
	}
#undef SCA
}

const bool DataStore::loadSpells(SpellData* pSpellData, uint32& pNumSpellsLoaded) {
	using namespace SpellDataXML;
	EXPECTED_BOOL(pSpellData);
	Profile p("DataStore::loadSpells");

	pNumSpellsLoaded = 0;
	TiXmlDocument document(SpellDataXML::FileLocation);
	EXPECTED_BOOL(document.LoadFile());

	auto spellsElement = document.FirstChildElement(Tag::Spells);
	EXPECTED_BOOL(spellsElement);
	auto spellElement = spellsElement->FirstChildElement(Tag::Spell);

	while (spellElement) {
		auto spellID = 0;
		EXPECTED_BOOL(readAttribute(spellElement, Attribute::ID, spellID));
		EXPECTED_BOOL(spellID > 0 && spellID < Limits::Spells::MAX_SPELL_ID);
		SpellData* currentSpell = &pSpellData[spellID];
		currentSpell->mInUse = true;
		currentSpell->mID = spellID;

		spellElement = spellElement->NextSiblingElement(Tag::Spell);
		pNumSpellsLoaded++;
	}

	return true;
}

namespace ItemDataXML {
#define SCA static const auto
	SCA FileLocation = "./data/items.xml";
	namespace Tag {
		SCA Items = "items";
		SCA Item = "item";
	}
	namespace Attribute {
		// Tag::Item
		SCA ID = "id";
		SCA Name = "name";
		SCA Lore = "lore";
		SCA IDFile = "id_file";
		SCA ItemClass = "item_class";
		SCA Weight = "weight";
		SCA Temporary = "temporary";
		SCA NoDrop = "no_drop";
		SCA Attunable = "attunable";
		SCA Size = "size";
		SCA Slots = "slots";
		SCA Price = "price";
		SCA Icon = "icon";
		SCA Tradeskills = "tradeskills";
		SCA ColdResist = "cold_resist";
		SCA DiseaseResist = "disease_resist";
		SCA PoisonResist = "poison_resist";
		SCA MagicResist = "magic_resist";
		SCA FireResist = "fire_resist";
		SCA CorruptionResist = "corruption_resist";
		SCA Strength = "strength";
		SCA Stamina = "stamina";
		SCA Agility = "agility";
		SCA Dexterity = "dexterity";
		SCA Charisma = "charisma";
		SCA Intelligence = "intelligence";
		SCA Wisdom = "wisdom";
		SCA Health = "health";
		SCA Mana = "mana";
		SCA Endurance = "endurance";
		SCA ArmorClass = "armor_class";
		SCA HealthRegen = "health_regen";
		SCA ManaRegen = "mana_regen";
		SCA EnduranceRegen = "endurance_regen";
		SCA Classes = "classes";
		SCA Races = "races";
		SCA Deity = "deity";
		SCA SkillModAmount = "skill_mod_amount";
		SCA SkillMod = "skill_mod";
		SCA BaneRace = "bane_race";
		SCA BaneBodyType = "bane_body_type";
		SCA BaneRaceAmount = "bane_race_amount";
		SCA BaneBodyTypeAmount = "bane_body_type_amount";
		SCA Magic = "magic";
		SCA CastTime = "cast_time"; // Used for Food/Drink as well.
		SCA ReqLevel = "req_level";
		SCA RecLevel = "rec_level";
		SCA RecSkill = "rec_skill";
		SCA BardType = "bard_type";
		SCA BardValue = "bard_value";
		SCA Light = "light";
		SCA Delay = "delay";
		SCA ElementalDamageType = "elemental_damage_type";
		SCA ElementalDamageAmount = "elemental_damage_amount";
		SCA Range = "range";
		SCA Damage = "damage";
		SCA Colour = "colour";
		SCA ItemType = "item_type";
		SCA Material = "material";
		SCA EliteMaterial = "elite_material";
		SCA SellRate = "sell_rate";
		SCA CombatEffects = "combat_effects";
		SCA Shielding = "shielding";
		SCA StunResist = "stun_resist";
		SCA StrikeThrough = "strike_through";
		SCA SkillDamageMod = "skill_damage_mod";
		SCA SkillDamageAmount = "skill_damage_amount";
		SCA SpellShield = "spell_shield";
		SCA Avoidance = "avoidance";
		SCA Accuracy = "accuracy";
		SCA CharmFileID = "charm_file_id";
		SCA FactionMod0ID = "faction_mod_0_id";
		SCA FactionMod0Amount = "faction_mod_0_amount";
		SCA FactionMod1ID = "faction_mod_1_id";
		SCA FactionMod1Amount = "faction_mod_1_amount";
		SCA FactionMod2ID = "faction_mod_2_id";
		SCA FactionMod2Amount = "faction_mod_2_amount";
		SCA FactionMod3ID = "faction_mod_3_id";
		SCA FactionMod3Amount = "faction_mod_3_amount";
		SCA CharmFile = "charm_file";
		SCA AugmentType = "augment_type";
		SCA AugmentRestrict = "augment_restrict";



		SCA HeroicColdResist = "heroic_cold_resist";
		SCA HeroicDiseaseResist = "heroic_disease_resist";
		SCA HeroicPoisonResist = "heroic_poison_resist";
		SCA HeroicMagicResist = "heroic_magic_resist";
		SCA HeroicFireResist = "heroic_fire_resist";
		SCA HeroicCorruptionResist = "heroic_corruption_resist";

		SCA HeroicStrength = "heroic_strength";
		SCA HeroicStamina = "heroic_stamina";
		SCA HeroicAgility = "heroic_agility";
		SCA HeroicDexterity = "heroic_dexterity";
		SCA HeroicCharisma = "heroic_charisma";
		SCA HeroicIntelligence = "heroic_intelligence";
		SCA HeroicWisdom = "heroic_wisdom";
	}
#undef SCA
}

const bool DataStore::loadItems(ItemData* pItemData, uint32& pNumItemsLoaded) {
	using namespace ItemDataXML;
	EXPECTED_BOOL(pItemData);
	Profile p("DataStore::loadItems");

	pNumItemsLoaded = 0;
	TiXmlDocument document(ItemDataXML::FileLocation);
	EXPECTED_BOOL(document.LoadFile());
	return true;
}

namespace ZoneDataXML {
#define SCA static const auto
	SCA FileLocation = "./data/zones.xml";
	namespace Tag {
		SCA Zones = "zones";
		SCA Zone = "zone";
		SCA ZonePoints = "zone_points";
		SCA ZonePoint = "zone_point";
		SCA SpawnPoints = "spawn_points";
		SCA SpawnPoint = "spawn_point";
	}
	namespace Attribute {
		// Tag::Zone
		SCA ID = "id";
		SCA ShortName = "short_name";
		SCA LongName = "long_name";
		SCA SafeX = "safe_x";
		SCA SafeY = "safe_y";
		SCA SafeZ = "safe_z";
		// Tag::ZonePoint
		SCA ZPID = "id";
		SCA ZPX = "x";
		SCA ZPY = "y";
		SCA ZPZ = "z";
		SCA ZPDestZoneID = "dest_zone";
		SCA ZPDestInstanceID = "dest_instance";
		SCA ZPDestX = "dest_x";
		SCA ZPDestY = "dest_y";
		SCA ZPDestZ = "dest_z";
		SCA ZPDestHeading = "dest_heading";
		// Tag::SpawnPoint
		SCA SPType = "type";
		SCA SPX = "x";
		SCA SPY = "y";
		SCA SPZ = "z";
		SCA SPHeading = "heading";
		SCA SPRespawn = "respawn";
		SCA SPNPCType = "npc_type";
		SCA SPSpawnGroup = "spawn_group";
	}
#undef SCA
}
const bool DataStore::loadZoneData(std::list<ZoneData*>& pZoneData) {
	using namespace ZoneDataXML;
	EXPECTED_BOOL(pZoneData.empty());
	Profile p("DataStore::loadZoneData");

	TiXmlDocument document(ZoneDataXML::FileLocation);
	EXPECTED_BOOL(document.LoadFile());

	auto zonesElement = document.FirstChildElement(Tag::Zones);
	EXPECTED_BOOL(zonesElement);
	auto zoneElement = zonesElement->FirstChildElement(Tag::Zone);

	while (zoneElement) {
		ZoneData* zoneData = new ZoneData();
		pZoneData.push_back(zoneData);

		EXPECTED_BOOL(readAttribute(zoneElement, Attribute::ID, zoneData->mID));
		EXPECTED_BOOL(readAttribute(zoneElement, Attribute::ShortName, zoneData->mShortName));
		EXPECTED_BOOL(readAttribute(zoneElement, Attribute::LongName, zoneData->mLongName));
		EXPECTED_BOOL(readAttribute(zoneElement, Attribute::SafeX, zoneData->mSafeX));
		EXPECTED_BOOL(readAttribute(zoneElement, Attribute::SafeY, zoneData->mSafeY));
		EXPECTED_BOOL(readAttribute(zoneElement, Attribute::SafeZ, zoneData->mSafeZ));

		// Read ZonePoints.
		auto zonePointsElement = zoneElement->FirstChildElement(Tag::ZonePoints);
		EXPECTED_BOOL(zonePointsElement);
		auto zonePointElement = zonePointsElement->FirstChildElement(Tag::ZonePoint);
		while (zonePointElement) {
			auto zp = new ZonePointData();
			zoneData->mZonePoints.push_back(zp);

			EXPECTED_BOOL(readAttribute(zonePointElement, Attribute::ZPID, zp->mID));
			EXPECTED_BOOL(readAttribute(zonePointElement, Attribute::ZPX, zp->mPosition.x));
			EXPECTED_BOOL(readAttribute(zonePointElement, Attribute::ZPY, zp->mPosition.y));
			EXPECTED_BOOL(readAttribute(zonePointElement, Attribute::ZPZ, zp->mPosition.z));
			EXPECTED_BOOL(readAttribute(zonePointElement, Attribute::ZPDestZoneID, zp->mDestinationZoneID));
			EXPECTED_BOOL(readAttribute(zonePointElement, Attribute::ZPDestInstanceID, zp->mDestinationInstanceID));
			EXPECTED_BOOL(readAttribute(zonePointElement, Attribute::ZPDestX, zp->mDestinationPosition.x));
			EXPECTED_BOOL(readAttribute(zonePointElement, Attribute::ZPDestY, zp->mDestinationPosition.y));
			EXPECTED_BOOL(readAttribute(zonePointElement, Attribute::ZPDestZ, zp->mDestinationPosition.z));
			EXPECTED_BOOL(readAttribute(zonePointElement, Attribute::ZPDestHeading, zp->mDestinationHeading));


			zonePointElement = zonePointElement->NextSiblingElement(Tag::ZonePoint);
		}

		// Read SpawnPoints
		auto spawnPointsElement = zoneElement->FirstChildElement(Tag::SpawnPoints);
		EXPECTED_BOOL(spawnPointsElement);
		auto spawnPointElement = spawnPointsElement->FirstChildElement(Tag::SpawnPoint);
		while (spawnPointElement) {
			auto sp = new SpawnPointData();
			zoneData->mSpawnPoints.push_back(sp);

			// Required Attributes.
			EXPECTED_BOOL(readAttribute(spawnPointElement, Attribute::SPX, sp->mPosition.x));
			EXPECTED_BOOL(readAttribute(spawnPointElement, Attribute::SPY, sp->mPosition.y));
			EXPECTED_BOOL(readAttribute(spawnPointElement, Attribute::SPZ, sp->mPosition.z));
			EXPECTED_BOOL(readAttribute(spawnPointElement, Attribute::SPHeading, sp->mHeading));
			EXPECTED_BOOL(readAttribute(spawnPointElement, Attribute::SPType, sp->mType));

			// TODO: Respawn Time.
			switch (sp->mType) {
			case SpawnPointType::TRASH:
				break;
			case SpawnPointType::SPECIFIC:
				EXPECTED_BOOL(readAttribute(spawnPointElement, Attribute::SPNPCType, sp->mNPCType));
				break;
			case SpawnPointType::SPAWN_GROUP:
				EXPECTED_BOOL(readAttribute(spawnPointElement, Attribute::SPSpawnGroup, sp->mSpawnGroupID));
				break;
			default:
				Log::error("Unknown SpawnPointType");
				return false;
			}

			spawnPointElement = spawnPointElement->NextSiblingElement(Tag::SpawnPoint);
		}

		zoneElement = zoneElement->NextSiblingElement(Tag::Zone);
	}
	return true;
}
