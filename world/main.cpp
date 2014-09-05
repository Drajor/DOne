#include "../common/timer.h"

#include "../common/timeoutmgr.h"
#include "../common/platform.h"
#include "../common/crash.h"

#include "World.h"
#include "ZoneData.h"
#include "GuildManager.h"
#include "AccountManager.h"
#include "ZoneManager.h"
#include "TitleManager.h"
#include "DataStore.h"
#include "LogSystem.h"
#include "UCS.h"

TimeoutManager timeout_manager;

//#include "Payload.h"
//#include "../common/eq_packet_structs.h"
//#include <iostream>

int main(int argc, char** argv)  {
	//std::cout << Payload::Zone::Surname::size() << std::endl;
	//std::cout << sizeof(Payload::Zone::Surname) << std::endl;
	//std::cout << sizeof(Surname_Struct) << std::endl;
	//system("pause");
	
	RegisterExecutablePlatform(ExePlatformWorld);
	set_exception_handler();

	EXPECTED_MAIN(DataStore::loadSettings());
	EXPECTED_MAIN(AccountManager::getInstance().initialise());
	EXPECTED_MAIN(ZoneData::getInstance().initialise());
	EXPECTED_MAIN(GuildManager::getInstance().initialise());
	EXPECTED_MAIN(ZoneManager::getInstance().initialise());
	EXPECTED_MAIN(DataStore::getInstance().initialise());
	EXPECTED_MAIN(World::getInstance().initialise());
	EXPECTED_MAIN(UCS::getInstance().initialise());
	EXPECTED_MAIN(TitleManager::getInstance().initialise());

	while(true) {
		Timer::SetCurrentTime();
		World::getInstance().update();

		//check for timeouts in other threads
		timeout_manager.CheckTimeouts();
		Sleep(5);
	}
	Log::status("World : Shutting down");

	return 0;
}