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
#include "ZoneData.h"
#include "GuildManager.h"
#include "AccountManager.h"
#include "ZoneManager.h"
#include "DataStore.h"
#include "LogSystem.h"

//#include "Payload.h"
#include "../common/eq_packet_structs.h"
#include <iostream>
//#include "Actor.h"


TimeoutManager timeout_manager; // Can't remove this for now...

int main(int argc, char** argv)  {
	//Payload::SpawnData sp;
	//Actor a;
	//std::cout << sizeof(Spawn_Struct) << std::endl;
	//std::cout << sizeof(Payload::SpawnData) << std::endl;
	//system("pause");
	//CharacterData* characterData = new CharacterData();
	//DataStore::getInstance().loadCharacter("Playerzero", characterData);

	//std::cout << sizeof(PlayerPositionUpdateServer_Struct) << std::endl;
	//system("pause");
	
	RegisterExecutablePlatform(ExePlatformWorld);
	set_exception_handler();

	// Initialise Account Manager.
	if (!AccountManager::getInstance().initialise()){
		Log::error("[Account Manager] Failed to initialise");
		return 1;
	}
	// Initialise Zone Data.
	if (!ZoneData::getInstance().initialise()){
		Log::error("[Zone Data] Failed to initialise");
		return 1;
	}
	// Initialise Guild Manager.
	if (!GuildManager::getInstance().initialise()) {
		Log::error("[Guild Manager] Failed to initialise");
		return 1;
	}
	// Initialise Zone Manager.
	if (!ZoneManager::getInstance().initialise()) {
		Log::error("[Zone Manager] Failed to initialise");
		return 1;
	}
	// Initialise Data Store.
	if (!DataStore::getInstance().initialise()) {
		Log::error("[Data Store] Failed to initialise");
		return 1;
	}
	// Initialise World.
	if (!World::getInstance().initialise()) {
		Log::error("[World] Failed to initialise");
		return 1;
	}

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