#pragma once

#include <string>
#include <sstream>

namespace Log {
	void status(std::string pMessage);
	void info(std::string pMessage);
	void error(std::string pMessage);
	void commonlog(std::string pMessage);
}