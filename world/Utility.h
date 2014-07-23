#pragma once

#include <string>

namespace Utility {
	void print(std::string pMessage);
	void criticalError(std::string pMessage);

	static std::string safeString(char* pCString, unsigned int pMaxSize) {
		pCString[pMaxSize - 1] = '\0'; // Ensure there is a null terminator at the very end
		return std::string(pCString);
	}

}