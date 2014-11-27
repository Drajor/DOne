#pragma once

#include "Constants.h"
#include "Bonuses.h"

class Item;
class Inventoryy : public Bonuses { // We get an extra y for now.

public:
	Inventoryy();

	// Items

	// Puts pItem at pSlot. Excluding the cursor, pSlot is expected to be empty (nullptr).
	const bool put(Item* pItem, const uint32 pSlot);
	const bool putContainer(Item* pItem, const uint32 pSlot);

	// Returns the Item at pSlot or nullptr.
	Item* getItem(const uint32 pSlot) const;

	Item* find(const uint32 pItemID, const uint32 pInstanceID) const;

	const unsigned char* getData(uint32& pSize); // Caller responsible for delete.
	const bool moveItem(const uint32 pFromSlot, const uint32 pToSlot, const uint32 pStackSize);
	const bool consume(const uint32 pSlot, const uint32 pStacks);
	const bool pushCursor(Item* pItem);
	inline const bool isCursorEmpty() const { return mCursor.empty(); }

	inline Item* peekCursor() const { return _peekCursor(); }
	inline std::list<Item*> getCursor() { return mCursor; }

	// Currency

	const bool moveCurrency(const uint32 pFromSlot, const uint32 pToSlot, const uint32 pFromType, const uint32 pToType, const int32 pAmount);

	// Currency
	inline const int32 getCursorPlatinum() const { return mCurrency[CurrencySlot::Cursor][CurrencyType::Platinum]; }
	inline const int32 getPersonalPlatinum() const { return mCurrency[CurrencySlot::Personal][CurrencyType::Platinum]; }
	inline const int32 getBankPlatinum() const { return mCurrency[CurrencySlot::Bank][CurrencyType::Platinum]; }
	inline const int32 getTradePlatinum() const { return mCurrency[CurrencySlot::Trade][CurrencyType::Platinum]; }
	inline const int32 getSharedBankPlatinum() const { return mCurrency[CurrencySlot::SharedBank][CurrencyType::Platinum]; }

	inline const int32 getCursorGold() const { return mCurrency[CurrencySlot::Cursor][CurrencyType::Gold]; }
	inline const int32 getPersonalGold() const { return mCurrency[CurrencySlot::Personal][CurrencyType::Gold]; }
	inline const int32 getBankGold() const { return mCurrency[CurrencySlot::Bank][CurrencyType::Gold]; }
	inline const int32 getTradeGold() const { return mCurrency[CurrencySlot::Trade][CurrencyType::Gold]; }

	inline const int32 getCursorSilver() const { return mCurrency[CurrencySlot::Cursor][CurrencyType::Silver]; }
	inline const int32 getPersonalSilver() const { return mCurrency[CurrencySlot::Personal][CurrencyType::Silver]; }
	inline const int32 getBankSilver() const { return mCurrency[CurrencySlot::Bank][CurrencyType::Silver]; }
	inline const int32 getTradeSilver() const { return mCurrency[CurrencySlot::Trade][CurrencyType::Silver]; }

	inline const int32 getCursorCopper() const { return mCurrency[CurrencySlot::Cursor][CurrencyType::Copper]; }
	inline const int32 getPersonalCopper() const { return mCurrency[CurrencySlot::Personal][CurrencyType::Copper]; }
	inline const int32 getBankCopper() const { return mCurrency[CurrencySlot::Bank][CurrencyType::Copper]; }
	inline const int32 getTradeCopper() const { return mCurrency[CurrencySlot::Trade][CurrencyType::Copper]; }
	
	const bool addCurrency(const uint32 pSlot, const uint32 pType, const int32 pAmount);

	const bool addCurrency(const uint32 pSlot, const int32 pPlatinum, const int32 pGold, const int32 pSilver, const int32 pCopper);

	inline bool removeCurrency(const uint32 pSlot, const uint32 pType, const int32 pAmount) { mCurrency[pSlot][pType] -= pAmount; return true; } // TODO: Make this not shit ;)

	// Sets the currency of pType at pSlot to pAmount.
	inline void setCurrency(const uint32 pSlot, const uint32 pType, const int32 pAmount) { mCurrency[pSlot][pType] = pAmount; }
	
	// Adds 'personal' currency.
	const bool addCurrency(const int32 pPlatinum, const int32 pGold, const int32 pSilver, const int32 pCopper);

	// Removes 'personal' currency.
	const bool removeCurrency(const int32 pPlatinum, const int32 pGold, const int32 pSilver, const int32 pCopper);

	// Returns the total value of all currency in copper pieces.
	const uint64 getTotalCurrency() const;

	// Returns the total value of cursor currency in copper pieces.
	const uint64 getTotalCursorCurrency() const;

	// Returns the total value of personal currency in copper pieces.
	const uint64 getTotalPersonalCurrency() const;

	// Returns the total  value of bank currency in copper pieces.
	const uint64 getTotalBankCurrency() const;

	// Returns the total value of shared bank currency in copper pieces.
	const uint64 getTotalSharedBankCurrency() const;

	const bool currencyValid() const;

	// Alternate Currency

	// Radiant Crystals.
	inline const uint32 getRadiantCrystals() const { return mRadiantCrystals; }
	inline const uint32 getTotalRadiantCrystals() const { return mTotalRadiantCrystals; }
	inline void addRadiantCrystals(const uint32 pCrystals) { mRadiantCrystals += pCrystals; mTotalRadiantCrystals += pCrystals; }
	const bool removeRadiantCrystals(const uint32 pCrystals);
	
	// Sets the number of Radiant Crystals. Should only be used during initialisation.
	inline void _setRadiantCrystals(const uint32 pCurrent, const uint32 pTotal) { mRadiantCrystals = pCurrent; mTotalRadiantCrystals = pTotal; }

	// Ebon Crystals.
	inline const uint32 getEbonCrystals() const { return mEbonCrystals; }
	inline const uint32 getTotalEbonCrystals() const { return mTotalEbonCrystals; }
	inline void addEbonCrystals(const uint32 pCrystals) { mEbonCrystals += pCrystals; mTotalEbonCrystals += pCrystals; }
	const bool removeEbonCrystals(const uint32 pCrystals);

	// Sets the number of Ebon Crystals. Should only be used during initialisation.
	inline void _setEbonCrystals(const uint32 pCurrent, const uint32 pTotal) { mEbonCrystals = pCurrent; mTotalEbonCrystals = pTotal; }

	void getTradeItems(std::list<Item*>& pItems) const;
	const bool clearTradeItems();
	const bool onTradeCancel();

	const uint32 findEmptySlot(Item* pItem) const;
	const uint32 findEmptySlot(const bool pContainer, const uint8 pItemSize) const;
	Item* findFirst(const uint8 pItemType) const;

	// Finds and returns the first Item with pItemID that does not have full stacks.
	Item* findStackable(const uint32 pItemID) const;

	void updateConsumables();

	inline const bool isAutoFood(Item* pItem) const { return pItem == mAutoFood; }
	inline const bool isAutoDrink(Item* pItem) const { return pItem == mAutoDrink; }

private:
	Item* _popCursor();
	Item* _peekCursor() const;
	const bool _clear(const uint32 pSlot);
	const bool _clearContainerSlot(const uint32 pSlot);

	void _calculateAdd(Item* pItem);
	void _calculateRemove(Item* pItem);

	Item* mItems[SlotID::MAIN_SLOTS]; // Slots 0 - 30
	std::list<Item*> mCursor; // Slot 31
	Item* mBank[SlotID::BANK_SLOTS]; // Slots 2000 - 2023
	Item* mSharedBank[SlotID::SHARED_BANK_SLOTS]; // Slots 2500 - 2501
	Item* mTrade[SlotID::TRADE_SLOTS]; // Slots 3000 - ?
	
	
	const bool _putDown(const uint32 pToSlot, const uint32 pStackSize);
	const bool _stackMergeCursor(const uint32 pToSlot, const uint32 pStackSize);
	const bool _pickUp(const uint32 pFromSlot, const uint32 pStackSize);
	Item* mAutoFood = nullptr;
	Item* mAutoDrink = nullptr;

	// Radiant Crystals
	uint32 mRadiantCrystals = 0;
	uint32 mTotalRadiantCrystals = 0;
	
	// Ebon Crystals
	uint32 mEbonCrystals = 0;
	uint32 mTotalEbonCrystals = 0;

	int32 mCurrency[CurrencySlot::MAX][CurrencyType::MAX];

	inline const int32 _getCurrency(const uint32 pSlot, const uint32 pType) const { return mCurrency[pSlot][pType]; }
};