#include "../common/timer.h"
#include "../common/timeoutmgr.h"
#include "../common/platform.h"
#include "../common/crash.h"

#include "LogSystem.h"
#include "World.h"
#include "ZoneData.h"
#include "GuildManager.h"
#include "AccountManager.h"
#include "ZoneManager.h"
#include "TitleManager.h"
#include "DataStore.h"
#include "SpellDataStore.h"
#include "StaticItemData.h"
#include "ItemDataStore.h"
#include "LogSystem.h"
#include "UCS.h"
#include "Random.h"
#include "NPCFactory.h"
#include "Timer.h"
#include "TimeUtility.h"

uint64 TTimer::mCurrentTime = 0;

TimeoutManager timeout_manager;

int main(int argc, char** argv)  {
	EXPECTED_MAIN(Log::start("logs/" + std::to_string(Time::nowMilliseconds()) + ".log"));
	//system("pause");

	RegisterExecutablePlatform(ExePlatformWorld);
	set_exception_handler();

	Random::initialise();

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
	EXPECTED_MAIN(StaticItemData::getInstance().initialise());
	EXPECTED_MAIN(ItemDataStore::getInstance().initialise());
	EXPECTED_MAIN(NPCFactory::getInstance().initialise());

	while(true) {
		Timer::SetCurrentTime();
		TTimer::setCurrentTime(Time::nowMilliseconds());
		World::getInstance().update();

		//check for timeouts in other threads
		timeout_manager.CheckTimeouts();
		Sleep(20);
	}
	Log::status("World : Shutting down");

	Log::end();
	return 0;
}