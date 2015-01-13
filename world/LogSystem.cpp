#include "LogSystem.h"
#include <windows.h>
#include <stdio.h>
#include <fstream>

#define PRINT_LOG
#ifdef PRINT_LOG
#include "Utility.h"
#endif

class LogFile {
public:
	const bool open(const String& pFileName) {
		if (mInitialised) return false;

		try {
			mLogFile.open(pFileName, std::fstream::out);
		}
		catch (...) {
			return false;
		}
		if (!mLogFile.good()) return false;

		return true;
	}
	inline void close() { mLogFile.close(); }
	inline void write(const String& pMessage) {
		auto t = std::time(nullptr);
		auto tm = *std::localtime(&t);
		mLogFile << std::put_time(&tm, "(%d-%m-%Y %H-%M-%S) ") << pMessage << std::endl;
	}
private:
	std::fstream mLogFile;
	bool mInitialised = false;
};
static LogFile logFile;

bool Log::start(const String& pFileName) {
	if (!logFile.open(pFileName)) return false;
	
	logFile.write("Log Started.");	
	return true;
}
void Log::end(){
	logFile.write("Log Ended.");
	logFile.close();
}

namespace ConsoleColours {
	enum : WORD {
		Black = 0,
		Blue = 1,
		Green = 2,
		Cyan = 3,
		Red = 4,
		Magenta = 5,
		Brown = 6,
		LightGray = 7,
		DarkGray = 8,
		LightBlue = 9,
		LightGreen = 10,
		LightCyan = 11,
		LightRed = 12,
		LightMagenta = 13,
		Yellow = 14,
		White = 15,
	};
}

void Log::status(String pMessage) {
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(consoleHandle, ConsoleColours::Yellow);
	Log::commonlog("[STATUS] " + pMessage);
}

void Log::info(String pMessage) {
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(consoleHandle, ConsoleColours::White);
	Log::commonlog("[INFO] " + pMessage);
}

void Log::error(String pMessage) {
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(consoleHandle, &csbi);

	SetConsoleTextAttribute(consoleHandle, ConsoleColours::Red);
	Log::commonlog("[ERROR] " + pMessage);

	SetConsoleTextAttribute(consoleHandle, csbi.wAttributes);

}

void Log::commonlog(String pMessage) {
#ifdef PRINT_LOG
	Utility::print(pMessage);
#endif
	logFile.write(pMessage);
}
