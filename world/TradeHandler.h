#pragma once

#include "Types.h"
#include <list>

class ILogFactory;
class ILog;
class Character;
class Zone;
class Item;

class TradeHandler {
public:
	TradeHandler(Zone* pZone, ILogFactory* pLogFactory);
	~TradeHandler();

	void onRequest(Character* pCharacter, const u32 pSpawnID);

	void onResponse(Character* pCharacter, const u32 pSpawnID);

	void onBusy(Character* pCharacter, const u32 pSpawnID);

	void onAccept(Character* pCharacter, const u32 pSpawnID);

	void onCancel(Character* pCharacter, const u32 pSpawnID);

	void onMoveCurrency(Character* pCharacter, const u32 pType, const i32 pAmount);

private:

	void orderItems(std::list<Item*>& pUnordered, std::list<Item*>& pOrdered);
	const bool returnTradeItems(Character* pCharacter);
	const bool returnTradeCurrency(Character* pCharacter);
	const bool trade(Character* pCharacterA, Character* pCharacterB);

	ILog* mLog = nullptr;
	Zone* mZone = nullptr;
};