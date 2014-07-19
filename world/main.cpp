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
#include "../common/timer.h"
#include "../common/version.h"
#include "../common/timeoutmgr.h"
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

#include "World.h"
#include "DataStore.h"
#include "MySQLDataProvider.h"
#include "LogSystem.h"

TimeoutManager timeout_manager; // Can't remove this for now...

int main(int argc, char** argv) {
	Log::status("World starting!");
	RegisterExecutablePlatform(ExePlatformWorld);
	set_exception_handler();

	Log::info("Data Provider : Initialising");
	MySQLDataProvider* dataProvider = new MySQLDataProvider();
	if (!dataProvider->initialise()) {
		Log::error("Data Provider : Failed to initialise");
		return 1;
	}
	Log::info("Data Provider : Initialised");

	DataStore* dataStore = new DataStore();
	dataStore->setProvider(dataProvider);

	Log::info("World : Initialising");
	World* world = new World(dataStore);
	if (!world->initialise()) {
		Log::error("World : Failed to initialise");
		return 1;
	}
	Log::info("World : Initialised");

	while(true) {
		Timer::SetCurrentTime();
		world->update();

		//check for timeouts in other threads
		timeout_manager.CheckTimeouts();
		Sleep(0);
	}
	Log::status("World : Shutting down");

	delete world;
	delete dataStore;
	delete dataProvider;

	return 0;
}