#include "LogSystem.h"
#include <stdio.h>

#define PRINT_LOG
#ifdef PRINT_LOG
#include "Utility.h"
#endif

void Log::status(std::string pMessage) {
	Log::commonlog("[STATUS] " + pMessage);
}

void Log::info(std::string pMessage) {
	Log::commonlog("[INFO] " + pMessage);
}

void Log::error(std::string pMessage) {
	Log::commonlog("[ERROR] " + pMessage);
}

void Log::commonlog(std::string pMessage) {
#ifdef PRINT_LOG
	Utility::print(pMessage);
#endif
}