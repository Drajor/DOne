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
#include "SpellDataStore.h"
#include "LogSystem.h"
#include "UCS.h"

TimeoutManager timeout_manager;

int main(int argc, char** argv)  {
	//system("pause");
	
	RegisterExecutablePlatform(ExePlatformWorld);
	set_exception_handler();

	EXPECTED_MAIN(DataStore::loadSettings());
	EXPECTED_MAIN(AccountManager::getInstance().initialise());
	EXPECTED_MAIN(ZoneDataManager::getInstance().initialise());
	EXPECTED_MAIN(GuildManager::getInstance().initialise());
	EXPECTED_MAIN(ZoneManager::getInstance().initialise());
	EXPECTED_MAIN(DataStore::getInstance().initialise());
	EXPECTED_MAIN(World::getInstance().initialise());
	EXPECTED_MAIN(UCS::getInstance().initialise());
	EXPECTED_MAIN(TitleManager::getInstance().initialise());
	EXPECTED_MAIN(SpellDataStore::getInstance().initialise());

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