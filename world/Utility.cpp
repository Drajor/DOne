#include "Utility.h"
#include "Limits.h"
#include "Character.h"
#include "Zone.h"

#include "../common/MiscFunctions.h" // TEMP: For MakeRandomInt getRandomName()

#include <iostream>
#include <ctime>

void Utility::print(String pMessage)
{
	std::cout << pMessage << std::endl;
}

String Utility::characterLogDetails(Character* pCharacter) {
	if (pCharacter == nullptr) return "[NULL CHARACTER POINTER]";

	StringStream ss;
	ss << "[Character(Name: " << pCharacter->getName() << /*" Account ID: " << pCharacter->getWorldAccountID() <<*/ ")]";
	return ss.str();
}

String Utility::zoneLogDetails(Zone* pZone) {
	if (pZone == nullptr) return "[NULL ZONE POINTER]";

	StringStream ss;
	ss << "[Zone(Name: " << pZone->getLongName() << " ID:" << static_cast<uint16>(pZone->getID()) << " InstanceID:" << pZone->getInstanceID() << ")]";
	return ss.str();
}

String Utility::getRandomName() {
	// creates up to a 10 char name
	char vowels[18] = "aeiouyaeiouaeioe";
	char cons[48] = "bcdfghjklmnpqrstvwxzybcdgklmnprstvwbcdgkpstrkd";
	char rndname[17] = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
	char paircons[33] = "ngrkndstshthphsktrdrbrgrfrclcr";
	int rndnum = MakeRandomInt(0, 75), n = 1;
	bool dlc = false;
	bool vwl = false;
	bool dbl = false;
	if (rndnum > 63)
	{	// rndnum is 0 - 75 where 64-75 is cons pair, 17-63 is cons, 0-16 is vowel
		rndnum = (rndnum - 61) * 2;	// name can't start with "ng" "nd" or "rk"
		rndname[0] = paircons[rndnum];
		rndname[1] = paircons[rndnum + 1];
		n = 2;
	}
	else if (rndnum > 16)
	{
		rndnum -= 17;
		rndname[0] = cons[rndnum];
	}
	else
	{
		rndname[0] = vowels[rndnum];
		vwl = true;
	}
	int namlen = MakeRandomInt(5, 10);
	for (int i = n; i<namlen; i++)
	{
		dlc = false;
		if (vwl)	//last char was a vowel
		{			// so pick a cons or cons pair
			rndnum = MakeRandomInt(0, 62);
			if (rndnum>46)
			{	// pick a cons pair
				if (i > namlen - 3)	// last 2 chars in name?
				{	// name can only end in cons pair "rk" "st" "sh" "th" "ph" "sk" "nd" or "ng"
					rndnum = MakeRandomInt(0, 7) * 2;
				}
				else
				{	// pick any from the set
					rndnum = (rndnum - 47) * 2;
				}
				rndname[i] = paircons[rndnum];
				rndname[i + 1] = paircons[rndnum + 1];
				dlc = true;	// flag keeps second letter from being doubled below
				i += 1;
			}
			else
			{	// select a single cons
				rndname[i] = cons[rndnum];
			}
		}
		else
		{		// select a vowel
			rndname[i] = vowels[MakeRandomInt(0, 16)];
		}
		vwl = !vwl;
		if (!dbl && !dlc)
		{	// one chance at double letters in name
			if (!MakeRandomInt(0, i + 9))	// chances decrease towards end of name
			{
				rndname[i + 1] = rndname[i];
				dbl = true;
				i += 1;
			}
		}
	}

	rndname[0] = toupper(rndname[0]);

	return String(rndname);
}

i64 Utility::Time::now() {
	return std::time(nullptr);
}

const String& Utility::Skills::fromID(const uint32 pSkillID) {
	// Check: pSkillID is in range.
	if (!Limits::Skills::validID(pSkillID)) {
		static const String BadSkillID = "BadSkillID";
		Log::error("Skill ID out of range: " + std::to_string(pSkillID) );
		return BadSkillID;
	}

	return SkillStrings[pSkillID];
}

const String& Utility::Languages::fromID(const uint32 pLanguageID) {
	// Check: pLanguageID is in range.
	if (!Limits::Languages::validID(pLanguageID)) {
		static const String BadLanguageID = "BadLanguageID";
		Log::error("Language ID out of range: " + std::to_string(pLanguageID));
		return BadLanguageID;
	}

	return LanguagesStrings[pLanguageID];
}
