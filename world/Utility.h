#pragma once

#include "Constants.h"
#include <string>
#include <sstream>
#include "LogSystem.h"

#define ARG_STR(pARG) #pARG
#define ARG_PTR_CHECK(pARG) if(pARG == nullptr) { StringStream ss; ss << "[ARG_PTR_CHECK] ("<< ARG_STR(pARG) << ") Failed in" << __FUNCTION__; Log::error(ss.str()); return; }
#define ARG_PTR_CHECK_BOOL(pARG) if(pARG == nullptr) { StringStream ss; ss << "[ARG_PTR_CHECK] ("<< ARG_STR(pARG) << ") Failed in" << __FUNCTION__; Log::error(ss.str()); return false; }
#define EXPECTED(pCondition) if(!(pCondition))  { StringStream ss; ss << "[EXPECTED] ("<< ARG_STR(pCondition) << ") Failed in" << __FUNCTION__; Log::error(ss.str()); return; }
#define EXPECTED_BOOL(pCondition) if(!(pCondition))  { StringStream ss; ss << "[EXPECTED] ("<< ARG_STR(pCondition) << ") Failed in" << __FUNCTION__; Log::error(ss.str()); return false; }
#define EXPECTED_PTR(pCondition) if(!(pCondition))  { StringStream ss; ss << "[EXPECTED] ("<< ARG_STR(pCondition) << ") Failed in" << __FUNCTION__; Log::error(ss.str()); return nullptr; }
#define EXPECTED_MAIN(pCondition) if(!(pCondition))  { StringStream ss; ss << "[EXPECTED] ("<< ARG_STR(pCondition) << ") Failed in" << __FUNCTION__; Log::error(ss.str()); return 1; }
#define PACKET_SIZE_CHECK(pCondition) if(!(pCondition))  { StringStream ss; ss << "[PACKET_SIZE_CHECK] ("<< ARG_STR(pCondition) << ") Failed in" << __FUNCTION__; Log::error(ss.str()); return; }
#define PACKET_SIZE_CHECK_BOOL(pCondition) if(!(pCondition))  { StringStream ss; ss << "[PACKET_SIZE_CHECK] ("<< ARG_STR(pCondition) << ") Failed in" << __FUNCTION__; Log::error(ss.str()); return false; }

class Character;
class Zone;
namespace Utility {
	void print(String pMessage);
	void criticalError(String pMessage);

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
		std::size_t getSize() { return mSize; }
		std::size_t getBytesWritten() { return mWritten; }
		bool check() { return mWritten == mSize; }
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
}