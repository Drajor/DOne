#pragma once

#include "Constants.h"

namespace Log {
	bool start(const String& pFileName);
	void end();

	void status(String pMessage);
	void info(String pMessage);
	void error(String pMessage);
	void commonlog(String pMessage);
}

class LogInterface {
public:
	virtual ~LogInterface() {};
	virtual void status(const String& pMessage) = 0;
	virtual void info(const String& pMessage) = 0;
	virtual void error(const String& pMessage) = 0;
private:
};

class LogContext : public LogInterface{
public:
	LogContext(String pContextName) {}
	void status(const String& pMessage) { Log::status(mContext + " " + pMessage); }
	void info(const String& pMessage) { Log::info(mContext + " " + pMessage); }
	void error(const String& pMessage) { Log::error(mContext + " " + pMessage); }
private:
	String mContext = "Unknown";
};