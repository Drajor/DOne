#pragma once

#include "Constants.h"
#include "LogSystem.h"
#include <algorithm>
#include <cctype>

#define ARG_STR(pARG) #pARG
#define EXPECTED(pCondition) if(!(pCondition))  { StringStream ss; ss << "[EXPECTED] ("<< ARG_STR(pCondition) << ") Failed in " << __FUNCTION__; Log::error(ss.str()); return; }
#define EXPECTED_VAR(pCondition, pReturn) if(!(pCondition))  { StringStream ss; ss << "[EXPECTED] ("<< ARG_STR(pCondition) << ") Failed in " << __FUNCTION__; Log::error(ss.str()); return pReturn; }
#define EXPECTED_BOOL(pCondition) { EXPECTED_VAR(pCondition, false) }
#define EXPECTED_PTR(pCondition) { EXPECTED_VAR(pCondition, nullptr) }
#define EXPECTED_MAIN(pCondition) { EXPECTED_VAR(pCondition, 1) }
//#define EXPECTED_PTR(pCondition) if(!(pCondition))  { StringStream ss; ss << "[EXPECTED] ("<< ARG_STR(pCondition) << ") Failed in " << __FUNCTION__; Log::error(ss.str()); return nullptr; }
//#define EXPECTED_MAIN(pCondition) if(!(pCondition))  { StringStream ss; ss << "[EXPECTED] ("<< ARG_STR(pCondition) << ") Failed in " << __FUNCTION__; Log::error(ss.str()); return 1; }

#define EXPECTEDX(pCondition, pLog) if(!(pCondition))  { StringStream ss; ss << "[EXPECTED] ("<< ARG_STR(pCondition) << ") Failed in " << __FUNCTION__; pLog->error(ss.str()); return; }
#define EXPECTED_VARX(pCondition, pReturn, pLog) if(!(pCondition))  { StringStream ss; ss << "[EXPECTED] ("<< ARG_STR(pCondition) << ") Failed in " << __FUNCTION__; pLog->error(ss.str()); return pReturn; }
#define EXPECTED_BOOLX(pCondition, pLog) { EXPECTED_VARX(pCondition, false, pLog) }
#define EXPECTED_PTRX(pCondition, pLog) { EXPECTED_VARX(pCondition, nullptr, pLog) }

#define PRINT_MEMBER(pMember) ss << ARG_STR(pMember) << " = " << pMember << " | ";

class Character;
class Zone;
namespace Utility {
	namespace Time {
		i64 now();
	}
	void print(String pMessage);
	String getRandomName();

	template<typename T>
	T clamp(const T& pValue, const T& pMinimum, const T& pMaximum) {
		return std::max(pMinimum, std::min(pMaximum, pValue));
	}

	template<typename T>
	const bool inRange(const T& pValue, const T& pMinimum, const T& pMaximum) {
		return pValue >= pMinimum && pValue <= pMaximum;
	}

	// Searches pStringA for pStringB case insensitively.
	static bool findCI(const String& pStringA, const String& pStringB) {
		String a = pStringA;
		std::transform(pStringA.begin(), pStringA.end(), a.begin(), ::toupper);
		String b = pStringB;
		std::transform(pStringB.begin(), pStringB.end(), b.begin(), ::toupper);

		return a.find(b) != String::npos;
	}

	// Compares pStringA and pStringB case insensitively.
	static bool compareCI(const String& pStringA, const String& pStringB) {
		String a = pStringA;
		std::transform(pStringA.begin(), pStringA.end(), a.begin(), ::toupper);
		String b = pStringB;
		std::transform(pStringB.begin(), pStringB.end(), b.begin(), ::toupper);

		return a == b;
	}

	// Converts from copper to platinum, gold, silver and copper.
	template <typename T>
	inline void convertFromCopper(const T pValue, int32& pPlatinum, int32& pGold, int32& pSilver, int32& pCopper) {
		pCopper = pValue % 10;
		pSilver = (pValue % 100) / 10;
		pGold = (pValue % 1000) / 100;
		pPlatinum = (pValue / 1000);
	}

	// Converts platinum, gold, silver and copper to copper.
	inline bool convertCurrency(int64& pResult, const int32 pPlatinum, const int32 pGold, const int32 pSilver, const int32 pCopper) {
		EXPECTED_BOOL(pResult == 0);
		EXPECTED_BOOL(pPlatinum >= 0);
		EXPECTED_BOOL(pGold >= 0);
		EXPECTED_BOOL(pSilver >= 0);
		EXPECTED_BOOL(pCopper >= 0);
		int64 temp = 0;

		// Copper.
		temp = (pResult += pCopper);
		
		// Silver.
		if (pSilver > 0) {
			temp = pResult + (pSilver * 10);
			EXPECTED_BOOL(temp >= pResult); // wrap.
			pResult = temp;
		}

		// Gold.
		if (pGold > 0) {
			temp = pResult + (pGold * 100);
			EXPECTED_BOOL(temp >= pResult); // wrap.
			pResult = temp;
		}

		// Platinum.
		if (pPlatinum > 0) {
			temp = pResult + (pPlatinum * 1000);
			EXPECTED_BOOL(temp >= pResult); // wrap.
			pResult = temp;
		}

		return true;
	}

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

	// Checks a String for any digits.
	static const bool containsDigits(const String& pString) {
		auto f = [](char c) { return !std::isdigit(c); };
		return !pString.empty() && std::find_if(pString.begin(), pString.end(), f) == pString.end();
	}

	static const bool isSafe(char* pCString, u32 pMaxSize) {
		if (!pCString) return false;
		// strnlen_s - http://msdn.microsoft.com/en-us/library/z50ty2zh.aspx
		auto strLength = strnlen_s(pCString, pMaxSize);
		return strLength != pMaxSize;
	}

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

	static String StringIDString(const u32 pStringID) {
		return std::to_string(static_cast<uint32>(pStringID));
	}

	class MemoryWriter {
	public:
		template <typename T>
		MemoryWriter(T pStart, int pSize) : mStart(reinterpret_cast<char*>(pStart)), mPointer(mStart), mSize(pSize), mWritten(0) {}

		inline const bool _memset(const int pWith, const std::size_t pSize) {
			if (pSize == 0) return false; // Sanity.
			if (mWritten == mSize) return false; // No more memory to write to.
			if (mWritten + pSize > mSize) return false; // Not enough memory to write to.

			memset(mPointer, pWith, pSize);
			mPointer += pSize;
			mWritten += pSize;

			return true;
		}

		template <typename T>
		inline void write(T pData) {
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

		inline void writeChunk(void* pFrom, const std::size_t pSize) {
			if (mWritten + pSize <= mSize) {
				memcpy(mPointer, pFrom, pSize);
				mPointer += pSize;
				mWritten += pSize;
			}
			else {
				Log::error("[Dynamic Structure] Attempt to write to invalid memory.");
			}
		}

		inline void writeString(const String& pData) {
			if (mWritten + pData.length() + 1 <= mSize) {
				strcpy_s(mPointer, pData.length() + 1, pData.c_str()); // +1 for null terminator.
				mPointer += pData.length() + 1;
				mWritten += pData.length() + 1;
			}
			else {
				Log::error("[Dynamic Structure] Attempt to write to invalid memory.");
			}
		}

		inline void writeFixedString(const String& pData, const std::size_t pSize) {
			if (mWritten + pSize <= mSize) {
				strcpy_s(mPointer, pData.length() + 1, pData.c_str()); // +1 for null terminator.
				mPointer += pData.length() + 1;
				mWritten += pData.length() + 1;
				const std::size_t difference = pSize - (pData.length() + 1);
				memset(mPointer, 0, difference); // Fill remaining memory with zeros
				mPointer += difference;
				mWritten += difference;

			}
		}

		inline void movePointer(int pBytes) {
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

	class MemoryReader {
	public:
		template <typename T>
		MemoryReader(T pStart, int pSize) : mStart(reinterpret_cast<char*>(pStart)), mPointer(mStart), mSize(pSize), mRead(0) {}

		const bool readString(std::string& pString, const unsigned int pMaxSize) {
			if (mRead == mSize) return false; // No more memory left.
			auto maxSize = pMaxSize;
			if (mRead + pMaxSize > mSize) maxSize = mSize - mRead; // Adjust max string length if the amount of memory remaining is less than the requested max string length.
			// Check whether it is safe to read a string from this memory.
			if (isSafe(mPointer, maxSize)) {
				pString = std::string(mPointer);
				mPointer += pString.length() + 1;
				mRead += pString.length() + 1;
				return true;
			}

			return false;
		}

		template <typename T>
		const bool read(T& pData) {
			if (mRead == mSize) return false; // No more memory left.
			if (mRead + sizeof(T) > mSize) return false; // Not enough memory left to read this sized type.

			pData = *reinterpret_cast<T*>(mPointer); // Copy data.
			mPointer += sizeof(T);
			mRead += sizeof(T);
			return true;
		}

		inline const std::size_t getSize() const { return mSize; }
		inline const std::size_t getRead() const { return mRead; }
		inline const std::size_t getRemaining() const { return getSize() - getRead(); }
		inline const bool check() const { return getRemaining() == 0; }
	private:
		std::size_t mSize = 0; // maximum number of bytes to be read.
		std::size_t mRead = 0; // number of bytes read.
		char* mStart = nullptr; // start memory location.
		char* mPointer = nullptr; // current location in memory.
	};

	// list.remove_if(Utility::containerEntryDelete)
	template <typename T>
	bool containerEntryDelete(T pValue) {
		delete pValue;
		return true;
	}

	//static bool stoSafe(float& pValue, String& pString) {
	//	try {
	//		pValue = std::stof(pString);
	//		return true;
	//	}
	//	catch (...) {
	//		return false;
	//	}
	//	return false;
	//}

	//static bool stoSafe(uint32& pValue, String& pString) {
	//	try {
	//		pValue = std::stoul(pString);
	//		return true;
	//	}
	//	catch (...) {
	//		return false;
	//	}
	//	return false;
	//}
	//static bool stou16Safe(uint16& pValue, String pString) {
	//	try {
	//		pValue = static_cast<uint16>(std::stoul(pString));
	//		return true;
	//	}
	//	catch (...) {
	//		return false;
	//	}
	//	return false;
	//}
	//static bool stou8Safe(uint8& pValue, String& pString) {
	//	try {
	//		pValue = static_cast<uint8>(std::stoul(pString));
	//		return true;
	//	}
	//	catch (...) {
	//		return false;
	//	}
	//	return false;
	//}
	//static bool stoi8Safe(int8& pValue, String& pString) {
	//	try {
	//		pValue = static_cast<int8>(std::stoi(pString));
	//		return true;
	//	}
	//	catch (...) {
	//		return false;
	//	}
	//	return false;
	//}
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

	inline bool stoSafe(i64& pValue, const String& pString) { return stdext::stoExecute(pValue, pString, stdext::stoi); }
	inline bool stoSafe(i32& pValue, const String& pString) { return stdext::stoExecute(pValue, pString, stdext::stoi); }
	inline bool stoSafe(i16& pValue, const String& pString) { return stdext::stoExecute(pValue, pString, stdext::stoi); }
	inline bool stoSafe(i8& pValue, const String& pString) { return stdext::stoExecute(pValue, pString, stdext::stoi); }

	inline bool stoSafe(u64& pValue, const String& pString) { return stdext::stoExecute(pValue, pString, stdext::stou); }
	inline bool stoSafe(u32& pValue, const String& pString) { return stdext::stoExecute(pValue, pString, stdext::stou); }
	inline bool stoSafe(u16& pValue, const String& pString) { return stdext::stoExecute(pValue, pString, stdext::stou); }
	inline bool stoSafe(u8& pValue, const String& pString) { return stdext::stoExecute(pValue, pString, stdext::stou); }

	inline bool stoSafe(float& pValue, const String& pString) { return stdext::stoExecute(pValue, pString, stdext::stof); }
	inline bool stoSafe(double& pValue, const String& pString) { return stdext::stoExecute(pValue, pString, stdext::stod); }
	inline bool stoSafe(bool& pValue, const String& pString) { pValue = pString != "0"; return true; }

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

		inline void tableBegin() { mText += "<table>"; }
		inline void tableEnd() { mText += "</table>"; }
		inline void rowBegin() { mText += "<tr>"; }
		inline void rowEnd() { mText += "</tr>"; }
		inline void writeColumn(const String& pText) { mText += "<td>" + pText + "</td>"; }
		
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