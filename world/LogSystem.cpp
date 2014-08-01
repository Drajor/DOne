#include "LogSystem.h"
#include <windows.h>
#include <stdio.h>

#define PRINT_LOG
#ifdef PRINT_LOG
#include "Utility.h"
#endif

enum ConsoleColours : WORD {
	CC_BLUE = 1,
	CC_GREEN = 2,
	CC_RED = 4
};

void Log::status(std::string pMessage) {
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(consoleHandle, CC_BLUE);
	Log::commonlog("[STATUS] " + pMessage);
}

void Log::info(std::string pMessage) {
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(consoleHandle, CC_GREEN);
	Log::commonlog("[INFO] " + pMessage);
}

void Log::error(std::string pMessage) {
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(consoleHandle, CC_RED);
	Log::commonlog("[ERROR] " + pMessage);
}

void Log::commonlog(std::string pMessage) {
#ifdef PRINT_LOG
	Utility::print(pMessage);
#endif
}