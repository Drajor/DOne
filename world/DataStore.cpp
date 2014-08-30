#include "DataStore.h"
#include "Utility.h"
#include "Limits.h"
#include "Profile.h"

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
	pAttributeValue = attribute == "1";
	return true;
}

bool DataStore::initialise() {

	return true;
}

bool DataStore::loadAccounts(std::list<AccountData*>& pAccounts) {
	Profile p("DataStore::loadAccounts");
	EXPECTED_BOOL(pAccounts.empty());
	TiXmlDocument document("./data/accounts.xml");
	EXPECTED_BOOL(document.LoadFile());

	auto accountsElement = document.FirstChildElement("accounts");
	EXPECTED_BOOL(accountsElement);
	auto accountElement = accountsElement->FirstChildElement("account");
	
	// There are no accounts yet.
	if (!accountElement)
		return true;

	// Iterate over each "account" element.
	while (accountElement) {
		auto accountData = new AccountData();
		pAccounts.push_back(accountData);

		EXPECTED_BOOL(readAttribute(accountElement, "id", accountData->mAccountID));
		EXPECTED_BOOL(readAttribute(accountElement, "name", accountData->mAccountName));
		EXPECTED_BOOL(Limits::LoginServer::accountNameLength(accountData->mAccountName));
		EXPECTED_BOOL(readAttribute(accountElement, "status", accountData->mStatus));
		EXPECTED_BOOL(readAttribute(accountElement, "suspend_until", accountData->mSuspendedUntil));
		EXPECTED_BOOL(readAttribute(accountElement, "last_login", accountData->mLastLogin));
		EXPECTED_BOOL(readAttribute(accountElement, "created", accountData->mCreated));
		
		accountElement = accountElement->NextSiblingElement("account");
	}
	
	return true;
}

bool DataStore::saveAccounts(std::list<AccountData*>& pAccounts) {
	Profile p("DataStore::saveAccounts");
	TiXmlDocument document("./data/accounts.xml");

	auto accountsElement = new TiXmlElement("accounts");
	document.LinkEndChild(accountsElement);

	for (auto i : pAccounts) {
		auto accountElement = new TiXmlElement("account");
		accountsElement->LinkEndChild(accountsElement);

		accountElement->SetAttribute("id", std::to_string(i->mAccountID).c_str());
		accountElement->SetAttribute("name", i->mAccountName.c_str());
		accountElement->SetAttribute("status", i->mStatus);
		accountElement->SetAttribute("suspend_until", std::to_string(i->mSuspendedUntil).c_str());
		accountElement->SetAttribute("last_login", std::to_string(i->mLastLogin).c_str());
		accountElement->SetAttribute("created", std::to_string(i->mCreated).c_str());
	}

	EXPECTED_BOOL(document.SaveFile());
	return true;
}

bool DataStore::loadAccountCharacterData(AccountData* pAccount) {
	Profile p("DataStore::loadAccountCharacterData");
	EXPECTED_BOOL(pAccount);
	TiXmlDocument document(String("./data/accounts/" + pAccount->mAccountName + ".xml").c_str());
	EXPECTED_BOOL(document.LoadFile());

	auto accountElement = document.FirstChildElement("account");
	EXPECTED_BOOL(accountElement);
	auto charactersElement = accountElement->FirstChildElement("characters");
	EXPECTED_BOOL(charactersElement);
	auto characterElement = charactersElement->FirstChildElement("character");

	// There are no characters yet.
	if (!characterElement)
		return true;

	// Iterate over each "account" element.
	auto characterSlot = 0;
	while (characterElement && characterSlot < Limits::Account::MAX_NUM_CHARACTERS) {
		auto characterData = new AccountData::CharacterData();
		pAccount->mCharacterData.push_back(characterData);

		// Read the basic/visual information about each character.
		EXPECTED_BOOL(readAttribute(characterElement, "name", characterData->mName));
		EXPECTED_BOOL(readAttribute(characterElement, "race", characterData->mRace));
		EXPECTED_BOOL(readAttribute(characterElement, "class", characterData->mClass));
		EXPECTED_BOOL(readAttribute(characterElement, "level", characterData->mLevel));
		EXPECTED_BOOL(readAttribute(characterElement, "gender", characterData->mGender));
		EXPECTED_BOOL(readAttribute(characterElement, "deity", characterData->mDeity));
		EXPECTED_BOOL(readAttribute(characterElement, "zone", characterData->mZoneID));
		EXPECTED_BOOL(readAttribute(characterElement, "face", characterData->mFace));
		EXPECTED_BOOL(readAttribute(characterElement, "hair_style", characterData->mHairStyle));
		EXPECTED_BOOL(readAttribute(characterElement, "hair_colour", characterData->mHairColour));
		EXPECTED_BOOL(readAttribute(characterElement, "beard_style", characterData->mBeardStyle));
		EXPECTED_BOOL(readAttribute(characterElement, "beard_colour", characterData->mBeardColour));
		EXPECTED_BOOL(readAttribute(characterElement, "eye_colour1", characterData->mEyeColourLeft));
		EXPECTED_BOOL(readAttribute(characterElement, "eye_colour2", characterData->mEyeColourRight));
		EXPECTED_BOOL(readAttribute(characterElement, "drakkin_heritage", characterData->mDrakkinHeritage));
		EXPECTED_BOOL(readAttribute(characterElement, "drakkin_tattoo", characterData->mDrakkinTattoo));
		EXPECTED_BOOL(readAttribute(characterElement, "drakkin_details", characterData->mDrakkinDetails));

		// Read the equipment information about each character.
		auto equipmentElement = characterElement->FirstChildElement("equipment");
		EXPECTED_BOOL(equipmentElement);
		EXPECTED_BOOL(readAttribute(equipmentElement, "primary", characterData->mPrimary)); // IDFile of item in primary slot.
		EXPECTED_BOOL(readAttribute(equipmentElement, "secondary", characterData->mSecondary)); // IDFile of item in secondary slot.

		auto slotElement = equipmentElement->FirstChildElement("slot");
		auto slotCount = 0;
		EXPECTED_BOOL(slotElement);
		while (slotElement && slotCount < Limits::Account::MAX_EQUIPMENT_SLOTS) {
			EXPECTED_BOOL(readAttribute(slotElement, "material", characterData->mEquipment[slotCount].mMaterial));
			EXPECTED_BOOL(readAttribute(slotElement, "colour", characterData->mEquipment[slotCount].mColour));
			slotCount++;
			slotElement = slotElement->NextSiblingElement("slot");
		}
		EXPECTED_BOOL(slotCount == Limits::Account::MAX_EQUIPMENT_SLOTS); // Check that we read the correct amount of slot data.

		characterSlot++;
		characterElement = characterElement->NextSiblingElement("character");
	}

	return true;
}

bool DataStore::saveAccountCharacterData(AccountData* pAccount) {
	Profile p("DataStore::saveAccountCharacterData");
	EXPECTED_BOOL(pAccount);
	TiXmlDocument document(String("./data/accounts/" + pAccount->mAccountName + ".xml").c_str());

	auto accountElement = document.LinkEndChild(new TiXmlElement("account"));
	auto charactersElement = accountElement->LinkEndChild(new TiXmlElement("characters"));
	
	// Write 
	for (auto i : pAccount->mCharacterData) {
		auto characterElement = static_cast<TiXmlElement*>(charactersElement->LinkEndChild(new TiXmlElement("character")));

		characterElement->SetAttribute("name", i->mName.c_str());
		characterElement->SetAttribute("race", i->mRace);
		characterElement->SetAttribute("class", i->mClass);
		characterElement->SetAttribute("level", i->mLevel);
		characterElement->SetAttribute("gender", i->mGender);
		characterElement->SetAttribute("deity", i->mDeity);
		characterElement->SetAttribute("zone", i->mZoneID);
		characterElement->SetAttribute("face", i->mFace);
		characterElement->SetAttribute("gender", i->mGender);
		characterElement->SetAttribute("hair_style", i->mHairStyle);
		characterElement->SetAttribute("hair_colour", i->mHairColour);
		characterElement->SetAttribute("beard_style", i->mBeardStyle);
		characterElement->SetAttribute("beard_colour", i->mBeardColour);
		characterElement->SetAttribute("eye_colour1", i->mEyeColourLeft);
		characterElement->SetAttribute("eye_colour2", i->mEyeColourRight);
		characterElement->SetAttribute("drakkin_heritage", i->mDrakkinHeritage);
		characterElement->SetAttribute("drakkin_tattoo", i->mDrakkinTattoo);
		characterElement->SetAttribute("drakkin_details", i->mDrakkinDetails);

		auto equipmentElement = static_cast<TiXmlElement*>(characterElement->LinkEndChild(new TiXmlElement("equipment")));
		equipmentElement->SetAttribute("primary", i->mPrimary);
		equipmentElement->SetAttribute("secondary", i->mSecondary);

		// Write equipment material / colours.
		for (int j = 0; j < Limits::Account::MAX_EQUIPMENT_SLOTS; j++) {
			auto slotElement = static_cast<TiXmlElement*>(equipmentElement->LinkEndChild(new TiXmlElement("slot")));
			slotElement->SetAttribute("material", i->mEquipment[j].mMaterial);
			slotElement->SetAttribute("colour", i->mEquipment[j].mColour);
		}
	}

	EXPECTED_BOOL(document.SaveFile());
	return true;
}

namespace CharacterDataXML {
	namespace Tag {
		static const auto Character = "character";
		static const auto Stats = "stats";
		static const auto Visual = "visual";
		static const auto Dyes = "dyes";
		static const auto Dye = "dye";
		static const auto Guild = "guild";
		static const auto Currency = "currency";
		static const auto Crystals = "crystals";
		static const auto Radiant = "radiant";
		static const auto Ebon = "ebon";
	}
	namespace Attribute {
		// Tag::Character
		static const auto Name = "name";
		static const auto GM = "gm";
		static const auto Status = "status";
		static const auto Level = "level";
		static const auto Class = "class";
		static const auto Zone = "zone";
		static const auto X = "x";
		static const auto Y = "y";
		static const auto Z = "z";
		static const auto Heading = "heading";
		static const auto Experience = "experience";
		static const auto LastName = "last_name";
		static const auto Title = "title";
		static const auto Suffix = "suffix";
		static const auto AutoConsentGroup = "auto_consent_group";
		static const auto AutoConsentRaid = "auto_consent_raid";
		static const auto AutoConsentGuild = "auto_consent_guild";
		// Tag::Stats
		static const auto Strength = "strength";
		static const auto Stamina = "stamina";
		static const auto Charisma = "charisma";
		static const auto Dexterity = "dexterity";
		static const auto Intelligence = "intelligence";
		static const auto Agility = "agility";
		static const auto Wisdom = "wisdom";
		// Tag::Visual
		static const auto Race = "race";
		static const auto Gender = "gender";
		static const auto Face = "face";
		static const auto HairStyle = "hair_style";
		static const auto HairColour = "hair_colour";
		static const auto BeardStyle = "beard_style";
		static const auto BeardColour = "beard_colour";
		static const auto EyeColour1 = "eye_colour1";
		static const auto EyeColour2 = "eye_colour2";
		static const auto DrakkinHeritage = "drakkin_heritage";
		static const auto DrakkinTattoo = "drakkin_tattoo";
		static const auto DrakkinDetails = "drakkin_Details";
		// Tag::Dye
		static const auto Colour = "colour";
		// Tag::Guild
		static const auto GuildID = "id";
		static const auto GuildRank = "rank";
		// Tag::Currency
		static const auto PlatinumCharacter = "platinum_character";
		static const auto PlatinumBank = "platinum_bank";
		static const auto PlatinumCursor = "platinum_cursor";
		static const auto GoldCharacter = "gold_character";
		static const auto GoldBank = "gold_bank";
		static const auto GoldCursor = "gold_cursor";
		static const auto SilverCharacter = "silver_character";
		static const auto SilverBank = "silver_bank";
		static const auto SilverCursor = "silver_cursor";
		static const auto CopperCharacter = "copper_character";
		static const auto CopperBank = "copper_bank";
		static const auto CopperCursor = "copper_cursor";
		// Tag::Crystals / Radiant / Ebon
		static const auto Current = "current";
		static const auto Total = "total";
	}
}

bool DataStore::loadCharacter(const String& pCharacterName, CharacterData* pCharacterData) {
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
	EXPECTED_BOOL(readAttribute(visualElement, Attribute::Face, pCharacterData->mFace));
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

	return true;
}

bool DataStore::saveCharacter(const String& pCharacterName, const CharacterData* pCharacterData) {
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
	visualElement->SetAttribute(Attribute::Face, pCharacterData->mFace);
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

	EXPECTED_BOOL(document.SaveFile());
	return true;
}




