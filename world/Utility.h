#pragma once

#include "Constants.h"
#include "LogSystem.h"

#define ARG_STR(pARG) #pARG
#define EXPECTED(pCondition) if(!(pCondition))  { StringStream ss; ss << "[EXPECTED] ("<< ARG_STR(pCondition) << ") Failed in" << __FUNCTION__; Log::error(ss.str()); return; }
#define EXPECTED_VAR(pCondition, pReturn) if(!(pCondition))  { StringStream ss; ss << "[EXPECTED] ("<< ARG_STR(pCondition) << ") Failed in" << __FUNCTION__; Log::error(ss.str()); return pReturn; }
//#define EXPECTED_BOOL(pCondition) if(!(pCondition))  { StringStream ss; ss << "[EXPECTED] ("<< ARG_STR(pCondition) << ") Failed in" << __FUNCTION__; Log::error(ss.str()); return false; }
#define EXPECTED_BOOL(pCondition) { EXPECTED_VAR(pCondition, false) }
#define EXPECTED_PTR(pCondition) if(!(pCondition))  { StringStream ss; ss << "[EXPECTED] ("<< ARG_STR(pCondition) << ") Failed in" << __FUNCTION__; Log::error(ss.str()); return nullptr; }
#define EXPECTED_MAIN(pCondition) if(!(pCondition))  { StringStream ss; ss << "[EXPECTED] ("<< ARG_STR(pCondition) << ") Failed in" << __FUNCTION__; Log::error(ss.str()); return 1; }

#define PRINT_MEMBER(pMember) ss << ARG_STR(pMember) << " = " << pMember << " | ";

class Character;
class Zone;
namespace Utility {
	namespace Time {
		int32 now();
	}
	void print(String pMessage);
	String getRandomName();

	inline const bool isCaster(const uint8 pClassID) {
		switch (pClassID) {
		case ClassID::Warrior:
		case ClassID::Monk:
		case ClassID::Rogue:
		case ClassID::Berserker:
			return false;
		default:
			break;
		}
		return true;
	}

	// Hybrid: A class that can both cast spells and use disciplines.
	inline const bool isHybrid(const uint8 pClassID) {
		switch (pClassID) {
		case ClassID::Paladin:
		case ClassID::Ranger:
		case ClassID::Shadowknight:
		case ClassID::Bard:
		case ClassID::Beastlord:
			return true;
		default:
			break;
		}
		return false;
	}

	inline const bool isMelee(const uint8 pClassID) {
		switch (pClassID) {
		case ClassID::Warrior:
		case ClassID::Monk:
		case ClassID::Rogue:
		case ClassID::Berserker:
			return true;
		default:
			break;
		}
		return false;
	}

	inline const bool canClassTaunt(const uint8 pClassID) {
		switch (pClassID) {
		case ClassID::Warrior:
		case ClassID::Paladin:
		case ClassID::Shadowknight:
			return true;
		default:
			break;
		}
		return false;
	}

	String characterLogDetails(Character* pCharacter);
	String zoneLogDetails(Zone* pZone);
	String guildRankToString(const GuildRank& pRank);

	static String safeString(char* pCString, unsigned int pMaxSize) {
		// strnlen_s - http://msdn.microsoft.com/en-us/library/z50ty2zh.aspx
		std::size_t strLength = strnlen_s(pCString, pMaxSize);
		if (strLength == pMaxSize) {
			// Not null terminated.
			Log::error("[safeString] Got string missing null terminator.");
			return "";
		}
		return String(pCString);
	}

	static String StringIDString(StringID pStringID) {
		return std::to_string(static_cast<uint32>(pStringID));
	}

	class DynamicStructure {
	public:
		template <typename T>
		DynamicStructure(T pStart, int pSize) : mStart(reinterpret_cast<char*>(pStart)), mPointer(mStart), mSize(pSize), mWritten(0) {}

		template <typename T>
		void write(T pData) {
			if (mWritten + sizeof(T) <= mSize) {
				auto TPtr = reinterpret_cast<T*>(mPointer);
				*TPtr = pData;
				mPointer += sizeof(T);
				mWritten += sizeof(T);
			}
			else {
				Log::error("[Dynamic Structure] Attempt to write to invalid memory.");
			}
		}

		void writeChunk(void* pFrom, std::size_t pSize) {
			if (mWritten + pSize <= mSize) {
				memcpy(mPointer, pFrom, pSize);
				mPointer += pSize;
				mWritten += pSize;
			}
			else {
				Log::error("[Dynamic Structure] Attempt to write to invalid memory.");
			}
		}

		void writeString(String pData) {
			if (mWritten + pData.length() + 1 <= mSize) {
				strcpy(mPointer, pData.c_str());
				mPointer += pData.length() + 1;
				mWritten += pData.length() + 1;
			}
			else {
				Log::error("[Dynamic Structure] Attempt to write to invalid memory.");
			}
		}

		void writeFixedString(String pData, std::size_t pSize) {
			if (mWritten + pSize <= mSize) {
				strcpy(mPointer, pData.c_str());
				mPointer += pData.length() + 1;
				mWritten += pData.length() + 1;
				const std::size_t difference = pSize - (pData.length() + 1);
				memset(mPointer, 0, difference); // Fill remaining memory with zeros
				mPointer += difference;
				mWritten += difference;

			}
		}

		void movePointer(int pBytes) {
			// TODO: Check this!
			mPointer += pBytes;
			mWritten += pBytes; // I need to think about this more.
		}
		const std::size_t getSize() const { return mSize; }
		const std::size_t getBytesWritten() const { return mWritten; }
		const bool check() const { return mWritten == mSize; }
	private:
		std::size_t mSize; // maximum number of bytes to be written.
		std::size_t mWritten; // number of bytes written.
		char* mStart; // start memory location.
		char* mPointer; // current location in memory.
	};

	// list.remove_if(Utility::containerEntryDelete)
	template <typename T>
	bool containerEntryDelete(T pValue) {
		delete pValue;
		return true;
	}

	static bool stofSafe(float& pValue, String& pString) {
		try {
			pValue = std::stof(pString);
			return true;
		}
		catch (...) {
			return false;
		}
		return false;
	}

	static bool stou32Safe(uint32& pValue, String& pString) {
		try {
			pValue = std::stoul(pString);
			return true;
		}
		catch (...) {
			return false;
		}
		return false;
	}
	static bool stou16Safe(uint16& pValue, String pString) {
		try {
			pValue = static_cast<uint16>(std::stoul(pString));
			return true;
		}
		catch (...) {
			return false;
		}
		return false;
	}
	static bool stou8Safe(uint8& pValue, String& pString) {
		try {
			pValue = static_cast<uint8>(std::stoul(pString));
			return true;
		}
		catch (...) {
			return false;
		}
		return false;
	}
	static bool stoi8Safe(int8& pValue, String& pString) {
		try {
			pValue = static_cast<int8>(std::stoi(pString));
			return true;
		}
		catch (...) {
			return false;
		}
		return false;
	}
	static bool stobool(String& pString) {
		// TODO: http://msdn.microsoft.com/en-us/library/chd90w8e%28VS.80%29.aspx
		if (pString == "true" || pString == "1")
			return true;
		return false;
	}

	namespace stdext {
		inline int32_t stoi(const String& pString) { return std::stoi(pString, 0); }
		inline uint32_t stou(const String& pString) { return std::stoul(pString, 0); }
		inline float stof(const String& pString) { return std::stof(pString, 0); }
		inline double stod(const String& pString) { return std::stod(pString, 0); }


		template <typename T, typename F>
		inline bool stoExecute(T& pValue, const std::string& pString, F&& pFunction) {
			try {
				pValue = static_cast<T>(pFunction(pString));
				return true;
			}
			catch (...) {
				return false;
			}
		}
	}

	inline bool stoSafe(int32_t& pValue, const String& pString) { return stdext::stoExecute(pValue, pString, stdext::stoi); }
	inline bool stoSafe(int16_t& pValue, const String& pString) { return stdext::stoExecute(pValue, pString, stdext::stoi); }
	inline bool stoSafe(int8_t& pValue, const String& pString) { return stdext::stoExecute(pValue, pString, stdext::stoi); }
	inline bool stoSafe(uint32_t& pValue, const String& pString) { return stdext::stoExecute(pValue, pString, stdext::stou); }
	inline bool stoSafe(uint16_t& pValue, const String& pString) { return stdext::stoExecute(pValue, pString, stdext::stou); }
	inline bool stoSafe(uint8_t& pValue, const String& pString) { return stdext::stoExecute(pValue, pString, stdext::stou); }
	inline bool stoSafe(float& pValue, const String& pString) { return stdext::stoExecute(pValue, pString, stdext::stof); }
	inline bool stoSafe(double& pValue, const String& pString) { return stdext::stoExecute(pValue, pString, stdext::stod); }

	// Thank you: http://stackoverflow.com/questions/236129/how-to-split-a-string-in-c
	static std::vector<String> &split(const String &s, char delim, std::vector<String> &elems) {
		StringStream ss(s);
		String item;
		while (std::getline(ss, item, delim)) {
			elems.push_back(item);
		}
		return elems;
	}

	// Thank you: http://stackoverflow.com/questions/236129/how-to-split-a-string-in-c
	static std::vector<String> split(const String &s, char delim) {
		std::vector<String> elems;
		split(s, delim, elems);
		return elems;
	}

	namespace Skills {
		static const String SkillStrings[Limits::Skills::MAX_ID] = {
			"One Hand Blunt",
			"One Hand Slashing",
			"Two Hand Blunt",
			"Two Hand Slashing",
			"Abjuration",
			"Alteration",
			"Apply Poison",
			"Archery",
			"Backstab",
			"Bind Wound",
			"Bash",
			"Block",
			"Brass Instruments",
			"Channeling",
			"Conjuration",
			"Defense",
			"Disarm",
			"Disarm Traps",
			"Divination",
			"Dodge",
			"Double Attack",
			"Dragon Punch",
			"Dual Wield",
			"Eagle Strike",
			"Evocation",
			"Feign Death",
			"Flying Kick",
			"Forage",
			"Hand To Hand",
			"Hide",
			"Kick",
			"Meditate",
			"Mend",
			"Offense",
			"Parry",
			"Pick Lock",
			"One Hand Piercing",
			"Riposte",
			"Round Kick",
			"Safe Fall",
			"Sense Heading",
			"Singing",
			"Sneak",
			"Specialize Abjuration",
			"Specialize Alteration",
			"Specialize Conjuration",
			"Specialize Divination",
			"Specialize Evocation",
			"Pick Pockets",
			"Stringed Instruments",
			"Swimming",
			"Throwing",
			"Tiger Claw",
			"Tracking",
			"Wind Instruments",
			"Fishing",
			"Make Poison",
			"Tinkering",
			"Research",
			"Alchemy",
			"Baking",
			"Tailoring",
			"Sense Traps",
			"Blacksmithing",
			"Fletching",
			"Brewing",
			"Alcohol Tolerance",
			"Begging",
			"Jewlry Making",
			"Pottery",
			"Percussion Instruments",
			"Intimidation",
			"Berserking",
			"Taunt",
			"Frenzy"
		};
		const String& fromID(const uint32 pSkillID);
	}
	namespace Languages {
		static const String LanguagesStrings[Limits::Languages::MAX_ID] = {
			"Common Tongue",
			"Barbarian",
			"Erudian",
			"Elvish",
			"Dark Elvish",
			"Dwarvish",
			"Troll",
			"Ogre",
			"Gnomish",
			"Halfling",
			"Thieves Cant",
			"Old Erudian",
			"Elder Elvish",
			"Froglok",
			"Goblin",
			"Gnoll",
			"Combine Tongue",
			"Elder Teir'Dal",
			"Lizardman",
			"Orcish",
			"Faerie",
			"Dragon",
			"Elder Dragon",
			"Dark Speech",
			"Vah Shir"
		};
		const String& fromID(const uint32 pLanguageID);
	}

	class PopupHelper {
	public:
		enum Colour : int32 {
			RED,
			GREEN,
			BLUE,
			AQUA,
			YELLOW,
			MAX
		};
		PopupHelper() {
			mColours.resize(Colour::MAX);
			mColours[RED] = "<c \"#FF0000\">";
			mColours[GREEN] = "<c \"#00FF00\">";
			mColours[BLUE] = "<c \"#0000FF\">";
			mColours[AQUA] = "<c \"#00FFFF\">";
			mColours[YELLOW] = "<c \"#FFFF00\">";
		}
		inline String getText() const { return mText; }
		inline void write(const String pText) { mText += pText; }
		inline void writeBr(const String pText) { write(pText); breakLine(); }
		
		//template <typename T>
		//inline void write(const string pString, const T pValue) { write(pString + std::to_string(pValue)); }
		
		inline void startColour(const Colour pColour) { write(mColours[pColour]); }
		inline void endColour() { mText += "</c>"; }
		inline void breakLine() { mText += "<br>"; }
		inline void clear() { mText.clear(); }
	private:
		String mText;
		std::vector<const String> mColours;
	};
}