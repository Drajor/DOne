#include "XMLDataStore.h"
#include "Data.h"
#include "Utility.h"
#include "Limits.h"
#include "Profile.h"
#include "Settings.h"
#include "ItemData.h"
#include "LogSystem.h"
#include "SpellContants.h"

#include <fstream>
#include <Windows.h>
#include "../common/tinyxml/tinyxml.h"

#include "Poco/Data/Session.h"
#include "Poco/Data/SQLite/Connector.h"
#include "Poco/Format.h"
#include "Poco/Exception.h"

static bool AttributeFound = true;
static bool AttributeNotFound = false;

#define PROFILE_XML_DS

template <typename T>
inline bool readAttribute(TiXmlElement* pElement, const String& pAttributeName, T& pAttributeValue, bool pRequired = true, bool& pFound = AttributeNotFound) {
	if (!pElement) {
		Log::error("null pElement in readRequiredAttribute");
		return false;
	}
	// Try to read attribute.
	const char* attribute = pElement->Attribute(pAttributeName.c_str());
	if (!attribute) {
		pFound = false;
		// Attribute required.
		if (pRequired) {
			Log::error("attribute not found in readRequiredAttribute");
			return false;
		}
		// Attribute not required.
		return true;
	}
	pFound = true;
	return Utility::stoSafe(pAttributeValue, String(attribute));
}

inline bool readAttribute(TiXmlElement* pElement, const String& pAttributeName, String& pAttributeValue, bool pRequired = true, bool& pFound = AttributeNotFound) {
	if (!pElement) {
		Log::error("null pElement in readRequiredAttribute");
		return false;
	}
	// Try to read attribute.
	const char* attribute = pElement->Attribute(pAttributeName.c_str());
	if (!attribute) {
		pFound = false;
		// Attribute required.
		if (pRequired) {
			Log::error("attribute not found in readRequiredAttribute");
			return false;
		}
		// Attribute not required.
		return true;
	}
	pFound = true;
	pAttributeValue = attribute;
	return true;
}

inline bool readAttribute(TiXmlElement* pElement, const String& pAttributeName, bool& pAttributeValue, bool pRequired = true, bool& pFound = AttributeNotFound) {
	if (!pElement) {
		Log::error("null pElement in readRequiredAttribute");
		return false;
	}
	// Try to read attribute.
	const char* attribute = pElement->Attribute(pAttributeName.c_str());
	if (!attribute) {
		pFound = false;
		// Attribute required.
		if (pRequired) {
			Log::error("attribute not found in readRequiredAttribute");
			return false;
		}
		// Attribute not required.
		return true;
	}
	pFound = true;
	pAttributeValue = String(attribute) == "1" || String(attribute) == "true";
	return true;
}

inline bool readVector3(TiXmlElement* pElement, Vector3& pVector) {
	EXPECTED_BOOL(pElement);
	EXPECTED_BOOL(readAttribute(pElement, "x", pVector.x));
	EXPECTED_BOOL(readAttribute(pElement, "y", pVector.y));
	EXPECTED_BOOL(readAttribute(pElement, "z", pVector.z));
	return true;
}

inline void writeVector3(TiXmlElement* pElement, const Vector3& pVector) {
	EXPECTED(pElement);
	pElement->SetDoubleAttribute("x", pVector.x);
	pElement->SetDoubleAttribute("y", pVector.y);
	pElement->SetDoubleAttribute("z", pVector.z);
}

XMLDataStore::~XMLDataStore() {
	if (mLog) {
		delete mLog;
		mLog = nullptr;
	}
}


const bool XMLDataStore::initialise(ILogFactory* pLogFactory) {
	if (!pLogFactory) return false;

	mLog = pLogFactory->make();
	mLog->setContext("[DataStore]");

	return true;
}

namespace AccountXML {
#define SCA static const auto
	SCA FileLocation = "./data/accounts.xml";
	namespace Tag {
		SCA Accounts = "accounts";
		SCA Account = "account";
	}
	namespace Attribute {
		// Tag::Account
		SCA LSServerID = "ls_id";
		SCA LSAccountID = "ls_account_id";
		SCA Name = "name";
		SCA Status = "status";
		SCA SuspendedUntil = "suspend_until";
		SCA LastLogin = "last_login";
		SCA Created = "created";
	}
#undef SCA
}

const bool XMLDataStore::loadAccounts(Data::AccountList pAccounts) {
	using namespace AccountXML;
#ifdef PROFILE_XML_DS
	Profile p(String(__FUNCTION__), mLog);
#endif
	if (!pAccounts.empty()) return false;

	// Load document.
	TiXmlDocument document(AccountXML::FileLocation);
	if (!document.LoadFile()) {
		mLog->error("Failed to load " + String(AccountXML::FileLocation));
		return false;
	}

	auto accountsElement = document.FirstChildElement(Tag::Accounts);
	EXPECTED_BOOL(accountsElement);
	auto accountElement = accountsElement->FirstChildElement(Tag::Account);
	
	// There are no accounts yet.
	if (!accountElement)
		return true;

	// Iterate over each "account" element.
	while (accountElement) {
		auto accountData = new Data::Account();
		pAccounts.push_back(accountData);

		EXPECTED_BOOL(readAttribute(accountElement, Attribute::LSServerID, accountData->mLoginServerID));
		EXPECTED_BOOL(readAttribute(accountElement, Attribute::LSAccountID, accountData->mLoginAccountID));
		EXPECTED_BOOL(readAttribute(accountElement, Attribute::Name, accountData->mLoginAccountName));
		EXPECTED_BOOL(Limits::LoginServer::accountNameLength(accountData->mLoginAccountName));
		EXPECTED_BOOL(readAttribute(accountElement, Attribute::Status, accountData->mStatus));
		EXPECTED_BOOL(readAttribute(accountElement, Attribute::SuspendedUntil, accountData->mSuspendedUntil));
		EXPECTED_BOOL(readAttribute(accountElement, Attribute::Created, accountData->mCreated));
		
		accountElement = accountElement->NextSiblingElement(Tag::Account);
	}
	
	return true;
}

const bool XMLDataStore::saveAccounts(Data::AccountList pAccounts) {
	using namespace AccountXML;
#ifdef PROFILE_XML_DS
	Profile p(String(__FUNCTION__), mLog);
#endif
	TiXmlDocument document(FileLocation);

	auto accountsElement = new TiXmlElement(Tag::Accounts);
	document.LinkEndChild(accountsElement);

	for (auto i : pAccounts) {
		auto accountElement = new TiXmlElement(Tag::Account);
		accountsElement->LinkEndChild(accountElement);

		accountElement->SetAttribute(Attribute::LSServerID, std::to_string(i->mLoginServerID).c_str());
		accountElement->SetAttribute(Attribute::LSAccountID, std::to_string(i->mLoginAccountID).c_str());
		accountElement->SetAttribute(Attribute::Name, i->mLoginAccountName.c_str());
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

const bool XMLDataStore::loadAccountCharacterData(Data::Account* pAccount) {
	using namespace AccountCharacterDataXML;
#ifdef PROFILE_XML_DS
	Profile p(String(__FUNCTION__), mLog);
#endif
	if (!pAccount) return false;

	// Load document.
	const String fileLocation = "./data/accounts/" + pAccount->mLoginAccountName + ".xml";
	TiXmlDocument document(fileLocation.c_str());
	if (!document.LoadFile()) {
		mLog->error("Failed to load " + String(fileLocation));
		return false;
	}

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
		auto characterData = new Data::AccountCharacter();
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

const bool XMLDataStore::saveAccountCharacterData(Data::Account* pAccount) {
	using namespace AccountCharacterDataXML;
#ifdef PROFILE_XML_DS
	Profile p(String(__FUNCTION__), mLog);
#endif
	if (!pAccount) return false;

	TiXmlDocument document(String("./data/accounts/" + pAccount->mLoginAccountName + ".xml").c_str());

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

namespace CharacterXML {
#define SCA static const auto
	namespace Tag {
		SCA Character = "character";
		SCA Experience = "experience";
		SCA Stats = "stats";
		SCA Visual = "visual";
		SCA BindLocations = "bind_locations";
		SCA BindLocation = "bind_location";
		SCA Buffs = "buffs";
		SCA Buff = "buff";
		SCA Dyes = "dyes";
		SCA Dye = "dye";
		SCA Guild = "guild";
		SCA Currency = "currency";
		SCA Crystals = "crystals";
		SCA Radiant = "radiant";
		SCA Ebon = "ebon";
		SCA AlternateCurrencies = "alternate_currencies";
		SCA AlternateCurrency = "currency";
		SCA SpellBook = "spellbook";
		SCA SpellBookSlot = "sb_slot";
		SCA SpellBar = "spellbar";
		SCA SpellBarSlot = "sb_slot";
		SCA Skills = "skills";
		SCA Skill = "skill";
		SCA Lanaguages = "languages";
		SCA Language = "language";
		SCA Inventory = "inventory";
		SCA Tasks = "tasks";
		SCA Task = "task";
		SCA TaskHistory = "task_history";
		SCA CompletedTask = "task";
	}
	namespace Attribute {
		// Tag::Character
		SCA Name = "name";
		SCA GM = "gm";
		SCA Muted = "muted";
		SCA New = "new";
		SCA Status = "status";
		// Tag::Experience
		namespace Experience {
			SCA Level = "level";
			SCA MaximumLevel = "max_level";
			SCA Experience = "xp";
			SCA ExperienceToAA = "xp_to_aa";
			SCA AAExperience = "aa_xp";
			SCA UnspentAA = "unspent_aa";
			SCA MaximumUnspentAA = "max_unspent_aa";
			SCA SpentAA = "spent_aa";
			SCA MaximumSpentAA = "max_spent_aa";
		}
		SCA Class = "class";
		SCA Zone = "zone";
		SCA X = "x";
		SCA Y = "y";
		SCA Z = "z";
		SCA Heading = "heading";
		SCA LastName = "last_name";
		SCA Title = "title";
		SCA Suffix = "suffix";
		SCA InspectMessage = "inspect_message";
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
		// Tag::BindLocation
		namespace BindLocation {
			SCA ZoneID = "zone_id";
			SCA X = "x";
			SCA Y = "y";
			SCA Z = "z";
			SCA Heading = "heading";
		}
		// Tag::Buff
		namespace Buff {
			SCA SlotIndex = "slot";
			SCA SpellID = "spell_id";
			SCA TicksRemaining = "ticks_remaining";
		}
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
		// Tag::AlternateCurrencies
		SCA CurrencyID = "id";
		SCA CurrencyQuantity = "quantity";
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
		namespace CompletedTask {
			SCA TaskID = "id";
			SCA CompletedTime = "time";
		}
	}
#undef SCA
}

namespace InventoryXML {
#define SCA static const auto
	namespace Tag {
		SCA Item = "item";
	}
	namespace Attribute {
		SCA ID = "id";
		SCA Slot = "slot";
		SCA Stacks = "stacks";
		SCA Charges = "charges";
		SCA Attuned = "attuned";
		SCA LastCastTime = "last_cast_time";
	}
#undef SCA
}

const bool loadItem(TiXmlElement* pElement, Data::Item& pItem) {
	using namespace InventoryXML;
	if (!pElement) return false;

	// Load Item
	EXPECTED_BOOL(readAttribute(pElement, Attribute::ID, pItem.mItemID));
	EXPECTED_BOOL(readAttribute(pElement, Attribute::Slot, pItem.mSlot));
	EXPECTED_BOOL(readAttribute(pElement, Attribute::Stacks, pItem.mStacks));

	// Optional.
	EXPECTED_BOOL(readAttribute(pElement, Attribute::Charges, pItem.mCharges, false));
	EXPECTED_BOOL(readAttribute(pElement, Attribute::Attuned, pItem.mAttuned, false));
	EXPECTED_BOOL(readAttribute(pElement, Attribute::LastCastTime, pItem.mLastCastTime, false));

	// Load Sub-items
	auto itemElement = pElement->FirstChildElement(Tag::Item);
	while (itemElement) {
		Data::Item i;
		EXPECTED_BOOL(loadItem(itemElement, i));
		pItem.mSubItems.push_back(i);
		itemElement = itemElement->NextSiblingElement(Tag::Item);
	}

	return true;
}

const bool XMLDataStore::loadInventory(TiXmlElement* pElement, Data::Inventory& pInventory) {
	using namespace InventoryXML;
#ifdef PROFILE_XML_DS
	Profile p(String(__FUNCTION__), mLog);
#endif
	if (!pElement) return false;

	auto itemElement = pElement->FirstChildElement(Tag::Item);
	while (itemElement) {
		Data::Item i;
		EXPECTED_BOOL(loadItem(itemElement, i));
		pInventory.mItems.push_back(i);

		itemElement = itemElement->NextSiblingElement(Tag::Item);
	}

	return true;
}

const bool readExperience(TiXmlElement* pElement, Data::Experience& pExperience) {
	using namespace CharacterXML;
	if (!pElement) return false;

	EXPECTED_BOOL(readAttribute(pElement, Attribute::Experience::Level, pExperience.mLevel));
	EXPECTED_BOOL(readAttribute(pElement, Attribute::Experience::MaximumLevel, pExperience.mMaximumLevel));
	EXPECTED_BOOL(readAttribute(pElement, Attribute::Experience::Experience, pExperience.mExperience));
	EXPECTED_BOOL(readAttribute(pElement, Attribute::Experience::ExperienceToAA, pExperience.mExperienceToAA));
	EXPECTED_BOOL(readAttribute(pElement, Attribute::Experience::AAExperience, pExperience.mAAExperience));
	EXPECTED_BOOL(readAttribute(pElement, Attribute::Experience::UnspentAA, pExperience.mUnspentAAPoints));
	EXPECTED_BOOL(readAttribute(pElement, Attribute::Experience::MaximumUnspentAA, pExperience.mMaximumUnspentAA));
	EXPECTED_BOOL(readAttribute(pElement, Attribute::Experience::SpentAA, pExperience.mSpentAAPoints));
	EXPECTED_BOOL(readAttribute(pElement, Attribute::Experience::MaximumSpentAA, pExperience.mMaximumSpentAA));

	return true;
}

const bool XMLDataStore::loadCharacter(const String& pCharacterName, Data::Character* pCharacter) {
	using namespace CharacterXML;
#ifdef PROFILE_XML_DS
	Profile p(String(__FUNCTION__), mLog);
#endif
	if (!pCharacter) return false;

	// Load document.
	const String fileLocation = "./data/characters/" + pCharacterName + ".xml";
	TiXmlDocument document(fileLocation.c_str());
	if (!document.LoadFile()) {
		mLog->error("Failed to load " + fileLocation);
		return false;
	}

	// Tag::Character
	auto characterElement = document.FirstChildElement(Tag::Character);
	EXPECTED_BOOL(characterElement);
	EXPECTED_BOOL(readAttribute(characterElement, Attribute::Name, pCharacter->mName));
	EXPECTED_BOOL(readAttribute(characterElement, Attribute::GM, pCharacter->mGM));
	EXPECTED_BOOL(readAttribute(characterElement, Attribute::Muted, pCharacter->mMuted));
	EXPECTED_BOOL(readAttribute(characterElement, Attribute::New, pCharacter->mNew));
	EXPECTED_BOOL(readAttribute(characterElement, Attribute::Status, pCharacter->mStatus));
	EXPECTED_BOOL(readAttribute(characterElement, Attribute::Class, pCharacter->mClass));
	EXPECTED_BOOL(readAttribute(characterElement, Attribute::Zone, pCharacter->mZoneID));
	EXPECTED_BOOL(readVector3(characterElement, pCharacter->mPosition));
	EXPECTED_BOOL(readAttribute(characterElement, Attribute::Heading, pCharacter->mHeading));
	EXPECTED_BOOL(readAttribute(characterElement, Attribute::LastName, pCharacter->mLastName));
	EXPECTED_BOOL(readAttribute(characterElement, Attribute::Title, pCharacter->mTitle));
	EXPECTED_BOOL(readAttribute(characterElement, Attribute::Suffix, pCharacter->mSuffix));
	EXPECTED_BOOL(readAttribute(characterElement, Attribute::InspectMessage, pCharacter->mInspectMessage));
	EXPECTED_BOOL(readAttribute(characterElement, Attribute::AutoConsentGroup, pCharacter->mAutoConsentGroup));
	EXPECTED_BOOL(readAttribute(characterElement, Attribute::AutoConsentRaid, pCharacter->mAutoConsentRaid));
	EXPECTED_BOOL(readAttribute(characterElement, Attribute::AutoConsentGuild, pCharacter->mAutoConsentGuild));

	// Tag::Experience
	auto experienceElement = characterElement->FirstChildElement(Tag::Experience);
	EXPECTED_BOOL(readExperience(experienceElement, pCharacter->mExperience));

	// Tag::Stats
	auto statsElement = characterElement->FirstChildElement(Tag::Stats);
	EXPECTED_BOOL(statsElement);
	EXPECTED_BOOL(readAttribute(statsElement, Attribute::Strength, pCharacter->mStrength));
	EXPECTED_BOOL(readAttribute(statsElement, Attribute::Stamina, pCharacter->mStamina));
	EXPECTED_BOOL(readAttribute(statsElement, Attribute::Charisma, pCharacter->mCharisma));
	EXPECTED_BOOL(readAttribute(statsElement, Attribute::Intelligence, pCharacter->mIntelligence));
	EXPECTED_BOOL(readAttribute(statsElement, Attribute::Agility, pCharacter->mAgility));
	EXPECTED_BOOL(readAttribute(statsElement, Attribute::Wisdom, pCharacter->mWisdom));

	// Tag::Visual
	auto visualElement = characterElement->FirstChildElement(Tag::Visual);
	EXPECTED_BOOL(visualElement);
	EXPECTED_BOOL(readAttribute(visualElement, Attribute::Race, pCharacter->mRace));
	EXPECTED_BOOL(readAttribute(visualElement, Attribute::Gender, pCharacter->mGender));
	EXPECTED_BOOL(readAttribute(visualElement, Attribute::Face, pCharacter->mFaceStyle));
	EXPECTED_BOOL(readAttribute(visualElement, Attribute::HairStyle, pCharacter->mHairStyle));
	EXPECTED_BOOL(readAttribute(visualElement, Attribute::HairColour, pCharacter->mHairColour));
	EXPECTED_BOOL(readAttribute(visualElement, Attribute::BeardStyle, pCharacter->mBeardStyle));
	EXPECTED_BOOL(readAttribute(visualElement, Attribute::BeardColour, pCharacter->mBeardColour));
	EXPECTED_BOOL(readAttribute(visualElement, Attribute::EyeColour1, pCharacter->mEyeColourLeft));
	EXPECTED_BOOL(readAttribute(visualElement, Attribute::EyeColour2, pCharacter->mEyeColourRight));
	EXPECTED_BOOL(readAttribute(visualElement, Attribute::DrakkinHeritage, pCharacter->mDrakkinHeritage));
	EXPECTED_BOOL(readAttribute(visualElement, Attribute::DrakkinTattoo, pCharacter->mDrakkinTattoo));

	// Tag::BindLocations
	auto bindLocationsElement = characterElement->FirstChildElement(Tag::BindLocations);
	EXPECTED_BOOL(bindLocationsElement);
	auto bindLocationElement = bindLocationsElement->FirstChildElement(Tag::BindLocation);
	u32 bindLocationCount = 0;
	while (bindLocationElement) {
		EXPECTED_BOOL(readAttribute(bindLocationElement, Attribute::BindLocation::ZoneID, pCharacter->mBindLocations[bindLocationCount].mZoneID));
		EXPECTED_BOOL(readVector3(bindLocationElement, pCharacter->mBindLocations[bindLocationCount].mPosition));
		EXPECTED_BOOL(readAttribute(bindLocationElement, Attribute::BindLocation::Heading, pCharacter->mBindLocations[bindLocationCount].mHeading));

		bindLocationElement = bindLocationElement->NextSiblingElement(Tag::BindLocation);
		bindLocationCount++;
	}
	EXPECTED_BOOL(bindLocationCount == 5);

	// Tag::Buffs
	auto buffsElement = characterElement->FirstChildElement(Tag::Buffs);
	EXPECTED_BOOL(buffsElement);
	auto buffElement = buffsElement->FirstChildElement(Tag::Buff);
	while (buffElement) {
		auto buff = new Data::Buff();
		pCharacter->mBuffs.push_back(buff);

		EXPECTED_BOOL(readAttribute(buffElement, Attribute::Buff::SlotIndex, buff->mSlotIndex));
		EXPECTED_BOOL(readAttribute(buffElement, Attribute::Buff::SpellID, buff->mSpellID));
		EXPECTED_BOOL(readAttribute(buffElement, Attribute::Buff::TicksRemaining, buff->mTicksRemaining));

		buffElement = buffElement->NextSiblingElement(Tag::Buff);
	}

	// Tag::Dyes
	auto dyesElement = characterElement->FirstChildElement(Tag::Dyes);
	EXPECTED_BOOL(dyesElement);
	int slotID = 0;
	auto dyeElement = dyesElement->FirstChildElement(Tag::Dye);
	while (dyeElement) {
		EXPECTED_BOOL(readAttribute(dyeElement, Attribute::Colour, pCharacter->mDyes[slotID]));
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
		pCharacter->mSkills[skillID] = skillValue;

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
		pCharacter->mLanguages[languageID] = languageValue;

		languageElement = languageElement->NextSiblingElement(Tag::Language);
	}
	
	// Tag::Guild
	auto guildElement = characterElement->FirstChildElement(Tag::Guild);
	EXPECTED_BOOL(guildElement);
	EXPECTED_BOOL(readAttribute(guildElement, Attribute::GuildID, pCharacter->mGuildID));
	EXPECTED_BOOL(readAttribute(guildElement, Attribute::GuildRank, pCharacter->mGuildRank));

	// Tag::Currency
	auto currencyElement = characterElement->FirstChildElement(Tag::Currency);
	EXPECTED_BOOL(currencyElement);
	EXPECTED_BOOL(readAttribute(currencyElement, Attribute::PlatinumCharacter, pCharacter->mPlatinumCharacter));
	EXPECTED_BOOL(readAttribute(currencyElement, Attribute::PlatinumBank, pCharacter->mPlatinumBank));
	EXPECTED_BOOL(readAttribute(currencyElement, Attribute::PlatinumCursor, pCharacter->mPlatinumCursor));
	EXPECTED_BOOL(readAttribute(currencyElement, Attribute::GoldCharacter, pCharacter->mGoldCharacter));
	EXPECTED_BOOL(readAttribute(currencyElement, Attribute::GoldBank, pCharacter->mGoldBank));
	EXPECTED_BOOL(readAttribute(currencyElement, Attribute::GoldCursor, pCharacter->mGoldCursor));
	EXPECTED_BOOL(readAttribute(currencyElement, Attribute::SilverCharacter, pCharacter->mSilverCharacter));
	EXPECTED_BOOL(readAttribute(currencyElement, Attribute::SilverBank, pCharacter->mSilverBank));
	EXPECTED_BOOL(readAttribute(currencyElement, Attribute::SilverCursor, pCharacter->mSilverCursor));
	EXPECTED_BOOL(readAttribute(currencyElement, Attribute::CopperCharacter, pCharacter->mCopperCharacter));
	EXPECTED_BOOL(readAttribute(currencyElement, Attribute::CopperBank, pCharacter->mCopperBank));
	EXPECTED_BOOL(readAttribute(currencyElement, Attribute::CopperCursor, pCharacter->mCopperCursor));

	// Tag::Crystals
	auto crystalsElement = characterElement->FirstChildElement(Tag::Crystals);
	EXPECTED_BOOL(crystalsElement);
	// Tag::Radiant
	auto radiantElement = crystalsElement->FirstChildElement(Tag::Radiant);
	EXPECTED_BOOL(radiantElement);
	EXPECTED_BOOL(readAttribute(radiantElement, Attribute::Current, pCharacter->mRadiantCrystals));
	EXPECTED_BOOL(readAttribute(radiantElement, Attribute::Total, pCharacter->mTotalRadiantCrystals));
	// Tag::Ebon
	auto ebonElement = crystalsElement->FirstChildElement(Tag::Ebon);
	EXPECTED_BOOL(ebonElement);
	EXPECTED_BOOL(readAttribute(ebonElement, Attribute::Current, pCharacter->mEbonCrystals));
	EXPECTED_BOOL(readAttribute(ebonElement, Attribute::Total, pCharacter->mTotalEbonCrystals));

	// Tag::AlternateCurrencies
	auto alternateCurrenciesElement = characterElement->FirstChildElement(Tag::AlternateCurrencies);
	EXPECTED_BOOL(alternateCurrenciesElement);
	auto alternateCurrencyElement = alternateCurrenciesElement->FirstChildElement(Tag::AlternateCurrency);
	while (alternateCurrencyElement) {
		// Tag::AlternateCurrency
		uint32 currencyID = 0;
		uint32 currencyQuantity = 0;
		EXPECTED_BOOL(readAttribute(alternateCurrencyElement, Attribute::CurrencyID, currencyID));
		EXPECTED_BOOL(readAttribute(alternateCurrencyElement, Attribute::CurrencyQuantity, currencyQuantity));
		pCharacter->mAlternateCurrency[currencyID] = currencyQuantity;

		alternateCurrencyElement = alternateCurrencyElement->NextSiblingElement(Tag::AlternateCurrency);
	}

	// Caster Only
	if (Utility::isCaster(pCharacter->mClass)) {
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

				pCharacter->mSpellBook[slotID] = spellID;

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

				pCharacter->mSpellBar[slotID] = spellID;

				slotElement = slotElement->NextSiblingElement(Tag::SpellBarSlot);
			}
		}
	}

	// Tag::Inventory
	auto inventoryElement = characterElement->FirstChildElement(Tag::Inventory);
	EXPECTED_BOOL(inventoryElement);
	EXPECTED_BOOL(loadInventory(inventoryElement, pCharacter->mInventory));

	// Tag::TaskHistory
	auto taskHistoryElement = characterElement->FirstChildElement(Tag::TaskHistory);
	EXPECTED_BOOL(taskHistoryElement);
	auto completedTaskElement = taskHistoryElement->FirstChildElement(Tag::CompletedTask);

	while (completedTaskElement) {
		// Tag::CompletedTask
		Data::CompletedTask completedTask;
		EXPECTED_BOOL(readAttribute(completedTaskElement, Attribute::CompletedTask::TaskID, completedTask.mTaskID));
		EXPECTED_BOOL(readAttribute(completedTaskElement, Attribute::CompletedTask::CompletedTime, completedTask.mCompleted));
		pCharacter->mCompletedTasks.push_front(completedTask);

		completedTaskElement = completedTaskElement->NextSiblingElement(Tag::CompletedTask);
	}

	return true;
}

const bool saveItem(TiXmlElement* pParent, const Data::Item& pItem) {
	using namespace InventoryXML;
	// Save Item
	auto itemElement = static_cast<TiXmlElement*>(pParent->LinkEndChild(new TiXmlElement(Tag::Item)));

	itemElement->SetAttribute(Attribute::ID, pItem.mItemID);
	itemElement->SetAttribute(Attribute::Slot, pItem.mSlot);	
	itemElement->SetAttribute(Attribute::Stacks, pItem.mStacks);

	// Optional.
	if (pItem.mCharges > 0)
		itemElement->SetAttribute(Attribute::Charges, pItem.mCharges);
	if (pItem.mAttuned != 0)
		itemElement->SetAttribute(Attribute::Attuned, pItem.mAttuned);
	if (pItem.mLastCastTime != 0)
		itemElement->SetAttribute(Attribute::LastCastTime, pItem.mLastCastTime);

	// Save Sub-Items
	for (auto& i : pItem.mSubItems)
		EXPECTED_BOOL(saveItem(itemElement, i));

	return true;
}

const bool XMLDataStore::saveInventory(TiXmlElement* pElement, const Data::Inventory& pInventory) {
	using namespace InventoryXML;
#ifdef PROFILE_XML_DS
	Profile p(String(__FUNCTION__), mLog);
#endif
	EXPECTED_BOOL(pElement);

	for (auto& i : pInventory.mItems)
		EXPECTED_BOOL(saveItem(pElement, i));

	return true;
}

const bool writeExperience(TiXmlElement* pElement, const Data::Experience& pExperience) {
	using namespace CharacterXML;
	EXPECTED_BOOL(pElement);

	pElement->SetAttribute(Attribute::Experience::Level, pExperience.mLevel);
	pElement->SetAttribute(Attribute::Experience::MaximumLevel, pExperience.mMaximumLevel);
	pElement->SetAttribute(Attribute::Experience::Experience, pExperience.mExperience);
	pElement->SetAttribute(Attribute::Experience::ExperienceToAA, pExperience.mExperienceToAA);
	pElement->SetAttribute(Attribute::Experience::AAExperience, pExperience.mAAExperience);
	pElement->SetAttribute(Attribute::Experience::UnspentAA, pExperience.mUnspentAAPoints);
	pElement->SetAttribute(Attribute::Experience::MaximumUnspentAA, pExperience.mMaximumUnspentAA);
	pElement->SetAttribute(Attribute::Experience::SpentAA, pExperience.mSpentAAPoints);
	pElement->SetAttribute(Attribute::Experience::MaximumSpentAA, pExperience.mMaximumSpentAA);

	return true;
}

const bool XMLDataStore::saveCharacter(const String& pCharacterName, const Data::Character* pCharacter) {
	using namespace CharacterXML;
#ifdef PROFILE_XML_DS
	Profile p(String(__FUNCTION__), mLog);
#endif
	if (!pCharacter) return false;
	TiXmlDocument document(String("./data/characters/" + pCharacterName + ".xml").c_str());

	// Tag::Character
	auto characterElement = static_cast<TiXmlElement*>(document.LinkEndChild(new TiXmlElement(Tag::Character)));
	characterElement->SetAttribute(Attribute::Name, pCharacter->mName.c_str());
	characterElement->SetAttribute(Attribute::GM, pCharacter->mGM);
	characterElement->SetAttribute(Attribute::Muted, pCharacter->mMuted);
	characterElement->SetAttribute(Attribute::New, pCharacter->mNew);
	characterElement->SetAttribute(Attribute::Status, pCharacter->mStatus);
	characterElement->SetAttribute(Attribute::Class, pCharacter->mClass);
	characterElement->SetAttribute(Attribute::Zone, pCharacter->mZoneID);
	writeVector3(characterElement, pCharacter->mPosition);
	characterElement->SetDoubleAttribute(Attribute::Heading, pCharacter->mHeading);
	characterElement->SetAttribute(Attribute::LastName, pCharacter->mLastName.c_str());
	characterElement->SetAttribute(Attribute::Title, pCharacter->mTitle.c_str());
	characterElement->SetAttribute(Attribute::Suffix, pCharacter->mSuffix.c_str());
	characterElement->SetAttribute(Attribute::InspectMessage, pCharacter->mInspectMessage.c_str());
	characterElement->SetAttribute(Attribute::AutoConsentGroup, pCharacter->mAutoConsentGroup);
	characterElement->SetAttribute(Attribute::AutoConsentRaid, pCharacter->mAutoConsentRaid);
	characterElement->SetAttribute(Attribute::AutoConsentGuild, pCharacter->mAutoConsentGuild);

	// Tag::Experience
	auto experienceElement = static_cast<TiXmlElement*>(characterElement->LinkEndChild(new TiXmlElement(Tag::Experience)));
	EXPECTED_BOOL(writeExperience(experienceElement, pCharacter->mExperience));

	// Tag::Stats
	auto statsElement = static_cast<TiXmlElement*>(characterElement->LinkEndChild(new TiXmlElement(Tag::Stats)));
	statsElement->SetAttribute(Attribute::Strength, pCharacter->mStrength);
	statsElement->SetAttribute(Attribute::Stamina, pCharacter->mStamina);
	statsElement->SetAttribute(Attribute::Charisma, pCharacter->mCharisma);
	statsElement->SetAttribute(Attribute::Intelligence, pCharacter->mIntelligence);
	statsElement->SetAttribute(Attribute::Agility, pCharacter->mAgility);
	statsElement->SetAttribute(Attribute::Wisdom, pCharacter->mWisdom);

	// Tag::Visual
	auto visualElement = static_cast<TiXmlElement*>(characterElement->LinkEndChild(new TiXmlElement(Tag::Visual)));
	visualElement->SetAttribute(Attribute::Race, pCharacter->mRace);
	visualElement->SetAttribute(Attribute::Gender, pCharacter->mGender);
	visualElement->SetAttribute(Attribute::Face, pCharacter->mFaceStyle);
	visualElement->SetAttribute(Attribute::HairStyle, pCharacter->mHairStyle);
	visualElement->SetAttribute(Attribute::HairColour, pCharacter->mHairColour);
	visualElement->SetAttribute(Attribute::BeardStyle, pCharacter->mBeardStyle);
	visualElement->SetAttribute(Attribute::BeardColour, pCharacter->mBeardColour);
	visualElement->SetAttribute(Attribute::EyeColour1, pCharacter->mEyeColourLeft);
	visualElement->SetAttribute(Attribute::EyeColour2, pCharacter->mEyeColourRight);
	visualElement->SetAttribute(Attribute::DrakkinHeritage, pCharacter->mDrakkinHeritage);
	visualElement->SetAttribute(Attribute::DrakkinTattoo, pCharacter->mDrakkinTattoo);

	// Tag::BindLocations
	auto bindLocationsElement = static_cast<TiXmlElement*>(characterElement->LinkEndChild(new TiXmlElement(Tag::BindLocations)));
	for (int i = 0; i < 5; i++) {
		// Tag::BindLocation
		auto bindLocationElement = static_cast<TiXmlElement*>(bindLocationsElement->LinkEndChild(new TiXmlElement(Tag::BindLocation)));
		bindLocationElement->SetAttribute(Attribute::BindLocation::ZoneID, pCharacter->mBindLocations[i].mZoneID);
		writeVector3(bindLocationElement, pCharacter->mBindLocations[i].mPosition);
		bindLocationElement->SetDoubleAttribute(Attribute::BindLocation::Heading, pCharacter->mBindLocations[i].mHeading);
	}

	// Tag::Buffs
	auto BuffsElement = static_cast<TiXmlElement*>(characterElement->LinkEndChild(new TiXmlElement(Tag::Buffs)));
	for (auto& i : pCharacter->mBuffs) {
		// Tag::Buff
		auto buffElement = static_cast<TiXmlElement*>(BuffsElement->LinkEndChild(new TiXmlElement(Tag::Buff)));
		buffElement->SetAttribute(Attribute::Buff::SlotIndex, i->mSlotIndex);
		buffElement->SetAttribute(Attribute::Buff::SpellID, i->mSpellID);
		buffElement->SetAttribute(Attribute::Buff::TicksRemaining, i->mTicksRemaining);
	}

	// Tag::Dyes
	auto dyesElement = static_cast<TiXmlElement*>(characterElement->LinkEndChild(new TiXmlElement(Tag::Dyes)));
	for (int i = 0; i < MAX_ARMOR_DYE_SLOTS; i++) {
		// Tag::Dye
		auto dyeElement = static_cast<TiXmlElement*>(dyesElement->LinkEndChild(new TiXmlElement(Tag::Dye)));
		dyeElement->SetAttribute(Attribute::Colour, pCharacter->mDyes[i]);
	}

	// Tag::Skills
	auto skillsElement = static_cast<TiXmlElement*>(characterElement->LinkEndChild(new TiXmlElement(Tag::Skills)));
	for (int i = 0; i < Limits::Skills::MAX_ID; i++) {
		if (pCharacter->mSkills[i] == 0) continue;
		// Tag::Skill
		auto skillElement = static_cast<TiXmlElement*>(skillsElement->LinkEndChild(new TiXmlElement(Tag::Skill)));
		skillElement->SetAttribute(Attribute::SkillID, i);
		skillElement->SetAttribute(Attribute::SkillValue, pCharacter->mSkills[i]);
	}

	// Tag::Languages
	auto LanguagesElement = static_cast<TiXmlElement*>(characterElement->LinkEndChild(new TiXmlElement(Tag::Lanaguages)));
	for (int i = 0; i < Limits::Languages::MAX_ID; i++) {
		if (pCharacter->mLanguages[i] == 0) continue;
		// Tag::Language
		auto languageElement = static_cast<TiXmlElement*>(LanguagesElement->LinkEndChild(new TiXmlElement(Tag::Language)));
		languageElement->SetAttribute(Attribute::LanguageID, i);
		languageElement->SetAttribute(Attribute::LanguageValue, pCharacter->mLanguages[i]);
	}

	// Tag::Guild
	auto guildElement = static_cast<TiXmlElement*>(characterElement->LinkEndChild(new TiXmlElement(Tag::Guild)));
	guildElement->SetAttribute(Attribute::GuildID, pCharacter->mGuildID);
	guildElement->SetAttribute(Attribute::GuildRank, pCharacter->mGuildRank);

	// Tag::Currency
	auto currencyElement = static_cast<TiXmlElement*>(characterElement->LinkEndChild(new TiXmlElement(Tag::Currency)));
	currencyElement->SetAttribute(Attribute::PlatinumCharacter, pCharacter->mPlatinumCharacter);
	currencyElement->SetAttribute(Attribute::PlatinumBank, pCharacter->mPlatinumBank);
	currencyElement->SetAttribute(Attribute::PlatinumCursor, pCharacter->mPlatinumCursor);
	currencyElement->SetAttribute(Attribute::GoldCharacter, pCharacter->mGoldCharacter);
	currencyElement->SetAttribute(Attribute::GoldBank, pCharacter->mGoldBank);
	currencyElement->SetAttribute(Attribute::GoldCursor, pCharacter->mGoldCursor);
	currencyElement->SetAttribute(Attribute::SilverCharacter, pCharacter->mSilverCharacter);
	currencyElement->SetAttribute(Attribute::SilverBank, pCharacter->mSilverBank);
	currencyElement->SetAttribute(Attribute::SilverCursor, pCharacter->mSilverCursor);
	currencyElement->SetAttribute(Attribute::CopperCharacter, pCharacter->mCopperCharacter);
	currencyElement->SetAttribute(Attribute::CopperBank, pCharacter->mCopperBank);
	currencyElement->SetAttribute(Attribute::CopperCursor, pCharacter->mCopperCursor);

	// Tag::Crystals
	auto crystalsElement = static_cast<TiXmlElement*>(characterElement->LinkEndChild(new TiXmlElement(Tag::Crystals)));
	// Tag::Radiant
	auto radiantElement = static_cast<TiXmlElement*>(crystalsElement->LinkEndChild(new TiXmlElement(Tag::Radiant)));
	radiantElement->SetAttribute(Attribute::Current, pCharacter->mRadiantCrystals);
	radiantElement->SetAttribute(Attribute::Total, pCharacter->mTotalRadiantCrystals);
	// Tag::Ebon
	auto ebonElement = static_cast<TiXmlElement*>(crystalsElement->LinkEndChild(new TiXmlElement(Tag::Ebon)));
	ebonElement->SetAttribute(Attribute::Current, pCharacter->mEbonCrystals);
	ebonElement->SetAttribute(Attribute::Total, pCharacter->mTotalEbonCrystals);

	// Tag::AlternateCurrencies
	auto alternateCurrenciesElement = static_cast<TiXmlElement*>(characterElement->LinkEndChild(new TiXmlElement(Tag::AlternateCurrencies)));
	for (auto i : pCharacter->mAlternateCurrency) {
		if (i.second == 0) continue;
		// Tag::AlternateCurrency
		auto currencyElement = static_cast<TiXmlElement*>(alternateCurrenciesElement->LinkEndChild(new TiXmlElement(Tag::AlternateCurrency)));
		currencyElement->SetAttribute(Attribute::CurrencyID, i.first);
		currencyElement->SetAttribute(Attribute::CurrencyQuantity, i.second);
	}

	// Caster Only
	if (Utility::isCaster(pCharacter->mClass)){
		// Tag::SpellBook
		auto spellbookElement = static_cast<TiXmlElement*>(characterElement->LinkEndChild(new TiXmlElement(Tag::SpellBook)));
		for (auto i = 0; i < Limits::SpellBook::MAX_SLOTS; i++) {
			if (pCharacter->mSpellBook[i] == 0)
				continue;
			
			auto slotElement = static_cast<TiXmlElement*>(spellbookElement->LinkEndChild(new TiXmlElement(Tag::SpellBookSlot)));
			slotElement->SetAttribute(Attribute::SpellBookSlot, i);
			slotElement->SetAttribute(Attribute::SpellBookSpell, pCharacter->mSpellBook[i]);
		}
		// Tag::SpellBar
		auto spellbarElement = static_cast<TiXmlElement*>(characterElement->LinkEndChild(new TiXmlElement(Tag::SpellBar)));
		for (auto i = 0; i < Limits::SpellBar::MAX_SLOTS; i++) {
			if (pCharacter->mSpellBar[i] == 0)
				continue;

			auto slotElement = static_cast<TiXmlElement*>(spellbarElement->LinkEndChild(new TiXmlElement(Tag::SpellBarSlot)));
			slotElement->SetAttribute(Attribute::SpellBarSlot, i);
			slotElement->SetAttribute(Attribute::SpellBarSpell, pCharacter->mSpellBar[i]);
		}
	}

	// Tag::Inventory
	auto inventoryElement = static_cast<TiXmlElement*>(characterElement->LinkEndChild(new TiXmlElement(Tag::Inventory)));
	EXPECTED_BOOL(saveInventory(inventoryElement, pCharacter->mInventory));

	// Tag::TaskHistory
	auto taskHistoryElement = static_cast<TiXmlElement*>(characterElement->LinkEndChild(new TiXmlElement(Tag::TaskHistory)));
	for (auto i : pCharacter->mCompletedTasks) {
		// Tag::CompletedTask
		auto completedTaskElement = static_cast<TiXmlElement*>(taskHistoryElement->LinkEndChild(new TiXmlElement(Tag::CompletedTask)));
		completedTaskElement->SetAttribute(Attribute::CompletedTask::TaskID, i.mTaskID);
		completedTaskElement->SetAttribute(Attribute::CompletedTask::CompletedTime, i.mCompleted);
	}

	EXPECTED_BOOL(document.SaveFile());
	return true;
}

namespace SettingsXML {
#define SCA static const auto
	SCA FileLocation = "./data/settings.xml";
	namespace Tag {
		SCA Settings = "settings";
		SCA World = "world";
		SCA LoginServer = "login_server";
		SCA UCS = "ucs";
		SCA Validation = "validation";
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
		// Tag::Validation
		SCA ValidationEnabled = "enabled";
	}
#undef SCA
}

const bool XMLDataStore::loadSettings() {
	using namespace SettingsXML;

	// Load document.
	TiXmlDocument document(SettingsXML::FileLocation);
	if (!document.LoadFile()) {
		mLog->error("Failed to load " + String(SettingsXML::FileLocation));
		return false;
	}

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

	// Tag::Validation
	auto validationElement = settingsElement->FirstChildElement(Tag::Validation);
	EXPECTED_BOOL(validationElement);
	
	bool validationEnabled = true;
	EXPECTED_BOOL(readAttribute(validationElement, Attribute::ValidationEnabled, validationEnabled));
	Settings::_setValidationEnabled(validationEnabled);

	return true;
}

namespace NPCAppearanceXML {
#define SCA static const auto
	SCA FileLocation = "./data/npc/appearances.xml";
	namespace Tag {
		SCA Appearances = "appearances";
		SCA Appearance = "appearance";
		SCA Colours = "colours";
		SCA Colour = "colour";
	}
	namespace Attribute {
		// Tag::Appearance
		SCA ID = "id";
		SCA Parent = "parent";
		SCA Race = "race";
		SCA Gender = "gender";
		SCA Texture = "texture";
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
		SCA PrimaryMaterial = "primary_material";
		SCA SecondaryMaterial = "secondary_material";
		// Tag::Colour
		SCA Slot = "slot";
		SCA Red = "r";
		SCA Green = "g";
		SCA Blue = "b";
	}
#undef SCA
}

const bool XMLDataStore::loadNPCAppearanceData(std::list<Data::NPCAppearance*>& pAppearances) {
	using namespace NPCAppearanceXML;
#ifdef PROFILE_XML_DS
	Profile p(String(__FUNCTION__), mLog);
#endif
	if (!pAppearances.empty()) return false;

	// Load document.
	TiXmlDocument document(NPCAppearanceXML::FileLocation);
	if (!document.LoadFile()) {
		mLog->error("Failed to load " + String(NPCAppearanceXML::FileLocation));
		return false;
	}

	auto appearancesElement = document.FirstChildElement(Tag::Appearances);
	EXPECTED_BOOL(appearancesElement);
	auto appearanceElement = appearancesElement->FirstChildElement(Tag::Appearance);

	while (appearanceElement) {
		auto d = new Data::NPCAppearance();
		pAppearances.push_back(d);

		// Required.
		EXPECTED_BOOL(readAttribute(appearanceElement, Attribute::ID, d->mID));
		EXPECTED_BOOL(readAttribute(appearanceElement, Attribute::Parent, d->mParentID));

		// Optional.
		/*
			When each attribute is read, we record in NPCAppearanceData::mOverrides whether or not this NPCAppearanceData specified a value for it.
			The data in mOverrides in then used later when resolving inheritance whether to take the parent value, or the child overridden value.
			I considered using a set of 'default values' and just checked whether a child value differs from that when resolving inheritance however 
			there this introduces edge case bugs and ambiguity. For this reason, overrides are recorded.
		*/
		bool found = false;

		EXPECTED_BOOL(readAttribute(appearanceElement, Attribute::Race, d->mRaceID, false, found));
		d->mOverrides[Data::NPCAppearance::Attributes::RaceID] = found;
		
		EXPECTED_BOOL(readAttribute(appearanceElement, Attribute::Gender, d->mGender, false, found));
		d->mOverrides[Data::NPCAppearance::Attributes::Gender] = found;

		EXPECTED_BOOL(readAttribute(appearanceElement, Attribute::Texture, d->mTexture, false, found));
		d->mOverrides[Data::NPCAppearance::Attributes::Texture] = found;

		EXPECTED_BOOL(readAttribute(appearanceElement, Attribute::BodyType, d->mBodyType, false, found));
		d->mOverrides[Data::NPCAppearance::Attributes::BodyType] = found;

		EXPECTED_BOOL(readAttribute(appearanceElement, Attribute::Size, d->mSize, false, found));
		d->mOverrides[Data::NPCAppearance::Attributes::Size] = found;

		EXPECTED_BOOL(readAttribute(appearanceElement, Attribute::FaceStyle, d->mFaceStyle, false, found));
		d->mOverrides[Data::NPCAppearance::Attributes::FaceStyle] = found;

		EXPECTED_BOOL(readAttribute(appearanceElement, Attribute::HairStyle, d->mHairStyle, false, found));
		d->mOverrides[Data::NPCAppearance::Attributes::HairStyle] = found;

		EXPECTED_BOOL(readAttribute(appearanceElement, Attribute::BeardStyle, d->mBeardStyle, false, found));
		d->mOverrides[Data::NPCAppearance::Attributes::BeardStyle] = found;

		EXPECTED_BOOL(readAttribute(appearanceElement, Attribute::HairColour, d->mHairColour, false, found));
		d->mOverrides[Data::NPCAppearance::Attributes::HairColour] = found;

		EXPECTED_BOOL(readAttribute(appearanceElement, Attribute::BeardColour, d->mBeardColour, false, found));
		d->mOverrides[Data::NPCAppearance::Attributes::BeardColour] = found;

		EXPECTED_BOOL(readAttribute(appearanceElement, Attribute::LeftEyeColour, d->mEyeColourLeft, false, found));
		d->mOverrides[Data::NPCAppearance::Attributes::EyeColourLeft] = found;

		EXPECTED_BOOL(readAttribute(appearanceElement, Attribute::RightEyeColour, d->mEyeColourRight, false, found));
		d->mOverrides[Data::NPCAppearance::Attributes::EyeColourRight] = found;

		EXPECTED_BOOL(readAttribute(appearanceElement, Attribute::DrakkinHeritage, d->mDrakkinHeritage, false, found));
		d->mOverrides[Data::NPCAppearance::Attributes::DrakkinHeritage] = found;

		EXPECTED_BOOL(readAttribute(appearanceElement, Attribute::DrakkinTattoo, d->mDrakkinTattoo, false, found));
		d->mOverrides[Data::NPCAppearance::Attributes::DrakkinTattoo] = found;

		EXPECTED_BOOL(readAttribute(appearanceElement, Attribute::DrakkinDetails, d->mDrakkinDetails, false, found));
		d->mOverrides[Data::NPCAppearance::Attributes::DrakkinDetails] = found;

		EXPECTED_BOOL(readAttribute(appearanceElement, Attribute::HelmTexture, d->mHelmTexture, false, found));
		d->mOverrides[Data::NPCAppearance::Attributes::HelmTexture] = found;

		EXPECTED_BOOL(readAttribute(appearanceElement, Attribute::PrimaryMaterial, d->mPrimaryMaterial, false, found));
		d->mOverrides[Data::NPCAppearance::Attributes::PrimaryMaterial] = found;

		EXPECTED_BOOL(readAttribute(appearanceElement, Attribute::SecondaryMaterial, d->mSecondaryMaterial, false, found));
		d->mOverrides[Data::NPCAppearance::Attributes::SecondaryMaterial] = found;

		// Colours.
		auto coloursElement = appearanceElement->FirstChildElement(Tag::Colours);
		if (coloursElement) {
			auto colourElement = coloursElement->FirstChildElement(Tag::Colour);
			while (colourElement) {
				uint32 slot = 0;
				Colour colour;
				EXPECTED_BOOL(readAttribute(colourElement, Attribute::Slot, slot));
				EXPECTED_BOOL(slot < 7);
				EXPECTED_BOOL(readAttribute(colourElement, Attribute::Red, colour.mRGB.mRed));
				EXPECTED_BOOL(readAttribute(colourElement, Attribute::Green, colour.mRGB.mGreen));
				EXPECTED_BOOL(readAttribute(colourElement, Attribute::Blue, colour.mRGB.mBlue));
				d->mColours[slot] = colour.mColour;

				colourElement = colourElement->NextSiblingElement(Tag::Colour);
			}
		}

		appearanceElement = appearanceElement->NextSiblingElement(Tag::Appearance);
	}

	return true;
}

namespace NPCTypeXML {
#define SCA static const auto
	SCA FileLocation = "./data/npc/types.xml";
	namespace Tag {
		SCA Types = "types";
		SCA Type = "type";
	}
	namespace Attribute {
		// Tag::Type
		SCA ID = "id";
		SCA AppearanceID = "appearance_id";
		SCA Name = "name";
		SCA LastName = "last_name";
		SCA Class = "class";
		SCA ShopID = "shop_id";
	}
#undef SCA
}
const bool XMLDataStore::loadNPCTypeData(std::list<Data::NPCType*>& pTypes) {
	using namespace NPCTypeXML;
#ifdef PROFILE_XML_DS
	Profile p(String(__FUNCTION__), mLog);
#endif
	if (!pTypes.empty()) return false;

	// Load document.
	TiXmlDocument document(NPCTypeXML::FileLocation);
	if (!document.LoadFile()) {
		mLog->error("Failed to load " + String(NPCTypeXML::FileLocation));
		return false;
	}

	auto typesElement = document.FirstChildElement(Tag::Types);
	EXPECTED_BOOL(typesElement);
	auto typeElement = typesElement->FirstChildElement(Tag::Type);

	while (typeElement) {
		auto d = new Data::NPCType();
		pTypes.push_back(d);

		EXPECTED_BOOL(readAttribute(typeElement, Attribute::ID, d->mID));
		EXPECTED_BOOL(readAttribute(typeElement, Attribute::AppearanceID, d->mAppearanceID));
		EXPECTED_BOOL(readAttribute(typeElement, Attribute::Name, d->mName));
		EXPECTED_BOOL(readAttribute(typeElement, Attribute::LastName, d->mLastName, false));
		EXPECTED_BOOL(readAttribute(typeElement, Attribute::Class, d->mClass));
		EXPECTED_BOOL(readAttribute(typeElement, Attribute::ShopID, d->mShopID, false));

		typeElement = typeElement->NextSiblingElement(Tag::Type);
	}

	return true;
}

const bool XMLDataStore::deleteCharacter(const String& pCharacterName) {
	const String existingFile = "./data/characters/" + pCharacterName + ".xml";
	const String newFile = "./data/characters/deleted/" + std::to_string(Utility::Time::now()) + "_" + pCharacterName + ".xml";
	// Copy the character xml to the deleted directory.
	//EXPECTED_BOOL(CopyFile(existingFile.c_str(), newFile.c_str(), true));
	// Delete the character file.
	//EXPECTED_BOOL(DeleteFile(existingFile.c_str()));

	// removed calls to CopyFile and DeleteFile for now as I am getting some strange errors.

	return true;
}

namespace SpellField {
	enum : int {
		ID,
		Name,
		Player,
		TeleportZone,
		YouCast,
		OtherCasts,
		CastOnYou,
		CastOnOther,
		FadeMessage,
		Range,
		AOERange,
		PushBack,
		PushUp,
		CastTime,
		RecoveryTime,
		RecastTime,
		BuffDurationFormula,
		BuffDuration,
		AOEDuration,
		ManaCost,
		EffectBase1,
		EffectBase2,
		EffectBase3,
		EffectBase4,
		EffectBase5,
		EffectBase6,
		EffectBase7,
		EffectBase8,
		EffectBase9,
		EffectBase10,
		EffectBase11,
		EffectBase12,
		EffectLimit1,
		EffectLimit2,
		EffectLimit3,
		EffectLimit4,
		EffectLimit5,
		EffectLimit6,
		EffectLimit7,
		EffectLimit8,
		EffectLimit9,
		EffectLimit10,
		EffectLimit11,
		EffectLimit12,
		EffectMax1,
		EffectMax2,
		EffectMax3,
		EffectMax4,
		EffectMax5,
		EffectMax6,
		EffectMax7,
		EffectMax8,
		EffectMax9,
		EffectMax10,
		EffectMax11,
		EffectMax12,
		Icon,
		MemIcon,
		ComponentID_1,
		ComponentID_2,
		ComponentID_3,
		ComponentID_4,
		ComponentCount_1,
		ComponentCount_2,
		ComponentCount_3,
		ComponentCount_4,
		NoExpendReagent_1,
		NoExpendReagent_2,
		NoExpendReagent_3,
		NoExpendReagent_4,
		EffectFormula1,
		EffectFormula2,
		EffectFormula3,
		EffectFormula4,
		EffectFormula5,
		EffectFormula6,
		EffectFormula7,
		EffectFormula8,
		EffectFormula9,
		EffectFormula10,
		EffectFormula11,
		EffectFormula12,
		LightType,
		GoodEffect,
		Activated,
		ResistType,
		EffectType1,
		EffectType2,
		EffectType3,
		EffectType4,
		EffectType5,
		EffectType6,
		EffectType7,
		EffectType8,
		EffectType9,
		EffectType10,
		EffectType11,
		EffectType12,
		TargetType,
		BaseDifficulty,
		ZoneType,
		EnvironmentType,
		TimeOfDay,
		UNK,
		WAR_Level,
		CLR_Level,
		PAL_Level,
		RNG_Level,
		SHD_Level,
		DRU_Level,
		MNK_Level,
		BRD_Level,
		ROG_Level,
		SHM_Level,
		NEC_Level,
		WIZ_Level,
		MAG_Level,
		ENC_Level,
		BST_Level,
		BER_Level,
	};
}
const bool XMLDataStore::loadSpells(Data::SpellDataArray pSpells, const u32 pMaxSpellID, u32& pNumSpellsLoaded) {
#ifdef PROFILE_XML_DS
	Profile p(String(__FUNCTION__), mLog);
#endif
	//if (!pSpells) return false;

	// Load file.
	static const String FileLocation = "./data/spells_us.txt";
	std::ifstream input;
	input.open(FileLocation, std::ios::in);
	if (!input.is_open()) {
		mLog->error("Failed to load " + FileLocation);
		return false;
	}

	// Read file.
	String line;
	while (std::getline(input, line)) {
		pNumSpellsLoaded++;
		// Split each line into an array.
		StringStream ss(line);
		static const auto NumFields = 215;
		std::array<String, NumFields> fields;
		for (auto i = 0; i < NumFields; i++) {
			std::getline(ss, fields[i], '^');
		}

		u32 id = 0;
		EXPECTED_BOOL(Utility::stoSafe(id, fields[SpellField::ID]));

		// Process.
		auto spell = pSpells[id];
		spell->mID = id;
		spell->mName = fields[SpellField::Name];
		spell->mPlayer = fields[SpellField::Player];
		spell->mTeleportZone = fields[SpellField::TeleportZone];
		spell->mYouCastMessage = fields[SpellField::YouCast];
		spell->mOtherCastsMessage = fields[SpellField::OtherCasts];
		spell->mCastOnYouMessage = fields[SpellField::CastOnYou];
		spell->mCastOnOtherMessage = fields[SpellField::CastOnOther];
		spell->mFadeMessage = fields[SpellField::FadeMessage];

		EXPECTED_BOOL(Utility::stoSafe(spell->mRange, fields[SpellField::Range]));
		EXPECTED_BOOL(Utility::stoSafe(spell->mAOERange, fields[SpellField::AOERange]));
		EXPECTED_BOOL(Utility::stoSafe(spell->mPushBack, fields[SpellField::PushBack]));
		EXPECTED_BOOL(Utility::stoSafe(spell->mPushUp, fields[SpellField::PushUp]));
		EXPECTED_BOOL(Utility::stoSafe(spell->mCastTime, fields[SpellField::CastTime]));
		EXPECTED_BOOL(Utility::stoSafe(spell->mRecoveryTime, fields[SpellField::RecoveryTime]));
		EXPECTED_BOOL(Utility::stoSafe(spell->mRecastTime, fields[SpellField::RecastTime]));
		EXPECTED_BOOL(Utility::stoSafe(spell->mBuffDurationFormula, fields[SpellField::BuffDurationFormula]));
		EXPECTED_BOOL(Utility::stoSafe(spell->mBuffDuration, fields[SpellField::BuffDuration]));
		EXPECTED_BOOL(Utility::stoSafe(spell->mAOEDuration, fields[SpellField::AOEDuration]));
		EXPECTED_BOOL(Utility::stoSafe(spell->mManaCost, fields[SpellField::ManaCost]));

		EXPECTED_BOOL(Utility::stoSafe(spell->mIcon, fields[SpellField::Icon]));
		EXPECTED_BOOL(Utility::stoSafe(spell->mMemIcon, fields[SpellField::MemIcon]));

		// Classes.
		for (auto i = 0; i < 16; i++)
			EXPECTED_BOOL(Utility::stoSafe(spell->mLevels[i], fields[SpellField::WAR_Level + i]));

		// Effects.
		static const auto NumEffects = 12;
		for (auto i = 0; i < NumEffects; i++) {
			// 254 = no effect.
			u32 effectType = 0;
			EXPECTED_BOOL(Utility::stoSafe(effectType, fields[SpellField::EffectType1 + i]));
			if (effectType == 254) continue;

			auto effect = new Data::SpellEffect();
			spell->mEffects.push_back(effect);

			effect->mType = effectType;
			EXPECTED_BOOL(Utility::stoSafe(effect->mBaseValue, fields[SpellField::EffectBase1 + i]));
			EXPECTED_BOOL(Utility::stoSafe(effect->mLimitValue, fields[SpellField::EffectLimit1 + i]));
			EXPECTED_BOOL(Utility::stoSafe(effect->mMaximum, fields[SpellField::EffectMax1 + i]));
			EXPECTED_BOOL(Utility::stoSafe(effect->mFormula, fields[SpellField::EffectFormula1 + i]));
		}

		// Components.
		static const auto NumComponents = 4;
		for (auto i = 0; i < NumComponents; i++) {
			// -1 = no component.
			i32 itemID = 0;
			EXPECTED_BOOL(Utility::stoSafe(itemID, fields[SpellField::ComponentID_1 + i]));
			if (itemID == -1) continue;

			auto component = new Data::SpellComponent();
			spell->mComponents.push_back(component);

			component->mItemID = itemID;
			EXPECTED_BOOL(Utility::stoSafe(component->mStacks, fields[SpellField::ComponentCount_1 + i]));
		}
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

const bool XMLDataStore::loadItems(ItemData* pItemData, uint32& pNumItemsLoaded) {
	using namespace ItemDataXML;
#ifdef PROFILE_XML_DS
	Profile p(String(__FUNCTION__), mLog);
#endif
	if (!pItemData) return false;

	// Load document.
	TiXmlDocument document(ItemDataXML::FileLocation);
	if (!document.LoadFile()) {
		mLog->error("Failed to load " + String(ItemDataXML::FileLocation));
		return false;
	}

	pNumItemsLoaded = 0;
	return true;
}

namespace ZoneXML {
#define SCA static const auto
	SCA FileLocation = "./data/zones.xml";
	namespace Tag {
		SCA Zones = "zones";
		SCA Zone = "zone";
		SCA ZonePoints = "zone_points";
		SCA ZonePoint = "zone_point";
		SCA SpawnPoints = "spawn_points";
		SCA SpawnPoint = "spawn_point";
		SCA SpawnGroups = "spawn_groups";
		SCA SpawnGroup = "spawn_group";
		SCA SpawnGroupEntry = "spawn_group_entry";
		SCA Fog = "fog";
		SCA Weather = "weather";
		SCA Objects = "objects";
		SCA Object = "object";
		SCA Doors = "doors";
		SCA Door = "door";
	}
	namespace Attribute {
		// Tag::Zone
		SCA ID = "id";
		SCA ShortName = "short_name";
		SCA LongName = "long_name";
		SCA ZoneType = "zone_type";
		SCA TimeType = "time_type";
		SCA SkyType = "sky_type";
		SCA MinimumClip = "minimum_clip";
		SCA MaximumClip = "maximum_clip";
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
		// Tag::Object
		namespace Object {
			SCA Type = "type";
			SCA Asset = "asset";
			SCA Heading = "heading";
			SCA Size = "size";
		}
		// Tag::Door
		namespace Door {
			SCA Asset = "asset";
			SCA Heading = "heading";
		}
		// Tag::SpawnGroup
		SCA SGID = "id";
		// Tag::SpawnGroupEntry
		SCA SGENPCType = "npc_type";
		SCA SGEChance = "chance";
		// Tag::SpawnPoint
		SCA SPSpawnGroup = "spawn_group";
		SCA SPRespawn = "respawn";
		SCA SPX = "x";
		SCA SPY = "y";
		SCA SPZ = "z";
		SCA SPHeading = "heading";
		// Tag::Fog / Tag::Weather
		SCA Index = "index";
		// Tag::Fog
		SCA Density = "density";
		SCA Red = "r";
		SCA Green = "g";
		SCA Blue = "b";
		SCA Minimum = "minimum";
		SCA Maximum = "maximum";
		// Tag::Weather
		SCA RainChance = "rain_chance";
		SCA RainDuration = "rain_duration";
		SCA SnowChance = "snow_chance";
		SCA SnowDuration = "snow_duration";
	}
#undef SCA
}
const bool XMLDataStore::loadZones(Data::ZoneList pZones) {
	using namespace ZoneXML;
#ifdef PROFILE_XML_DS
	Profile p(String(__FUNCTION__), mLog);
#endif
	if (!pZones.empty()) return false;

	// Load document.
	TiXmlDocument document(ZoneXML::FileLocation);
	if (!document.LoadFile()) {
		mLog->error("Failed to load " + String(ZoneXML::FileLocation));
		return false;
	}

	auto zonesElement = document.FirstChildElement(Tag::Zones);
	EXPECTED_BOOL(zonesElement);
	auto zoneElement = zonesElement->FirstChildElement(Tag::Zone);

	while (zoneElement) {
		auto zoneData = new Data::Zone();
		pZones.push_back(zoneData);

		EXPECTED_BOOL(readAttribute(zoneElement, Attribute::ID, zoneData->mID));
		EXPECTED_BOOL(readAttribute(zoneElement, Attribute::ShortName, zoneData->mShortName));
		EXPECTED_BOOL(readAttribute(zoneElement, Attribute::LongName, zoneData->mLongName));
		EXPECTED_BOOL(readAttribute(zoneElement, Attribute::ZoneType, zoneData->mZoneType));
		EXPECTED_BOOL(readAttribute(zoneElement, Attribute::TimeType, zoneData->mTimeType));
		EXPECTED_BOOL(readAttribute(zoneElement, Attribute::SkyType, zoneData->mSkyType));
		EXPECTED_BOOL(readAttribute(zoneElement, Attribute::MinimumClip, zoneData->mMinimumClip));
		EXPECTED_BOOL(readAttribute(zoneElement, Attribute::MaximumClip, zoneData->mMaximumClip));
		EXPECTED_BOOL(readAttribute(zoneElement, Attribute::SafeX, zoneData->mSafePosition.x));
		EXPECTED_BOOL(readAttribute(zoneElement, Attribute::SafeY, zoneData->mSafePosition.y));
		EXPECTED_BOOL(readAttribute(zoneElement, Attribute::SafeZ, zoneData->mSafePosition.z));

		// Read Zone Fog.
		auto fogParentElement = zoneElement->FirstChildElement(Tag::Fog);
		EXPECTED_BOOL(fogParentElement);
		EXPECTED_BOOL(readAttribute(fogParentElement, Attribute::Density, zoneData->mFogDensity));

		auto fogElement = fogParentElement->FirstChildElement(Tag::Fog);
		while (fogElement) {
			u8 index = 0;
			EXPECTED_BOOL(readAttribute(fogElement, Attribute::Index, index));
			EXPECTED_BOOL(index <= 3);

			EXPECTED_BOOL(readAttribute(fogElement, Attribute::Red, zoneData->mFog[index].mRed));
			EXPECTED_BOOL(readAttribute(fogElement, Attribute::Green, zoneData->mFog[index].mGreen));
			EXPECTED_BOOL(readAttribute(fogElement, Attribute::Blue, zoneData->mFog[index].mBlue));
			EXPECTED_BOOL(readAttribute(fogElement, Attribute::Minimum, zoneData->mFog[index].mMinimumClip));
			EXPECTED_BOOL(readAttribute(fogElement, Attribute::Maximum, zoneData->mFog[index].mMaximumClip));

			fogElement = fogElement->NextSiblingElement(Tag::Fog);
		}

		// Read Zone Weather.
		auto weatherParentElement = zoneElement->FirstChildElement(Tag::Weather);
		// Optional for now.
		if (weatherParentElement) {
			auto weatherElement = weatherParentElement->FirstChildElement(Tag::Weather);
			EXPECTED_BOOL(weatherElement);
			while (weatherElement) {
				u8 index = 0;
				EXPECTED_BOOL(readAttribute(weatherElement, Attribute::Index, index));
				EXPECTED_BOOL(index <= 3);

				EXPECTED_BOOL(readAttribute(weatherElement, Attribute::RainChance, zoneData->mWeather[index].mRainChance));
				EXPECTED_BOOL(readAttribute(weatherElement, Attribute::RainDuration, zoneData->mWeather[index].mRainDuration));
				EXPECTED_BOOL(readAttribute(weatherElement, Attribute::SnowChance, zoneData->mWeather[index].mSnowChance));
				EXPECTED_BOOL(readAttribute(weatherElement, Attribute::SnowDuration, zoneData->mWeather[index].mSnowDuration));

				weatherElement = weatherElement->NextSiblingElement(Tag::Weather);
			}
		}

		// Read Zone Points.
		auto zonePointsElement = zoneElement->FirstChildElement(Tag::ZonePoints);
		EXPECTED_BOOL(zonePointsElement);
		auto zonePointElement = zonePointsElement->FirstChildElement(Tag::ZonePoint);
		while (zonePointElement) {
			// Read Zone Point.
			auto zp = new Data::ZonePoint();
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

		// Read Objects.
		auto objectsElement = zoneElement->FirstChildElement(Tag::Objects);
		EXPECTED_BOOL(objectsElement);
		auto objectElement = objectsElement->FirstChildElement(Tag::Object);
		while (objectElement) {
			auto o = new Data::Object();
			zoneData->mObjects.push_back(o);

			EXPECTED_BOOL(readAttribute(objectElement, Attribute::Object::Type, o->mType));
			EXPECTED_BOOL(readAttribute(objectElement, Attribute::Object::Asset, o->mAsset));
			EXPECTED_BOOL(readAttribute(objectElement, Attribute::Object::Size, o->mSize));
			EXPECTED_BOOL(readVector3(objectElement, o->mPosition));
			EXPECTED_BOOL(readAttribute(objectElement, Attribute::Object::Heading, o->mHeading));

			objectElement = objectElement->NextSiblingElement(Tag::Object);
		}

		// Read Doors.
		auto doorsElement = zoneElement->FirstChildElement(Tag::Doors);
		EXPECTED_BOOL(doorsElement);
		auto doorElement = doorsElement->FirstChildElement(Tag::Door);
		while (doorElement) {
			auto d = new Data::Door();
			zoneData->mDoors.push_back(d);

			EXPECTED_BOOL(readAttribute(doorElement, Attribute::Door::Asset, d->mAsset));
			EXPECTED_BOOL(readVector3(doorElement, d->mPosition));
			EXPECTED_BOOL(readAttribute(doorElement, Attribute::Door::Heading, d->mHeading));

			doorElement = doorElement->NextSiblingElement(Tag::Door);
		}

		// Read Spawn Groups.
		auto spawnGroupsElement = zoneElement->FirstChildElement(Tag::SpawnGroups);
		EXPECTED_BOOL(spawnGroupsElement);
		auto spawnGroupElement = spawnGroupsElement->FirstChildElement(Tag::SpawnGroup);
		while (spawnGroupElement) {
			// Read Spawn Group.
			auto sg = new Data::SpawnGroup();
			zoneData->mSpawnGroups.push_back(sg);

			EXPECTED_BOOL(readAttribute(spawnGroupElement, Attribute::SGID, sg->mID));

			// Read Spawn Group Entries.
			auto spawnGroupEntryElement = spawnGroupElement->FirstChildElement(Tag::SpawnGroupEntry);
			EXPECTED_BOOL(spawnGroupEntryElement); // At least one entry is required.
			while (spawnGroupEntryElement) {
				// Read Spawn Group Entry.
				auto sge = new Data::SpawnGroup::Entry();
				sg->mEntries.push_back(sge);

				EXPECTED_BOOL(readAttribute(spawnGroupEntryElement, Attribute::SGENPCType, sge->mNPCType));
				EXPECTED_BOOL(readAttribute(spawnGroupEntryElement, Attribute::SGEChance, sge->mChance));

				spawnGroupEntryElement = spawnGroupEntryElement->NextSiblingElement(Tag::SpawnGroupEntry);
			}

			spawnGroupElement = spawnGroupElement->NextSiblingElement(Tag::SpawnGroup);
		}

		// Read Spawn Points.
		auto spawnPointsElement = zoneElement->FirstChildElement(Tag::SpawnPoints);
		EXPECTED_BOOL(spawnPointsElement);
		auto spawnPointElement = spawnPointsElement->FirstChildElement(Tag::SpawnPoint);
		while (spawnPointElement) {
			auto sp = new Data::SpawnPoint();
			zoneData->mSpawnPoints.push_back(sp);

			// Required Attributes.
			EXPECTED_BOOL(readAttribute(spawnPointElement, Attribute::SPSpawnGroup, sp->mSpawnGroupID));
			EXPECTED_BOOL(readAttribute(spawnPointElement, Attribute::SPRespawn, sp->mRespawnTime));
			EXPECTED_BOOL(readAttribute(spawnPointElement, Attribute::SPX, sp->mPosition.x));
			EXPECTED_BOOL(readAttribute(spawnPointElement, Attribute::SPY, sp->mPosition.y));
			EXPECTED_BOOL(readAttribute(spawnPointElement, Attribute::SPZ, sp->mPosition.z));
			EXPECTED_BOOL(readAttribute(spawnPointElement, Attribute::SPHeading, sp->mHeading));

			spawnPointElement = spawnPointElement->NextSiblingElement(Tag::SpawnPoint);
		}

		zoneElement = zoneElement->NextSiblingElement(Tag::Zone);
	}
	return true;
}

const bool XMLDataStore::saveZones(Data::ZoneList pZones) {
	using namespace ZoneXML;
#ifdef PROFILE_XML_DS
	Profile p(String(__FUNCTION__), mLog);
#endif
	TiXmlDocument document(ZoneXML::FileLocation);

	auto zonesElement = new TiXmlElement(Tag::Zones);
	document.LinkEndChild(zonesElement);

	for (auto i : pZones) {
		// Zone.
		auto zoneElement = new TiXmlElement(Tag::Zone);
		zonesElement->LinkEndChild(zoneElement);

		zoneElement->SetAttribute(Attribute::ID, i->mID);
		zoneElement->SetAttribute(Attribute::ShortName, i->mShortName.c_str());
		zoneElement->SetAttribute(Attribute::LongName, i->mLongName.c_str());
		zoneElement->SetAttribute(Attribute::ZoneType, i->mZoneType);
		zoneElement->SetAttribute(Attribute::TimeType, i->mTimeType);
		zoneElement->SetAttribute(Attribute::SkyType, i->mSkyType);
		zoneElement->SetDoubleAttribute(Attribute::MinimumClip, i->mMinimumClip);
		zoneElement->SetDoubleAttribute(Attribute::MaximumClip, i->mMaximumClip);
		zoneElement->SetDoubleAttribute(Attribute::SafeX, i->mSafePosition.x);
		zoneElement->SetDoubleAttribute(Attribute::SafeY, i->mSafePosition.y);
		zoneElement->SetDoubleAttribute(Attribute::SafeZ, i->mSafePosition.z);

		// Write Zone Fog.
		auto fogParentElement = new TiXmlElement(Tag::Fog);
		zoneElement->LinkEndChild(fogParentElement);
		fogParentElement->SetDoubleAttribute(Attribute::Density, i->mFogDensity);
		for (auto j = 0; j < 4; j++) {
			// Write Fog.
			auto fogElement = new TiXmlElement(Tag::Fog);
			fogParentElement->LinkEndChild(fogElement);

			fogElement->SetAttribute(Attribute::Index, j);
			fogElement->SetAttribute(Attribute::Red, i->mFog[j].mRed);
			fogElement->SetAttribute(Attribute::Green, i->mFog[j].mGreen);
			fogElement->SetAttribute(Attribute::Blue, i->mFog[j].mBlue);
			fogElement->SetDoubleAttribute(Attribute::Minimum, i->mFog[j].mMinimumClip);
			fogElement->SetDoubleAttribute(Attribute::Maximum, i->mFog[j].mMaximumClip);
		}

		// Write Zone Weather.
		auto weatherParentElement = new TiXmlElement(Tag::Weather);
		zoneElement->LinkEndChild(weatherParentElement);
		for (auto j = 0; j < 4; j++) {
			// Write Weather.
			auto weatherElement = new TiXmlElement(Tag::Weather);
			weatherParentElement->LinkEndChild(weatherElement);

			weatherElement->SetAttribute(Attribute::Index, j);
			weatherElement->SetAttribute(Attribute::RainChance, i->mWeather[j].mRainChance);
			weatherElement->SetAttribute(Attribute::RainDuration, i->mWeather[j].mRainDuration);
			weatherElement->SetAttribute(Attribute::SnowChance, i->mWeather[j].mSnowChance);
			weatherElement->SetAttribute(Attribute::SnowDuration, i->mWeather[j].mSnowDuration);
		}

		// Write Zone Points.
		auto zonePointsElement = new TiXmlElement(Tag::ZonePoints);
		zoneElement->LinkEndChild(zonePointsElement);

		for (auto j : i->mZonePoints) {
			// Write Zone Point.
			auto zonePointElement = new TiXmlElement(Tag::ZonePoint);
			zonePointsElement->LinkEndChild(zonePointElement);

			zonePointElement->SetAttribute(Attribute::ZPID, j->mID);
			zonePointElement->SetDoubleAttribute(Attribute::ZPX, j->mPosition.x);
			zonePointElement->SetDoubleAttribute(Attribute::ZPY, j->mPosition.y);
			zonePointElement->SetDoubleAttribute(Attribute::ZPZ, j->mPosition.z);
			zonePointElement->SetAttribute(Attribute::ZPDestZoneID, j->mDestinationZoneID);
			zonePointElement->SetAttribute(Attribute::ZPDestInstanceID, j->mDestinationInstanceID);
			zonePointElement->SetDoubleAttribute(Attribute::ZPDestX, j->mDestinationPosition.x);
			zonePointElement->SetDoubleAttribute(Attribute::ZPDestY, j->mDestinationPosition.y);
			zonePointElement->SetDoubleAttribute(Attribute::ZPDestZ, j->mDestinationPosition.z);
			zonePointElement->SetDoubleAttribute(Attribute::ZPDestHeading, j->mDestinationHeading);
		}

		// Write Objects.
		auto objectsElement = new TiXmlElement(Tag::Objects);
		zoneElement->LinkEndChild(objectsElement);

		for (auto j : i->mObjects) {
			// Write Object.
			auto objectElement = new TiXmlElement(Tag::Object);
			objectsElement->LinkEndChild(objectElement);

			objectElement->SetAttribute(Attribute::Object::Type, j->mType);
			objectElement->SetAttribute(Attribute::Object::Asset, j->mAsset.c_str());
			objectElement->SetDoubleAttribute(Attribute::Object::Size, j->mSize);
			writeVector3(objectElement, j->mPosition);
			objectElement->SetDoubleAttribute(Attribute::Object::Heading, j->mHeading);
		}

		// Write Doors.
		auto doorsElement = new TiXmlElement(Tag::Doors);
		zoneElement->LinkEndChild(doorsElement);

		for (auto j : i->mDoors) {
			// Write Door.
			auto doorElement = new TiXmlElement(Tag::Door);
			doorElement->LinkEndChild(doorElement);

			doorElement->SetAttribute(Attribute::Door::Asset, j->mAsset.c_str());
			writeVector3(doorElement, j->mPosition);
			doorElement->SetDoubleAttribute(Attribute::Door::Heading, j->mHeading);
		}

		// Write Spawn Groups.
		auto spawnGroupsElement = new TiXmlElement(Tag::SpawnGroups);
		zoneElement->LinkEndChild(spawnGroupsElement);

		for (auto j : i->mSpawnGroups) {
			// Write Spawn Group.
			auto spawnGroupElement = new TiXmlElement(Tag::SpawnGroup);
			spawnGroupsElement->LinkEndChild(spawnGroupElement);

			spawnGroupElement->SetAttribute(Attribute::SGID, j->mID);

			// Write Spawn Group Entries.
			for (auto k : j->mEntries) {
				// Write Spawn Group Entry.
				auto spawnGroupEntryElement = new TiXmlElement(Tag::SpawnGroupEntry);
				spawnGroupElement->LinkEndChild(spawnGroupEntryElement);

				spawnGroupEntryElement->SetAttribute(Attribute::SGENPCType, k->mNPCType);
				spawnGroupEntryElement->SetAttribute(Attribute::SGEChance, k->mChance);
			}
		}

		// Write Spawn Points.
		auto spawnPointsElement = new TiXmlElement(Tag::SpawnPoints);
		zoneElement->LinkEndChild(spawnPointsElement);

		for (auto j : i->mSpawnPoints) {
			// Write Spawn Point.
			auto spawnPointElement = new TiXmlElement(Tag::SpawnPoint);
			spawnPointsElement->LinkEndChild(spawnPointElement);

			spawnPointElement->SetAttribute(Attribute::SPSpawnGroup, j->mSpawnGroupID);
			spawnPointElement->SetAttribute(Attribute::SPRespawn, j->mRespawnTime);
			spawnPointElement->SetDoubleAttribute(Attribute::SPX, j->mPosition.x);
			spawnPointElement->SetDoubleAttribute(Attribute::SPY, j->mPosition.y);
			spawnPointElement->SetDoubleAttribute(Attribute::SPZ, j->mPosition.z);
			spawnPointElement->SetDoubleAttribute(Attribute::SPHeading, j->mHeading);
		}
	}

	EXPECTED_BOOL(document.SaveFile());
	return true;
}

namespace TransmutationComponentXML {
#define SCA static const auto
	SCA FileLocation = "./data/transmutation.xml";
	namespace Tag {
		SCA Components = "components";
		SCA Component = "component";
	}
	namespace Attribute {
		// Tag::Component
		SCA ItemID = "item_id";
		SCA Attribute = "attribute";
		SCA Minimum = "min";
		SCA Maximum = "max";
		SCA RequiredLevel = "level";
	}
#undef SCA
}
const bool XMLDataStore::loadTransmutationComponents(Data::TransmutationComponentList pComponents) {
	using namespace TransmutationComponentXML;
#ifdef PROFILE_XML_DS
	Profile p(String(__FUNCTION__), mLog);
#endif
	if (!pComponents.empty()) return false;

	// Load document.
	TiXmlDocument document(TransmutationComponentXML::FileLocation);
	if (!document.LoadFile()) {
		mLog->error("Failed to load " + String(TransmutationComponentXML::FileLocation));
		return false;
	}

	auto componentsElement = document.FirstChildElement(Tag::Components);
	EXPECTED_BOOL(componentsElement);
	auto componentElement = componentsElement->FirstChildElement(Tag::Component);

	while (componentElement) {
		auto c = new Data::TransmutationComponent();
		pComponents.push_back(c);

		EXPECTED_BOOL(readAttribute(componentElement, Attribute::ItemID, c->mItemID));
		EXPECTED_BOOL(readAttribute(componentElement, Attribute::Attribute, c->mAttribute));
		EXPECTED_BOOL(readAttribute(componentElement, Attribute::Minimum, c->mMinimum));
		EXPECTED_BOOL(readAttribute(componentElement, Attribute::Maximum, c->mMaximum));
		EXPECTED_BOOL(readAttribute(componentElement, Attribute::RequiredLevel, c->mRequiredLevel));

		componentElement = componentElement->NextSiblingElement(Tag::Component);
	}

	return true;
}

namespace AlternateCurrencyXML {
#define SCA static const auto
	SCA FileLocation = "./data/alternate_currency.xml";
	namespace Tag {
		SCA Currencies = "currencies";
		SCA Currency = "currency";
	}
	namespace Attribute {
		// Tag::Currency
		SCA ID = "id";
		SCA ItemID = "item_id";
		SCA Icon = "icon";
		SCA MaxStacks = "max_stacks";
	}
#undef SCA
}
const bool XMLDataStore::loadAlternateCurrencies(Data::AlternateCurrencyList pCurrencies) {
	using namespace AlternateCurrencyXML;
#ifdef PROFILE_XML_DS
	Profile p(String(__FUNCTION__), mLog);
#endif
	if (!pCurrencies.empty()) return false;

	// Load document.
	TiXmlDocument document(AlternateCurrencyXML::FileLocation);
	if (!document.LoadFile()) {
		mLog->error("Failed to load " + String(AlternateCurrencyXML::FileLocation));
		return false;
	}

	auto currenciesElement = document.FirstChildElement(Tag::Currencies);
	EXPECTED_BOOL(currenciesElement);
	auto currencyElement = currenciesElement->FirstChildElement(Tag::Currency);

	while (currencyElement) {
		auto c = new Data::AlternateCurrency();
		pCurrencies.push_back(c);

		EXPECTED_BOOL(readAttribute(currencyElement, Attribute::ID, c->mCurrencyID));
		EXPECTED_BOOL(readAttribute(currencyElement, Attribute::ItemID, c->mItemID));
		EXPECTED_BOOL(readAttribute(currencyElement, Attribute::Icon, c->mIcon));
		EXPECTED_BOOL(readAttribute(currencyElement, Attribute::MaxStacks, c->mMaxStacks));

		currencyElement = currencyElement->NextSiblingElement(Tag::Currency);
	}
	return true;
}

namespace ShopXML {
#define SCA static const auto
	SCA FileLocation = "./data/shops.xml";
	namespace Tag {
		SCA Shops = "shops";
		SCA Shop = "shop";
		SCA Item = "item";
	}
	namespace Attribute {
		// Tag::Shop
		SCA ID = "id";
		// Tag::Item
		SCA ItemID = "id";
		SCA Quantity = "quantity";
	}
#undef SCA
}
const bool XMLDataStore::loadShops(Data::ShopList pShops) {
	using namespace ShopXML;
#ifdef PROFILE_XML_DS
	Profile p(String(__FUNCTION__), mLog);
#endif
	if (!pShops.empty()) return false;

	// Load document.
	TiXmlDocument document(ShopXML::FileLocation);
	if (!document.LoadFile()) {
		mLog->error("Failed to load " + String(ShopXML::FileLocation));
		return false;
	}

	auto shopsElement = document.FirstChildElement(Tag::Shops);
	EXPECTED_BOOL(shopsElement);
	auto shopElement = shopsElement->FirstChildElement(Tag::Shop);
	
	while (shopElement) {
		auto s = new Data::Shop();
		pShops.push_back(s);

		EXPECTED_BOOL(readAttribute(shopElement, Attribute::ID, s->mID));

		auto itemElement = shopElement->FirstChildElement(Tag::Item);
		EXPECTED_BOOL(itemElement);
		while (itemElement) {
			uint32 itemID = 0;
			EXPECTED_BOOL(readAttribute(itemElement, Attribute::ItemID, itemID));
			int32 quantity = 0;
			EXPECTED_BOOL(readAttribute(itemElement, Attribute::Quantity, quantity));

			s->mItems.insert(std::make_pair(itemID, quantity));
			itemElement = itemElement->NextSiblingElement(Tag::Item);
		}

		shopElement = shopElement->NextSiblingElement(Tag::Shop);
	}

	return true;
}

namespace GuildXML {
#define SCA static const auto
	SCA FileLocation = "./data/guilds.xml";
	namespace Tag {
		SCA Guilds = "guilds";
		SCA Guild = "guild";
		SCA Members = "members";
		SCA Member = "member";
	}
	namespace Attribute {
		// Tag::Guild
		SCA ID = "id";
		SCA Name = "name";
		SCA MOTD = "motd";
		SCA MOTDSetter = "motd_setter";
		SCA URL = "url";
		SCA Channel = "channel";
		// Tag::Member
		namespace Member {
			SCA Name = "name";
			SCA Rank = "rank";
			SCA Level = "level";
			SCA Flags = "flags";
			SCA Class = "class";
			SCA TimeLastOn = "time_last_on";
			SCA TributeEnabled = "tribute_enabled";
			SCA TotalTribute = "total_tribtue";
			SCA LastTribute = "last_tribute";
			SCA PublicNote = "public_note";
		}
	}
#undef SCA
}
 // <member id="170123" name="Playerzero" rank="2" level="7" banker="0" class="1" time_last_on="0" tribute_enabled="0" total_tribute="0" last_tribute="0" public_note="" />
const bool XMLDataStore::readGuildMember(TiXmlElement* pElement, Data::GuildMember* pMember) {
	using namespace GuildXML;
#ifdef PROFILE_XML_DS
	Profile p(String(__FUNCTION__), mLog);
#endif
	if (!pElement) return false;
	if (!pMember) return false;

	EXPECTED_BOOL(readAttribute(pElement, Attribute::Member::Name, pMember->mName));
	EXPECTED_BOOL(readAttribute(pElement, Attribute::Member::Rank, pMember->mRank));
	EXPECTED_BOOL(readAttribute(pElement, Attribute::Member::Level, pMember->mLevel));
	EXPECTED_BOOL(readAttribute(pElement, Attribute::Member::Flags, pMember->mFlags));
	EXPECTED_BOOL(readAttribute(pElement, Attribute::Member::TributeEnabled, pMember->mTributeEnabled));
	EXPECTED_BOOL(readAttribute(pElement, Attribute::Member::Class, pMember->mClass));
	EXPECTED_BOOL(readAttribute(pElement, Attribute::Member::TimeLastOn, pMember->mTimeLastOn));
	EXPECTED_BOOL(readAttribute(pElement, Attribute::Member::TotalTribute, pMember->mTotalTribute));
	EXPECTED_BOOL(readAttribute(pElement, Attribute::Member::LastTribute, pMember->mLastTribute));
	EXPECTED_BOOL(readAttribute(pElement, Attribute::Member::PublicNote, pMember->mPublicNote));

	return true;
}

const bool XMLDataStore::readGuild(TiXmlElement* pElement, Data::Guild* pGuild) {
	using namespace GuildXML;
#ifdef PROFILE_XML_DS
	Profile p(String(__FUNCTION__), mLog);
#endif
	if (!pElement) return false;
	if (!pGuild) return false;

	EXPECTED_BOOL(readAttribute(pElement, Attribute::ID, pGuild->mID));
	EXPECTED_BOOL(readAttribute(pElement, Attribute::Name, pGuild->mName));
	EXPECTED_BOOL(readAttribute(pElement, Attribute::MOTD, pGuild->mMOTD));
	EXPECTED_BOOL(readAttribute(pElement, Attribute::MOTDSetter, pGuild->mMOTDSetter));
	EXPECTED_BOOL(readAttribute(pElement, Attribute::URL, pGuild->mURL));
	EXPECTED_BOOL(readAttribute(pElement, Attribute::Channel, pGuild->mChannel));

	auto membersElement = pElement->FirstChildElement(Tag::Members);
	EXPECTED_BOOL(membersElement);
	auto memberElement = membersElement->FirstChildElement(Tag::Member);

	// Read Data::GuidMember
	while (memberElement) {
		auto member = new Data::GuildMember();
		pGuild->mMembers.push_back(member);

		if (!readGuildMember(memberElement, member)) {
			return false;
		}

		memberElement = memberElement->NextSiblingElement(Tag::Member);
	}

	return true;
}

const bool XMLDataStore::loadGuilds(Data::GuildList pGuilds) {
	using namespace GuildXML;
#ifdef PROFILE_XML_DS
	Profile p(String(__FUNCTION__), mLog);
#endif
	if (!pGuilds.empty()) return false;

	// Load document.
	TiXmlDocument document(GuildXML::FileLocation);
	if (!document.LoadFile()) {
		mLog->error("Failed to load " + String(GuildXML::FileLocation));
		return false;
	}

	auto guildsElement = document.FirstChildElement(Tag::Guilds);
	EXPECTED_BOOL(guildsElement);
	auto guildElement = guildsElement->FirstChildElement(Tag::Guild);

	// Read Data::Guild
	while (guildElement) {
		auto guild = new Data::Guild();
		pGuilds.push_back(guild);

		if (!readGuild(guildElement, guild)) {
			return false;
		}

		guildElement = guildElement->NextSiblingElement(Tag::Guild);
	}

	return true;
}

const bool XMLDataStore::writeGuildMember(TiXmlElement* pElement, Data::GuildMember* pMember) {
	using namespace GuildXML;
#ifdef PROFILE_XML_DS
	Profile p(String(__FUNCTION__), mLog);
#endif
	if (!pElement) return false;
	if (!pMember) return false;

	pElement->SetAttribute(Attribute::Member::Name, pMember->mName.c_str());
	pElement->SetAttribute(Attribute::Member::Rank, std::to_string(pMember->mRank).c_str());
	pElement->SetAttribute(Attribute::Member::Level, std::to_string(pMember->mLevel).c_str());
	pElement->SetAttribute(Attribute::Member::Flags, std::to_string(pMember->mFlags).c_str());
	pElement->SetAttribute(Attribute::Member::TributeEnabled, std::to_string(pMember->mTributeEnabled).c_str());
	pElement->SetAttribute(Attribute::Member::Class, std::to_string(pMember->mClass).c_str());
	pElement->SetAttribute(Attribute::Member::TimeLastOn, std::to_string(pMember->mTimeLastOn).c_str());
	pElement->SetAttribute(Attribute::Member::TotalTribute, std::to_string(pMember->mTotalTribute).c_str());
	pElement->SetAttribute(Attribute::Member::LastTribute, std::to_string(pMember->mLastTribute).c_str());
	pElement->SetAttribute(Attribute::Member::PublicNote, pMember->mPublicNote.c_str());

	return true;
}

const bool XMLDataStore::writeGuild(TiXmlElement* pElement, Data::Guild* pGuild) {
	using namespace GuildXML;
#ifdef PROFILE_XML_DS
	Profile p(String(__FUNCTION__), mLog);
#endif
	if (!pElement) return false;
	if (!pGuild) return false;

	pElement->SetAttribute(Attribute::ID, std::to_string(pGuild->mID).c_str());
	pElement->SetAttribute(Attribute::Name, pGuild->mName.c_str());
	pElement->SetAttribute(Attribute::MOTD, pGuild->mMOTD.c_str());
	pElement->SetAttribute(Attribute::MOTDSetter, pGuild->mMOTDSetter.c_str());
	pElement->SetAttribute(Attribute::URL, pGuild->mURL.c_str());
	pElement->SetAttribute(Attribute::Channel, pGuild->mChannel.c_str());

	auto membersElement = new TiXmlElement(Tag::Members);
	pElement->LinkEndChild(membersElement);

	// Write Data::GuildMember.
	for (auto i : pGuild->mMembers) {
		auto memberElement = new TiXmlElement(Tag::Member);
		membersElement->LinkEndChild(memberElement);

		EXPECTED_BOOL(writeGuildMember(memberElement, i));
	}

	return true;
}

const bool XMLDataStore::saveGuilds(Data::GuildList pGuilds) {
	using namespace GuildXML;
#ifdef PROFILE_XML_DS
	Profile p(String(__FUNCTION__), mLog);
#endif
	TiXmlDocument document(GuildXML::FileLocation);

	auto guildsElement = new TiXmlElement(Tag::Guilds);
	document.LinkEndChild(guildsElement);

	// Write Data::Guild
	for (auto i : pGuilds) {
		auto guildElement = new TiXmlElement(Tag::Guild);
		guildsElement->LinkEndChild(guildElement);

		EXPECTED_BOOL(writeGuild(guildElement, i));
	}

	EXPECTED_BOOL(document.SaveFile());
	return true;
}

const bool XMLDataStore::loadTitles(Data::TitleList pTitles) {
	auto a = new Data::Title();
	a->mID = 1;
	a->mPrefix = "Apples";
	a->mSuffix = "Oranges";
	pTitles.push_back(a);

	auto b = new Data::Title();
	b->mID = 2;
	b->mPrefix = "Jam";
	pTitles.push_back(b);

	return true;
}

namespace TaskXML {
#define SCA static const auto
	SCA FileLocation = "./data/tasks.xml";
	namespace Tag {
		SCA Tasks = "tasks";
		SCA Task = "task";
		SCA Stage = "stage";
		//SCA Objectives = "objectives";
		SCA Objective = "objective";
	}
	namespace Attribute {
		// Tag::Task
		SCA ID = "id";
		SCA Type = "type";
		SCA Title = "title";
		SCA Description = "description";
		SCA RewardText = "reward_text";
		SCA Duration = "duration";
		SCA Repeatable = "repeatable";
		// Tag::Objective
		namespace Objective {
			SCA Type = "type";
			SCA TextA = "textA";
			SCA TextB = "textB";
			SCA ZoneID = "zone_id";
			SCA Required = "required";
			SCA Hidden = "hidden";
			SCA Optional = "optional";
		}
	}
#undef SCA
}
const bool XMLDataStore::loadTasks(Data::TaskDataArray& pTasks, const u32 pMaxTaskID, u32& pTasksLoaded) {
	using namespace TaskXML;
#ifdef PROFILE_XML_DS
	Profile p(String(__FUNCTION__), mLog);
#endif

	// Load document.
	TiXmlDocument document(TaskXML::FileLocation);
	if (!document.LoadFile()) {
		mLog->error("Failed to load " + String(TaskXML::FileLocation));
		return false;
	}

	auto tasksElement = document.FirstChildElement(Tag::Tasks);
	EXPECTED_BOOL(tasksElement);
	auto taskElement = tasksElement->FirstChildElement(Tag::Task);

	// Read Data::Task
	while (taskElement) {
		// Check: Task ID is valid.
		u32 taskID = 0;
		EXPECTED_BOOL(readAttribute(taskElement, Attribute::ID, taskID));
		if (taskID >= pMaxTaskID) return false; // Range.
		if (pTasks[taskID]->mID != 0) return false; // Duplicate ID.
		
		auto task = pTasks[taskID];
		if (!readTask(taskElement, task))
			return false;

		pTasksLoaded++;
		taskElement = taskElement->NextSiblingElement(Tag::Task);
	}

	return true;
}

const bool XMLDataStore::readTask(TiXmlElement* pElement, Data::Task* pTask) {
	using namespace TaskXML;
#ifdef PROFILE_XML_DS
	Profile p(String(__FUNCTION__), mLog);
#endif
	if (!pElement) return false;
	if (!pTask) return false;

	EXPECTED_BOOL(readAttribute(pElement, Attribute::ID, pTask->mID));
	EXPECTED_BOOL(readAttribute(pElement, Attribute::Type, pTask->mType));
	EXPECTED_BOOL(TaskType::isValid(pTask->mType));
	EXPECTED_BOOL(readAttribute(pElement, Attribute::Title, pTask->mTitle));
	EXPECTED_BOOL(readAttribute(pElement, Attribute::Description, pTask->mDescription));
	EXPECTED_BOOL(readAttribute(pElement, Attribute::RewardText, pTask->mRewardText));
	EXPECTED_BOOL(readAttribute(pElement, Attribute::Duration, pTask->mDuration));
	EXPECTED_BOOL(readAttribute(pElement, Attribute::Repeatable, pTask->mRepeatable));

	u32 objectiveID = 0;
	// Iterate stages.
	auto stageElement = pElement->FirstChildElement(Tag::Stage);
	while (stageElement) {
		auto stage = new Data::TaskStage();
		stage->mIndex = pTask->mStages.size();
		pTask->mStages.push_back(stage);

		auto objectiveElement = stageElement->FirstChildElement(Tag::Objective);

		// Read Data::TaskObjective
		while (objectiveElement) {
			auto objective = new Data::TaskObjective();
			objective->mIndex = objectiveID;
			objectiveID++;
			stage->mObjectives.push_back(objective);

			if (!readTaskObjective(objectiveElement, objective))
				return false;

			objectiveElement = objectiveElement->NextSiblingElement(Tag::Objective);
		}

		stageElement = stageElement->NextSiblingElement(Tag::Stage);
	}

	return true;
}

const bool XMLDataStore::readTaskObjective(TiXmlElement* pElement, Data::TaskObjective* pObjective) {
	using namespace TaskXML;
#ifdef PROFILE_XML_DS
	Profile p(String(__FUNCTION__), mLog);
#endif
	if (!pElement) return false;
	if (!pObjective) return false;

	EXPECTED_BOOL(readAttribute(pElement, Attribute::Objective::Type, pObjective->mType));
	EXPECTED_BOOL(ObjectiveType::isValid(pObjective->mType));
	EXPECTED_BOOL(readAttribute(pElement, Attribute::Objective::TextA, pObjective->mTextA));
	EXPECTED_BOOL(readAttribute(pElement, Attribute::Objective::TextB, pObjective->mTextB));
	EXPECTED_BOOL(readAttribute(pElement, Attribute::Objective::ZoneID, pObjective->mZoneID));
	EXPECTED_BOOL(readAttribute(pElement, Attribute::Objective::Required, pObjective->mRequired));
	EXPECTED_BOOL(readAttribute(pElement, Attribute::Objective::Hidden, pObjective->mHidden));
	EXPECTED_BOOL(readAttribute(pElement, Attribute::Objective::Optional, pObjective->mOptional));

	return true;
}
