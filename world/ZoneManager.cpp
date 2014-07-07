#include "ZoneManager.h"
#include "Zone.h"
#include "client.h"
#include "DataStore.h"

ZoneManager::ZoneManager(DataStore* pDataStore) :mDataStore(pDataStore) { }

void ZoneManager::update() {
	for (auto& i : mZones) {
		i->update();
	}
}

void ZoneManager::clientConnect(Client* pClient)
{
}
