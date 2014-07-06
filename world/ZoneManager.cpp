#include "ZoneManager.h"
#include "Zone.h"
#include "client.h"

void ZoneManager::update() {
	for (auto& i : mZones) {
		i->update();
	}
}

void ZoneManager::clientConnect(Client* pClient)
{
}
