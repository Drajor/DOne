#pragma once

#include "Constants.h"
#include <string>
#include <sstream>
#include "LogSystem.h"

#define ARG_STR(pARG) #pARG
#define ARG_PTR_CHECK(pARG) if(pARG == nullptr) { std::stringstream ss; ss << "[ARG_PTR_CHECK] ("<< ARG_STR(pARG) << ") Failed in" << __FUNCTION__; Log::error(ss.str()); return; }
#define ARG_PTR_CHECK_BOOL(pARG) if(pARG == nullptr) { std::stringstream ss; ss << "[ARG_PTR_CHECK] ("<< ARG_STR(pARG) << ") Failed in" << __FUNCTION__; Log::error(ss.str()); return false; }
#define ERROR_CONDITION(pCondition) if(!(pCondition))  { std::stringstream ss; ss << "[ERROR_CONDITION] ("<< ARG_STR(pCondition) << ") Found in" << __FUNCTION__; Log::error(ss.str()); return; }
#define ERROR_CONDITION_BOOL(pCondition) if(!(pCondition))  { std::stringstream ss; ss << "[ERROR_CONDITION] ("<< ARG_STR(pCondition) << ") Found in" << __FUNCTION__; Log::error(ss.str()); return false; }
#define PACKET_SIZE_CHECK(pCondition) if(!(pCondition))  { std::stringstream ss; ss << "[PACKET_SIZE_CHECK] ("<< ARG_STR(pCondition) << ") Failed in" << __FUNCTION__; Log::error(ss.str()); return; }
#define PACKET_SIZE_CHECK_BOOL(pCondition) if(!(pCondition))  { std::stringstream ss; ss << "[PACKET_SIZE_CHECK] ("<< ARG_STR(pCondition) << ") Failed in" << __FUNCTION__; Log::error(ss.str()); return false; }

class Character;
class Zone;
namespace Utility {
	void print(std::string pMessage);
	void criticalError(std::string pMessage);

	std::string characterLogDetails(Character* pCharacter);
	std::string zoneLogDetails(Zone* pZone);

	static std::string safeString(char* pCString, unsigned int pMaxSize) {
		pCString[pMaxSize - 1] = '\0'; // Ensure there is a null terminator at the very end
		return std::string(pCString);
	}

	static std::string StringIDString(StringID pStringID) {
		return std::to_string(static_cast<std::uint32_t>(pStringID));
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

		void writeString(std::string pData) {
			if (mWritten + pData.length() + 1 <= mSize) {
				strcpy(mPointer, pData.c_str());
				mPointer += pData.length() + 1;
				mWritten += pData.length() + 1;
			}
			else {
				Log::error("[Dynamic Structure] Attempt to write to invalid memory.");
			}
		}

		void movePointer(int pBytes) {
			// TODO: Check this!
			mPointer += pBytes;
			mWritten += pBytes; // I need to think about this more.
		}
		std::size_t getSize() { return mSize; }
		std::size_t getBytesWritten() { return mWritten; }

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

	static bool stofSafe(float& pValue, std::string& pString) {
		try {
			pValue = std::stof(pString);
			return true;
		}
		catch (...) {
			return false;
		}
		return false;
	}

	static bool stoulSafe(uint32& pValue, std::string& pString) {
		try {
			pValue = std::stoul(pString);
			return true;
		}
		catch (...) {
			return false;
		}
		return false;
	}

	// String To Unsigned 16
	static bool stou16Safe(std::uint16_t& pValue, std::string pString) {
		try {
			pValue = static_cast<std::uint16_t>(std::stoul(pString));
			return true;
		}
		catch (...) {
			return false;
		}
		return false;
	}
}