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

enum ConsoleColours : WORD {
	CC_BLUE = 1,
	CC_GREEN = 2,
	CC_RED = 4
};

void Log::status(String pMessage) {
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(consoleHandle, CC_BLUE);
	Log::commonlog("[STATUS] " + pMessage);
}

void Log::info(String pMessage) {
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(consoleHandle, CC_GREEN);
	Log::commonlog("[INFO] " + pMessage);
}

void Log::error(String pMessage) {
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(consoleHandle, &csbi);

	SetConsoleTextAttribute(consoleHandle, CC_RED);
	Log::commonlog("[ERROR] " + pMessage);

	SetConsoleTextAttribute(consoleHandle, csbi.wAttributes);

}

void Log::commonlog(String pMessage) {
#ifdef PRINT_LOG
	Utility::print(pMessage);
#endif
	logFile.write(pMessage);
}
