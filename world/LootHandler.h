#pragma once

#include "Types.h"

class ILogFactory;
class ILog;
class Character;
class Zone;

class LootHandler {
public:

	LootHandler(Zone* pZone, ILogFactory* pLogFactory);
	~LootHandler();

	// Character is requesting to loot.
	void onRequest(Character* pCharacter, const u32 pSpawnID);

	// Character has finished looting.
	void onFinished(Character* pCharacter);

	// Character loots an Item.
	void onLootItem(Character* pCharacter, const u32 pSlotID);

private:

	ILog* mLog = nullptr;
	Zone* mZone = nullptr;
};