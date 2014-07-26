#pragma once

#include <string>
#include <sstream>
#include "LogSystem.h"

namespace Utility {
	void print(std::string pMessage);
	void criticalError(std::string pMessage);

	static std::string safeString(char* pCString, unsigned int pMaxSize) {
		pCString[pMaxSize - 1] = '\0'; // Ensure there is a null terminator at the very end
		return std::string(pCString);
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
		int getSize() { return mSize; }
		int getBytesWritten() { return mWritten; }

	private:
		int mSize; // maximum number of bytes to be written.
		int mWritten; // number of bytes written.
		char* mStart; // start memory location.
		char* mPointer; // current location in memory.
	};
}