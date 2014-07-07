#include "Utility.h"
#include <iostream>

void Utility::print(std::string pMessage)
{
	std::cout << pMessage << std::endl;
}

void Utility::criticalError(std::string pMessage)
{
	print("[Critical Error] " + pMessage);
}
