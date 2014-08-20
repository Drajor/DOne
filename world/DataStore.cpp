#include "DataStore.h"
#include "Utility.h"
#include "Limits.h"

#include "../common/tinyxml/tinyxml.h"

static bool readRequiredAttribute(TiXmlElement* pElement, const String& pAttributeName, uint32& pAttributeValue) {
	if (!pElement) {
		Log::error("null pElement in readRequiredAttribute");
		return false;
	}
	const char* attribute = pElement->Attribute(pAttributeName.c_str());
	if (!attribute) {
		Log::error("attribute not found in readRequiredAttribute");
		return false;
	}
	return Utility::stou32Safe(pAttributeValue, String(attribute));
}

static bool readRequiredAttribute(TiXmlElement* pElement, const String& pAttributeName, String& pAttributeValue) {
	if (!pElement) {
		Log::error("null pElement in readRequiredAttribute");
		return false;
	}
	const char* attribute = pElement->Attribute(pAttributeName.c_str());
	if (!attribute) {
		Log::error("attribute not found in readRequiredAttribute");
		return false;
	}
	pAttributeValue = attribute;
	return true;
}

static bool readRequiredAttribute(TiXmlElement* pElement, const String& pAttributeName, int8& pAttributeValue) {
	if (!pElement) {
		Log::error("null pElement in readRequiredAttribute");
		return false;
	}
	const char* attribute = pElement->Attribute(pAttributeName.c_str());
	if (!attribute) {
		Log::error("attribute not found in readRequiredAttribute");
		return false;
	}
	return Utility::stoi8Safe(pAttributeValue, String(attribute));
}

static bool readRequiredAttribute(TiXmlElement* pElement, const String& pAttributeName, uint8& pAttributeValue) {
	if (!pElement) {
		Log::error("null pElement in readRequiredAttribute");
		return false;
	}
	const char* attribute = pElement->Attribute(pAttributeName.c_str());
	if (!attribute) {
		Log::error("attribute not found in readRequiredAttribute");
		return false;
	}
	return Utility::stou8Safe(pAttributeValue, String(attribute));
}

bool DataStore::initialise() {

	return true;
}

bool DataStore::loadAccounts(std::list<AccountData*>& pAccounts) {
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

		EXPECTED_BOOL(readRequiredAttribute(accountElement, "id", accountData->mAccountID));
		EXPECTED_BOOL(readRequiredAttribute(accountElement, "name", accountData->mAccountName));
		EXPECTED_BOOL(Limits::LoginServer::accountNameLength(accountData->mAccountName));
		EXPECTED_BOOL(readRequiredAttribute(accountElement, "status", accountData->mStatus));
		EXPECTED_BOOL(readRequiredAttribute(accountElement, "suspend_until", accountData->mSuspendedUntil));
		EXPECTED_BOOL(readRequiredAttribute(accountElement, "last_login", accountData->mLastLogin));
		EXPECTED_BOOL(readRequiredAttribute(accountElement, "created", accountData->mCreated));
		
		accountElement = accountElement->NextSiblingElement("account");
	}
	
	return true;
}

bool DataStore::saveAccounts(std::list<AccountData*>& pAccounts) {
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
		EXPECTED_BOOL(readRequiredAttribute(characterElement, "name", characterData->mName));
		EXPECTED_BOOL(readRequiredAttribute(characterElement, "race", characterData->mRace));
		EXPECTED_BOOL(readRequiredAttribute(characterElement, "class", characterData->mClass));
		EXPECTED_BOOL(readRequiredAttribute(characterElement, "level", characterData->mLevel));
		EXPECTED_BOOL(readRequiredAttribute(characterElement, "gender", characterData->mGender));
		EXPECTED_BOOL(readRequiredAttribute(characterElement, "deity", characterData->mDeity));
		EXPECTED_BOOL(readRequiredAttribute(characterElement, "zone", characterData->mZoneID));
		EXPECTED_BOOL(readRequiredAttribute(characterElement, "face", characterData->mFace));
		EXPECTED_BOOL(readRequiredAttribute(characterElement, "hair_style", characterData->mHairStyle));
		EXPECTED_BOOL(readRequiredAttribute(characterElement, "hair_colour", characterData->mHairColour));
		EXPECTED_BOOL(readRequiredAttribute(characterElement, "beard_style", characterData->mBeardStyle));
		EXPECTED_BOOL(readRequiredAttribute(characterElement, "beard_colour", characterData->mBeardColour));
		EXPECTED_BOOL(readRequiredAttribute(characterElement, "eye_colour1", characterData->mEyeColourLeft));
		EXPECTED_BOOL(readRequiredAttribute(characterElement, "eye_colour2", characterData->mEyeColourRight));
		EXPECTED_BOOL(readRequiredAttribute(characterElement, "drakkin_heritage", characterData->mDrakkinHeritage));
		EXPECTED_BOOL(readRequiredAttribute(characterElement, "drakkin_tattoo", characterData->mDrakkinTattoo));
		EXPECTED_BOOL(readRequiredAttribute(characterElement, "drakkin_details", characterData->mDrakkinDetails));

		// Read the equipment information about each character.
		auto equipmentElement = characterElement->FirstChildElement("equipment");
		EXPECTED_BOOL(equipmentElement);
		EXPECTED_BOOL(readRequiredAttribute(equipmentElement, "primary", characterData->mPrimary)); // IDFile of item in primary slot.
		EXPECTED_BOOL(readRequiredAttribute(equipmentElement, "secondary", characterData->mSecondary)); // IDFile of item in secondary slot.

		auto slotElement = equipmentElement->FirstChildElement("slot");
		auto slotCount = 0;
		EXPECTED_BOOL(slotElement);
		while (slotElement && slotCount < Limits::Account::MAX_EQUIPMENT_SLOTS) {
			EXPECTED_BOOL(readRequiredAttribute(slotElement, "material", characterData->mEquipment[slotCount].mMaterial));
			EXPECTED_BOOL(readRequiredAttribute(slotElement, "colour", characterData->mEquipment[slotCount].mColour));
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

bool DataStore::loadCharacter(const String& pCharacterName, CharacterData* pCharacterData)
{
	return true;
}

bool DataStore::saveCharacter(const String& pCharacterName, const CharacterData* pCharacterData)
{
	return true;
}




