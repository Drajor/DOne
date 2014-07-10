/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/
#include "../common/debug.h"

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <signal.h>

#include "../common/debug.h"
#include "../common/queue.h"
#include "../common/timer.h"
#include "../common/EQStreamFactory.h"
#include "../common/EQPacket.h"
#include "worlddb.h"
#include "../common/seperator.h"
#include "../common/version.h"
#include "../common/eqtime.h"
#include "../common/timeoutmgr.h"
#include "../common/opcodemgr.h"
#include "../common/guilds.h"
#include "../common/EQStreamIdent.h"
#include "../common/rulesys.h"
#include "../common/platform.h"
#include "../common/crash.h"
#ifdef _WINDOWS
	#include <process.h>
	#define snprintf	_snprintf
	#define strncasecmp	_strnicmp
	#define strcasecmp	_stricmp
	#include <conio.h>
#else
	#include <pthread.h>
	#include "../common/unix.h"
	#include <sys/types.h>
	#include <sys/ipc.h>
	#include <sys/sem.h>
	#include <sys/shm.h>
	#if not defined (FREEBSD) && not defined (DARWIN)
		union semun {
			int val;
			struct semid_ds *buf;
			ushort *array;
			struct seminfo *__buf;
			void *__pad;
		};
	#endif

#endif

#include "../common/servertalk.h"
#include "../common/dbasync.h"
#include "WorldConfig.h"
#include "World.h"
#include "DataStore.h"
#include "MySQLDataProvider.h"
#include "Utility.h"

TimeoutManager timeout_manager; // Can't remove this for now...

DBAsync *dbasync = nullptr;
volatile bool RunLoops = true;
uint32 numplayers = 0;
uint32 numzones = 0;
bool holdzones = false;

void CatchSignal(int sig_num);

int main(int argc, char** argv) {
	MySQLDataProvider* dataProvider = new MySQLDataProvider();
	if (!dataProvider->initialise()) {
		Utility::criticalError("Data Provider failed to initialise");
		return 1;
	}
	DataStore* dataStore = new DataStore();
	dataStore->setProvider(dataProvider);

	World* world = new World(dataStore);
	if (!world->initialise()) {
		Utility::criticalError("World failed to initialise");
		return 1;
	}

	RegisterExecutablePlatform(ExePlatformWorld);
	set_exception_handler();

	// Load server configuration
	_log(WORLD__INIT, "Loading server configuration..");
	if (!WorldConfig::LoadConfig()) {
		_log(WORLD__INIT_ERR, "Loading server configuration failed.");
		return 1;
	}
	const WorldConfig *Config=WorldConfig::get();

	if(!load_log_settings(Config->LogSettingsFile.c_str()))
		_log(WORLD__INIT, "Warning: Unable to read %s", Config->LogSettingsFile.c_str());
	else
		_log(WORLD__INIT, "Log settings loaded from %s", Config->LogSettingsFile.c_str());


	_log(WORLD__INIT, "CURRENT_VERSION: %s", CURRENT_VERSION);

	#ifdef _DEBUG
		_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	#endif

	if (signal(SIGINT, CatchSignal) == SIG_ERR)	{
		_log(WORLD__INIT_ERR, "Could not set signal handler");
		return 1;
	}
	if (signal(SIGTERM, CatchSignal) == SIG_ERR)	{
		_log(WORLD__INIT_ERR, "Could not set signal handler");
		return 1;
	}
	#ifndef WIN32
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)	{
		_log(WORLD__INIT_ERR, "Could not set signal handler");
		return 1;
	}
	#endif

	_log(WORLD__INIT, "Connecting to MySQL...");
	if (!database.Connect(
		Config->DatabaseHost.c_str(),
		Config->DatabaseUsername.c_str(),
		Config->DatabasePassword.c_str(),
		Config->DatabaseDB.c_str(),
		Config->DatabasePort)) {
		_log(WORLD__INIT_ERR, "Cannot continue without a database connection.");
		return 1;
	}
	dbasync = new DBAsync(&database);

	_log(WORLD__INIT, "Loading variables..");
	database.LoadVariables();
	_log(WORLD__INIT, "Loading zones..");
	database.LoadZoneNames();
	_log(WORLD__INIT, "Clearing groups..");
	database.ClearGroup();
	_log(WORLD__INIT, "Clearing raids..");
	database.ClearRaid();
	database.ClearRaidDetails();
	_log(WORLD__INIT, "Loading items..");
	if (!database.LoadItems()) {
		_log(WORLD__INIT_ERR, "Error: Could not load item data. But ignoring");
	}
	_log(WORLD__INIT, "Loading guilds..");
	//guild_mgr.loadGuilds();
	//rules:
	{
		char tmp[64];
		if (database.GetVariable("RuleSet", tmp, sizeof(tmp)-1)) {
			_log(WORLD__INIT, "Loading rule set '%s'", tmp);
			if(!RuleManager::Instance()->LoadRules(&database, tmp)) {
				_log(WORLD__INIT_ERR, "Failed to load ruleset '%s', falling back to defaults.", tmp);
			}
		} else {
			if(!RuleManager::Instance()->LoadRules(&database, "default")) {
				_log(WORLD__INIT, "No rule set configured, using default rules");
			} else {
				_log(WORLD__INIT, "Loaded default rule set 'default'", tmp);
			}
		}
	}
	if(RuleB(World, ClearTempMerchantlist)){
		_log(WORLD__INIT, "Clearing temporary merchant lists..");
		database.ClearMerchantTemp();
	}


	char tmp[20];
	tmp[0] = '\0';
	database.GetVariable("holdzones",tmp, 20);
	if ((strcasecmp(tmp, "1") == 0)) {
		holdzones = true;
	}
	_log(WORLD__INIT, "Reboot zone modes %s",holdzones ? "ON" : "OFF");

	_log(WORLD__INIT, "Deleted %i stale player corpses from database", database.DeleteStalePlayerCorpses());
	if (RuleB(World, DeleteStaleCorpeBackups) == true) {
	_log(WORLD__INIT, "Deleted %i stale player backups from database", database.DeleteStalePlayerBackups());
	}

	_log(WORLD__INIT, "Purging expired instances");
	database.PurgeExpiredInstances();
	Timer PurgeInstanceTimer(450000);
	PurgeInstanceTimer.Start(450000);

	_log(WORLD__INIT, "Loading char create info...");
	database.LoadCharacterCreateAllocations();
	database.LoadCharacterCreateCombos();

	Timer InterserverTimer(INTERSERVER_TIMER); // does MySQL pings and auto-reconnect
	InterserverTimer.Trigger();
	uint8 ReconnectCounter = 100;

	while(RunLoops) {
		Timer::SetCurrentTime();
		world->update();

		if(PurgeInstanceTimer.Check())
		{
			database.PurgeExpiredInstances();
		}

		

		//check for timeouts in other threads
		timeout_manager.CheckTimeouts();

		if (InterserverTimer.Check()) {
			InterserverTimer.Start();
			database.ping();
			// AsyncLoadVariables(dbasync, &database);
			ReconnectCounter++;
//			if (ReconnectCounter >= 12) { // only create thread to reconnect every 10 minutes. previously we were creating a new thread every 10 seconds
//				ReconnectCounter = 0;
//				if (loginserverlist.AllConnected() == false) {
//#ifdef _WINDOWS
//					_beginthread(AutoInitLoginServer, 0, nullptr);
//#else
//					pthread_t thread;
//					pthread_create(&thread, nullptr, &AutoInitLoginServer, nullptr);
//#endif
//				}
//			}
		}
		Sleep(20);
	}
	_log(WORLD__SHUTDOWN,"World main loop completed.");

	delete world;
	delete dataStore;
	delete dataProvider;

	return 0;
}

void CatchSignal(int sig_num) {
	RunLoops = false;
}

void UpdateWindowTitle(char* iNewTitle) {
#ifdef _WINDOWS
	char tmp[500];
	if (iNewTitle) {
		snprintf(tmp, sizeof(tmp), "World: %s", iNewTitle);
	}
	else {
		snprintf(tmp, sizeof(tmp), "World");
	}
	SetConsoleTitle(tmp);
#endif
}

