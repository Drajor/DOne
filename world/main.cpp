#include "../common/timer.h"
#include "../common/timeoutmgr.h"
#include "../common/platform.h"
#include "../common/crash.h"

#include "DataValidation.h"
#include "LogSystem.h"
#include "World.h"
#include "ZoneData.h"
#include "GuildManager.h"
#include "AccountManager.h"
#include "ZoneManager.h"
#include "TitleManager.h"
#include "XMLDataStore.h"
#include "SpellDataStore.h"
#include "StaticItemData.h"
#include "ItemDataStore.h"
#include "LogSystem.h"
#include "UCS.h"
#include "Random.h"
#include "NPCFactory.h"
#include "Timer.h"
#include "TimeUtility.h"
#include "Transmutation.h"
#include "AlternateCurrencyManager.h"
#include "ShopDataStore.h"
#include "Settings.h"
#include "HateControllerFactory.h"
#include "ServiceLocator.h"
#include "GroupManager.h"
#include "RaidManager.h"
#include "ItemFactory.h"
#include "CommandHandler.h"
#include "CharacterFactory.h"

#include "Testing.h"
#include "gtest/gtest.h"
#pragma comment(lib, "../../dependencies/gtest/gtestd.lib")

#include <memory>

/*
Google Test Notes
- Changed CRT to static
- Changed 'Debug Information Format' to 'Program Database'
- Addition Includes + gtest
*/

uint64 TTimer::mCurrentTime = 0;

TimeoutManager timeout_manager;

int main(int argc, char** argv)  {
	//system("pause");
	::testing::InitGoogleTest(&argc, argv);
	RUN_ALL_TESTS();

	EXPECTED_MAIN(Log::start("logs/" + std::to_string(Time::nowMilliseconds()) + ".log"));
	//system("pause");

	RegisterExecutablePlatform(ExePlatformWorld);
	set_exception_handler();

	Random::initialise();

	ServiceLocator::reset();

	ILogFactory* logFactory = new DefaultLogFactory();

	IDataStore* dataStore = new XMLDataStore();
	EXPECTED_MAIN(dataStore->loadSettings());
	EXPECTED_MAIN(dataStore->initialise());

	ShopDataStore* shopDataStore = new ShopDataStore();
	ServiceLocator::setShopDataStore(shopDataStore);
	EXPECTED_MAIN(shopDataStore->initialise(dataStore, logFactory));

	AccountManager* accountManager = new AccountManager();
	ServiceLocator::setAccountManager(accountManager);
	EXPECTED_MAIN(accountManager->initialise(dataStore, logFactory));

	ZoneDataStore* zoneDataManager = new ZoneDataStore();
	ServiceLocator::setZoneDataManager(zoneDataManager);
	EXPECTED_MAIN(zoneDataManager->initialise(dataStore, logFactory));

	TitleManager* titleManager = new TitleManager();
	ServiceLocator::setTitleManager(titleManager);
	EXPECTED_MAIN(titleManager->initialise(dataStore, logFactory));

	SpellDataStore* spellDataStore = new SpellDataStore();
	ServiceLocator::setSpellDataStore(spellDataStore);
	EXPECTED_MAIN(spellDataStore->initialise(dataStore, logFactory));

	AlternateCurrencyManager* alternateCurrencyManager = new AlternateCurrencyManager();
	ServiceLocator::setAlternateCurrencyManager(alternateCurrencyManager);
	EXPECTED_MAIN(alternateCurrencyManager->initialise(dataStore, logFactory));

	ItemDataStore* itemDataStore = new ItemDataStore();
	ServiceLocator::setItemDataStore(itemDataStore);
	EXPECTED_MAIN(itemDataStore->initialise(dataStore, logFactory));

	ItemFactory* itemFactory = new ItemFactory();
	ServiceLocator::setItemFactory(itemFactory);
	EXPECTED_MAIN(itemFactory->initialise(itemDataStore, logFactory));

	ItemGenerator* itemGenerator = new ItemGenerator();
	ServiceLocator::setItemGenerator(itemGenerator);
	EXPECTED_MAIN(itemGenerator->initialise(itemFactory, dataStore, logFactory));

	ZoneManager* zoneManager = new ZoneManager();
	ServiceLocator::setZoneManager(zoneManager);

	GroupManager* groupManager = new GroupManager();
	ServiceLocator::setGroupManager(groupManager);
	
	RaidManager* raidManager = new RaidManager();
	ServiceLocator::setRaidManager(raidManager);

	GuildManager* guildManager = new GuildManager();
	ServiceLocator::setGuildManager(guildManager);

	CommandHandler* commandHandler = new CommandHandler();
	EXPECTED_MAIN(commandHandler->initialise(dataStore));

	NPCFactory* npcFactory = new NPCFactory();
	ServiceLocator::setNPCFactory(npcFactory);
	EXPECTED_MAIN(npcFactory->initialise(dataStore, logFactory, itemFactory, shopDataStore));

	World* world = new World();
	ServiceLocator::setWorld(world);

	CharacterFactory* characterFactory = new CharacterFactory();
	EXPECTED_MAIN(characterFactory->initialise(dataStore, logFactory, itemFactory));

	EXPECTED_MAIN(zoneManager->initialise(world, zoneDataManager, groupManager, raidManager, guildManager, titleManager, commandHandler, itemFactory, logFactory, npcFactory));
	EXPECTED_MAIN(groupManager->initialise(logFactory, zoneManager));
	EXPECTED_MAIN(raidManager->initialise(zoneManager));
	EXPECTED_MAIN(guildManager->initialise(dataStore, logFactory));


	EXPECTED_MAIN(world->initialise(dataStore, logFactory, characterFactory, guildManager, zoneManager, accountManager));

	UCS* ucs = new UCS();
	ServiceLocator::setUCS(ucs);
	EXPECTED_MAIN(ucs->initialise());


	// Validate Data
	if (Settings::getValidationEnabled()) {
		if (!validateData()) {
			Log::error("Data Validation has failed!");
			system("pause");
			return 0;
		}
	}

	while(true) {
		Timer::SetCurrentTime();
		TTimer::setCurrentTime(Time::nowMilliseconds());
		world->update();

		//check for timeouts in other threads
		timeout_manager.CheckTimeouts();
		Sleep(20);
	}
	Log::status("World : Shutting down");

	Log::end();
	return 0;
}