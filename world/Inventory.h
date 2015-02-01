#pragma once

#include "Constants.h"
#include "Bonuses.h"
#include <functional>
#include <array>

namespace Data {
	struct Item;
	struct Inventory;
}

class ILog;
class ILogFactory;
class ItemFactory;
class Item;

//namespace SearchFlags {
//	enum : u32 {
//		Worn = 0x01,
//		Main = 0x02,
//		MainContents = 0x03,
//		Bank = 0x04,
//		BankContents = 0x05,
//		SharedBank = 0x06,
//		SharedBankContents = 0x07,
//		Cursor = 0x08,
//
//		All = Worn + Main + MainContents + Bank + BankContents + SharedBank + SharedBankContents + cursor,
//	};
//}

class Inventoryy : public Bonuses { // We get an extra y for now.

public:
	Inventoryy();
	~Inventoryy();

	const bool initialise(Data::Inventory* pData, ItemFactory* pItemFactory, ILogFactory* pLogFactory, const String& pCharacterName);

	const bool updateForSave(Data::Inventory& pInventoryData);
	const bool loadFromSave(Data::Inventory& pInventoryData);
	Item* loadItem(Data::Item& pItem);
	// Items

	static constexpr u32 getPrimarySlotIndex(const u32 pSlotID);
	static const u32 getContainerSlotIndex(const u32 pSlotID);

	// Puts pItem at pSlot. Excluding the cursor, pSlot is expected to be empty (nullptr).
	const bool put(Item* pItem, const u32 pSlot);
	Item* get(const u32 pSlot) const;
	const bool moveItem(const u32 pFromSlot, const u32 pToSlot, const u32 pStacks);
	const bool moveCurrency(const u32 pFromSlot, const u32 pToSlot, const u32 pFromType, const u32 pToType, const i32 pAmount);


	inline Item* find(std::function<bool(Item*)> pPredicate) const {
		for (auto i : mItems) {
			if (pPredicate(i))
				return i;
		}
		return nullptr;
	}

	Item* find(const u32 pItemID, const u32 pInstanceID) const;

	const unsigned char* getData(u32& pSize); // Caller responsible for delete.
	
	const bool consume(const u32 pSlot, const u32 pStacks);
	const bool pushCursor(Item* pItem);
	inline const bool isCursorEmpty() const { return mCursorItems.empty(); }

	inline Item* peekCursor() const { return _peekCursor(); }
	inline std::list<Item*> getCursor() { return mCursorItems; }

	// Currency

	// Currency
	inline const i32 getCursorPlatinum() const { return mCurrency[CurrencySlot::Cursor][CurrencyType::Platinum]; }
	inline const i32 getPersonalPlatinum() const { return mCurrency[CurrencySlot::Personal][CurrencyType::Platinum]; }
	inline const i32 getBankPlatinum() const { return mCurrency[CurrencySlot::Bank][CurrencyType::Platinum]; }
	inline const i32 getTradePlatinum() const { return mCurrency[CurrencySlot::Trade][CurrencyType::Platinum]; }
	inline const i32 getSharedBankPlatinum() const { return mCurrency[CurrencySlot::SharedBank][CurrencyType::Platinum]; }

	inline const i32 getCursorGold() const { return mCurrency[CurrencySlot::Cursor][CurrencyType::Gold]; }
	inline const i32 getPersonalGold() const { return mCurrency[CurrencySlot::Personal][CurrencyType::Gold]; }
	inline const i32 getBankGold() const { return mCurrency[CurrencySlot::Bank][CurrencyType::Gold]; }
	inline const i32 getTradeGold() const { return mCurrency[CurrencySlot::Trade][CurrencyType::Gold]; }

	inline const i32 getCursorSilver() const { return mCurrency[CurrencySlot::Cursor][CurrencyType::Silver]; }
	inline const i32 getPersonalSilver() const { return mCurrency[CurrencySlot::Personal][CurrencyType::Silver]; }
	inline const i32 getBankSilver() const { return mCurrency[CurrencySlot::Bank][CurrencyType::Silver]; }
	inline const i32 getTradeSilver() const { return mCurrency[CurrencySlot::Trade][CurrencyType::Silver]; }

	inline const i32 getCursorCopper() const { return mCurrency[CurrencySlot::Cursor][CurrencyType::Copper]; }
	inline const i32 getPersonalCopper() const { return mCurrency[CurrencySlot::Personal][CurrencyType::Copper]; }
	inline const i32 getBankCopper() const { return mCurrency[CurrencySlot::Bank][CurrencyType::Copper]; }
	inline const i32 getTradeCopper() const { return mCurrency[CurrencySlot::Trade][CurrencyType::Copper]; }
	
	const bool addCurrency(const u32 pSlot, const u32 pType, const i32 pAmount);

	const bool addCurrency(const u32 pSlot, const i32 pPlatinum, const i32 pGold, const i32 pSilver, const i32 pCopper);

	inline bool removeCurrency(const u32 pSlot, const u32 pType, const i32 pAmount) { mCurrency[pSlot][pType] -= pAmount; return true; } // TODO: Make this not shit ;)

	// Sets the currency of pType at pSlot to pAmount.
	inline void setCurrency(const u32 pSlot, const u32 pType, const i32 pAmount) { mCurrency[pSlot][pType] = pAmount; }
	
	// Adds 'personal' currency.
	const bool addCurrency(const i32 pPlatinum, const i32 pGold, const i32 pSilver, const i32 pCopper);

	// Removes 'personal' currency.
	const bool removeCurrency(const i32 pPlatinum, const i32 pGold, const i32 pSilver, const i32 pCopper);

	// Returns the total value of all currency in copper pieces.
	const u64 getTotalCurrency() const;

	// Returns the total value of cursor currency in copper pieces.
	const u64 getTotalCursorCurrency() const;

	// Returns the total value of personal currency in copper pieces.
	const u64 getTotalPersonalCurrency() const;

	// Returns the total  value of bank currency in copper pieces.
	const u64 getTotalBankCurrency() const;

	// Returns the total  value of trade currency in copper pieces.
	const u64 getTotalTradeCurrency() const;

	// Returns the total value of shared bank currency in copper pieces.
	const u64 getTotalSharedBankCurrency() const;

	const bool currencyValid() const;

	// Alternate Currency

	// Radiant Crystals.
	inline const u32 getRadiantCrystals() const { return mRadiantCrystals; }
	inline const u32 getTotalRadiantCrystals() const { return mTotalRadiantCrystals; }
	inline void addRadiantCrystals(const u32 pCrystals) { mRadiantCrystals += pCrystals; mTotalRadiantCrystals += pCrystals; }
	const bool removeRadiantCrystals(const u32 pCrystals);
	
	// Sets the number of Radiant Crystals. Should only be used during initialisation.
	inline void _setRadiantCrystals(const u32 pCurrent, const u32 pTotal) { mRadiantCrystals = pCurrent; mTotalRadiantCrystals = pTotal; }

	// Ebon Crystals.
	inline const u32 getEbonCrystals() const { return mEbonCrystals; }
	inline const u32 getTotalEbonCrystals() const { return mTotalEbonCrystals; }
	inline void addEbonCrystals(const u32 pCrystals) { mEbonCrystals += pCrystals; mTotalEbonCrystals += pCrystals; }
	const bool removeEbonCrystals(const u32 pCrystals);

	// Sets the number of Ebon Crystals. Should only be used during initialisation.
	inline void _setEbonCrystals(const u32 pCurrent, const u32 pTotal) { mEbonCrystals = pCurrent; mTotalEbonCrystals = pTotal; }

	// Returns the quantity of an alternate currency by ID.
	const u32 getAlternateCurrencyQuantity(const u32 pCurrencyID) const;

	// Returns a reference to the alternate currencies.
	inline std::map<u32, u32>& getAlternateCurrency() { return mAlternateCurrency; }

	// Sets the quantity of an alternate currency by ID.
	inline void setAlternateCurrencyQuantity(const u32 pCurrencyID, const u32 pQuantity) { mAlternateCurrency[pCurrencyID] = pQuantity; }

	// Adds a specified quantity to an alternate currency.
	void addAlternateCurrency(const u32 pCurrencyID, const u32 pQuantity);

	// Removes a specified quantity from an alternate currency.
	void removeAlternateCurrency(const u32 pCurrencyID, const u32 pQuantity);

	void getTradeItems(std::list<Item*>& pItems) const;
	
	void clearTradeItems();

	const bool onTradeAccept();
	const bool onTradeCancel();

	// Finds a slot ID for an Item. If no slot ID is found, SlotID::CURSOR is returned.
	const u32 findEmptySlot(Item* pItem) const;

	const u32 findSlotFor(const bool pContainer, const u8 pItemSize) const;
	Item* findFirst(const u8 pItemType) const;

	// Finds and returns the first Item with pItemID that does not have full stacks.
	Item* findPartialStack(const u32 pItemID) const;

	void updateConsumables();

	inline const bool isAutoFood(Item* pItem) const { return pItem == mAutoFood; }
	inline const bool isAutoDrink(Item* pItem) const { return pItem == mAutoDrink; }

	inline const i32 _getCurrency(const u32 pSlot, const u32 pType) const { return mCurrency[pSlot][pType]; }

private:

	bool mInitialised = false;
	ILog* mLog = nullptr;
	ItemFactory* mItemFactory = nullptr;
	Data::Inventory* mData = nullptr;

	// Removes and returns the first Item in the cursor list.
	Item* _popCursor();

	// Returns the first Item in the cursor list.
	Item* _peekCursor() const;

	const bool _clear(const u32 pSlot);

	void _calculateAdd(Item* pItem);
	void _calculateRemove(Item* pItem);

	std::array<Item*, SlotID::MaxPrimarySlots> mItems;
	std::list<Item*> mCursorItems;

	const bool _putDown(const u32 pToSlot, const u32 pStackSize);
	const bool _stackMergeCursor(const u32 pToSlot, const u32 pStackSize);
	const bool _pickUp(const u32 pFromSlot, const u32 pStackSize);
	Item* mAutoFood = nullptr;
	Item* mAutoDrink = nullptr;

	// Radiant Crystals
	u32 mRadiantCrystals = 0;
	u32 mTotalRadiantCrystals = 0;
	
	// Ebon Crystals
	u32 mEbonCrystals = 0;
	u32 mTotalEbonCrystals = 0;
	std::map<u32, u32> mAlternateCurrency;

	i32 mCurrency[CurrencySlot::MAX][CurrencyType::MAX];
};