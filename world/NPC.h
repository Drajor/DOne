#pragma once

#include "Actor.h"

class SpawnPoint;
class Item;

class NPC : public Actor {
public:
	NPC();
	inline const bool isNPC() const { return true; }
	const bool isNPCCorpse() const { return getActorType() == AT_NPC_CORPSE; }
	const bool initialise();
	const bool onDeath();
	void onDestroy();
	const bool update();
	inline SpawnPoint* getSpawnPoint() const { return mSpawnPoint; }
	inline void setSpawnPoint(SpawnPoint* pSpawnPoint) { mSpawnPoint = pSpawnPoint; }

	// Standard Currency
	inline const int32 getCopper() const { return mCopper; }
	inline void setCopper(const int32 pCopper) { mCopper = pCopper; }
	inline void addCopper(const int32 pCopper) { setCopper(getCopper() + pCopper); }

	inline const int32 getSilver() const { return mSilver; }
	inline void setSilver(const int32 pSilver) { mSilver = pSilver; }
	inline void addSilver(const int32 pSilver) { setSilver(getSilver() + pSilver); }

	inline const int32 getGold() const { return mGold; }
	inline void setGold(const int32 pGold) { mGold = pGold; }
	inline void addGold(const int32 pGold) { setGold(getGold() + pGold); }

	inline const int32 getPlatinum() const { return mPlatinum; }
	inline void setPlatinum(const int32 pPlatinum) { mPlatinum = pPlatinum; }
	inline void addPlatinum(const int32 pPlatinum) { setPlatinum(getPlatinum() + pPlatinum); }

	inline const bool hasCurrency() const { return (getPlatinum() + getGold() + getSilver() + getCopper()) > 0; }
	inline void getCurrency(int32& pPlatinum, int32& pGold, int32& pSilver, int32& pCopper) { pPlatinum = getPlatinum(); pGold = getGold(); pSilver = getSilver(); pCopper = getCopper(); }
	inline void setCurrency(const int32 pPlatinum, const int32 pGold, const int32 pSilver, const int32 pCopper) { setPlatinum(pPlatinum); setGold(pGold); setSilver(pSilver); setCopper(pCopper); }
	inline void addCurrency(const int32 pPlatinum, const int32 pGold, const int32 pSilver, const int32 pCopper) { addPlatinum(pPlatinum); addGold(pGold); addSilver(pSilver); addCopper(pCopper); }
	inline void removeCurrency() { setCurrency(0, 0, 0, 0); }

	inline const bool isBanker() const { return getClass() == 40; }
	void addLootItem(Item* pItem) { mLootItems.push_back(pItem); }
	std::list<Item*>& getLootItems() { return mLootItems; }
private:
	int32 mCopper = 0;
	int32 mSilver = 0;
	int32 mGold = 0;
	int32 mPlatinum = 0;
	std::list<Item*> mLootItems;

	SpawnPoint* mSpawnPoint = nullptr;
};