#pragma once

#include "Types.h"

namespace Log {
	bool start(const String& pFileName);
	void end();

	void status(String pMessage);
	void info(String pMessage);
	void error(String pMessage);
	void commonlog(String pMessage);
}

class ILog {
public:
	virtual ~ILog() {};
	virtual void status(const String& pMessage) = 0;
	virtual void info(const String& pMessage) = 0;
	virtual void error(const String& pMessage) = 0;
	virtual void setContext(String pContext) { mContext = pContext; }
protected:
	String mContext = "[Unknown]";
};

class LogContext : public ILog {
public:
	LogContext() {}
	LogContext(String pContext) { setContext(pContext); }
	void status(const String& pMessage) { Log::status(mContext + " " + pMessage); }
	void info(const String& pMessage) { Log::info(mContext + " " + pMessage); }
	void error(const String& pMessage) { Log::error(mContext + " " + pMessage); }
private:
};

class ILogFactory {
public:
	virtual ILog* make() = 0;
private:
};

class DefaultLogFactory : public ILogFactory {
public:
	ILog* make() { return new LogContext(); }
private:
};