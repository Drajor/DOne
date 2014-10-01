#include "Inventory.h"
#include "Item.h"
#include "ItemData.h"
#include "Character.h"
#include "Utility.h"
#include "ItemGenerator.h"

Inventoryy::Inventoryy(Character* pCharacter) : mCharacter(pCharacter) {
	for (auto& i : mItems) i = nullptr;

	//auto itemData = new ItemData();
	//itemData->mID = 2;
	//itemData->mInstanceID = 2;
	//itemData->mSlot = 23;
	//itemData->mItemType = 17;
	//itemData->mClasses = 65535;
	//itemData->mIcon = 639;
	//strcpy(itemData->mItemName, String("Test Item").c_str());
	//strcpy(itemData->mIDFile, String("IT63").c_str());

	//auto item = new Item(itemData);
	//put(item, 23);

	//for (auto i = )

	//put(ItemGenerator::makeRandomContainer(ItemGenerator::COMMON), 23);
	//put(ItemGenerator::makeRandomContainer(ItemGenerator::MAGIC), 24);
	//put(ItemGenerator::makeRandomContainer(ItemGenerator::RARE), 25);
	//put(ItemGenerator::makeRandomContainer(ItemGenerator::ARTIFACT), 26);

	//put(ItemGenerator::makeFood(), 23);

	//put(ItemGenerator::makeDrink(), 23);
	//put(ItemGenerator::makeDrink(), 24);
	//put(ItemGenerator::makeDrink(), 25);
	//put(ItemGenerator::makeDrink(), 26);

	//put(ItemGenerator::makeFood(), 27);
	//put(ItemGenerator::makeFood(), 28);
	//put(ItemGenerator::makeFood(), 29);
	//put(ItemGenerator::makeFood(), 30);

	//put(ItemGenerator::makePowerSource(0, Rarity::Common), 23);
	//put(ItemGenerator::makePowerSource(0, Rarity::Common), 24);
	//put(ItemGenerator::makePowerSource(0, Rarity::Common), 25);
	//put(ItemGenerator::makePowerSource(0, Rarity::Common), 26);

	//put(ItemGenerator::makePowerSource(0, Rarity::Common), 27);
	//put(ItemGenerator::makePowerSource(0, Rarity::Common), 28);
	//put(ItemGenerator::makePowerSource(0, Rarity::Common), 29);
	//put(ItemGenerator::makePowerSource(0, Rarity::Common), 30);

	put(ItemGenerator::makeOneHandBlunt(0, Rarity::Common), 23);
	put(ItemGenerator::makeShield(0, Rarity::Common), 24);
	//put(ItemGenerator::makeOneHandBlunt(0, Rarity::Common), 24);
	//put(ItemGenerator::makeOneHandBlunt(0, Rarity::Common), 25);
	//put(ItemGenerator::makeOneHandBlunt(0, Rarity::Common), 36);
	//put(ItemGenerator::makeOneHandBlunt(0, Rarity::Common), 27);
	//put(ItemGenerator::makeOneHandBlunt(0, Rarity::Common), 28);
	//put(ItemGenerator::makeOneHandBlunt(0, Rarity::Common), 29);
	//put(ItemGenerator::makeOneHandBlunt(0, Rarity::Common), 30);
	
}

const unsigned char* Inventoryy::getData(uint32& pSize) {
	unsigned char * data = nullptr;
	uint32 numItems = 0;

	// First we need to calculate how much memory will be required to store the inventory data.

	// Inventory
	for (auto i = 0; i < 32; i++) {
		auto item = getItem(i);
		if (item) {
			numItems++;
			pSize += item->getDataSize();
			numItems += item->getSubItems();
		}
			
	}

	// Bank Items

	// Shared Bank Items.

	// Allocate the required memory.
	pSize += sizeof(uint32); // Item Count
	data = new unsigned char[pSize];

	Utility::DynamicStructure ds(data, pSize);
	ds.write<uint32>(numItems);

	// Second we need to copy

	// Inventory.
	for (auto i = 0; i < 32; i++) {
		auto item = getItem(i);
		if (item)
			item->copyData(ds);
	}

	// Bank Items

	// Shared Bank Items.

	if (ds.check() == false) Log::info("badcheck");

	return data;
}

Item* Inventoryy::getItem(const uint32 pSlot) const {
	// Getting: Cursor
	if (SlotID::isCursor(pSlot))
		return mCursor.empty() ? nullptr : mCursor.front();

	// Getting: Main Inventory.
	if (SlotID::isMainInventory(pSlot))
		return mItems[pSlot];

	return nullptr;
}

const bool Inventoryy::put(Item* pItem, const uint32 pSlot) {
	EXPECTED_BOOL(pItem);
	Log::info("[Inventory] PUT " + pItem->getName() + " to " + std::to_string(pSlot));

	// Putting item on cursor.
	if (SlotID::isCursor(pSlot)) {
		mCursor.push(pItem);
		pItem->setSlot(pSlot);
		return true;
	}

	// Putting item in Main Inventory.
	if (SlotID::isMainInventory(pSlot)) {
		Item* existingItem = getItem(pSlot);
		EXPECTED_BOOL(existingItem == nullptr); // Failure = desync.
		pItem->setSlot(pSlot);
		mItems[pSlot] = pItem;
		return true;
	}

	return false;
}

const bool Inventoryy::move(const uint32 pFromSlot, const uint32 pToSlot, const uint32 pStackSize) {
	// Moving To: Free Memory
	if (SlotID::isDelete(pToSlot)) {
		Log::info("[Inventory] DELETE");
		// NOTE: As far as I know the only possible slot to delete from is cursor.
		EXPECTED_BOOL(SlotID::isCursor(pFromSlot));
		Item* itemFrom = _popCursor();
		EXPECTED_BOOL(itemFrom); // Failure = desync
		EXPECTED_BOOL(SlotID::isCursor(itemFrom->getSlot())); // Failure = bug

		delete itemFrom;
		return true;
	}

	// Moving To: Cursor
	if (SlotID::isCursor(pToSlot)) {
		Item* itemFrom = getItem(pFromSlot);
		EXPECTED_BOOL(itemFrom); // Failure = desync
		EXPECTED_BOOL(itemFrom->getSlot() == pFromSlot); // Failure = bug.

		// Check: Cursor is empty.
		if (isCursorEmpty()) {
			// Push the Item from pFromSlot on to the cursor.
			EXPECTED_BOOL(pushCursor(itemFrom));
			// Free pFromSlot
			_set(nullptr, pFromSlot);
			return true;
		}

		// Cursor is not empty.
		auto cursorItem = _popCursor();
		EXPECTED_BOOL(cursorItem); // Failure = desync
		EXPECTED_BOOL(SlotID::isCursor(cursorItem->getSlot())); // Failure = bug.

		// Push the Item from pFromSlot on to the cursor.
		EXPECTED_BOOL(pushCursor(itemFrom));
		// Put the Item that was on the cursor into pFromSlot.
		_set(nullptr, pFromSlot);
		EXPECTED_BOOL(put(cursorItem, pFromSlot));
		return true;
	}

	// Moving To: Main Inventory
	if (SlotID::isMainInventory(pToSlot)) {
		// Moving From: Cursor
		if (SlotID::isCursor(pFromSlot)) {
			Item* cursorItem = _popCursor();
			EXPECTED_BOOL(cursorItem); // Failure = desync.
			EXPECTED_BOOL(SlotID::isCursor(cursorItem->getSlot())); // Failure = bug.

			// Check: Item in pToSlot
			Item* invItem = getItem(pToSlot);
			if (invItem) {
				EXPECTED_BOOL(invItem->getSlot() == pToSlot); // Failure = bug.
				// Push the Item from pToSlot on to the cursor.
				EXPECTED_BOOL(pushCursor(invItem));
			}

			// NOTE: _set is required because put prevents Item pointers overriding.
			_set(nullptr, pToSlot);
			EXPECTED_BOOL(put(cursorItem, pToSlot));

			return true;
		}

		return false;
	}

	return false;
}

Item* Inventoryy::_popCursor() {
	EXPECTED_PTR(!mCursor.empty());
	Item* item = mCursor.front();
	mCursor.pop();
	return item;
}

const bool Inventoryy::pushCursor(Item* pItem) {
	EXPECTED_BOOL(pItem);
	mCursor.push(pItem);
	pItem->setSlot(SlotID::CURSOR);
	return true;
}

void Inventoryy::_set(Item* pItem, const uint32 pSlot) {
	mItems[pSlot] = pItem;
}

const bool Inventoryy::consume(uint32 pSlot) {
	// NOTE: UF will only consume from the Main Inventory
	return true;
}

void Inventoryy::_calculateAdd(Item* pItem) {
	EXPECTED(pItem);

	mStrength += pItem->_getStrength();
	mStamina += pItem->_getStamina();
	mIntelligence += pItem->_getIntelligence();
	mWisdom += pItem->_getWisdom();
	mAgility += pItem->_getAgility();
	mDexterity += pItem->_getDexterity();
	mCharisma += pItem->_getCharisma();

	mHeroicStrength += pItem->_getHeroicStrength();
	mHeroicStamina += pItem->_getHeroicStamina();
	mHeroicIntelligence += pItem->_getHeroicIntelligence();
	mHeroicWisdom += pItem->_getHeroicWisdom();
	mHeroicAgility += pItem->_getHeroicAgility();
	mHeroicDexterity += pItem->_getHeroicDexterity();
	mHeroicCharisma += pItem->_getHeroicCharisma();

	mMagicResist += pItem->_getMagicResist();
	mFireResist += pItem->_getFireResist();
	mColdResist += pItem->_getColdResist();
	mDiseaseResist += pItem->_getDiseaseResist();
	mPoisonResist += pItem->_getPoisonResist();
	mCorruptionResist += pItem->getCorruptionResist();

	mHeroicMagicResist += pItem->_getHeroicMagicResist();
	mHeroicFireResist += pItem->_getHeroicFireResist();
	mHeroicColdResist += pItem->_getHeroicColdResist();
	mHeroicDiseaseResist += pItem->_getHeroicDiseaseResist();
	mHeroicPoisonResist += pItem->_getHeroicPoisonResist();
	mHeroicCorruptionResist += pItem->_getHeroicCorruptionResist();

	mHealth += pItem->_getHealth();
	mMana += pItem->_getMana();
	mEndurance += pItem->_getEndurance();

	mHealthRegen += pItem->_getHealthRegen();
	mManaRegen += pItem->_getManaRegen();
	mEnduranceRegen += pItem->_getEnduranceRegen();
}

void Inventoryy::_calculateRemove(Item* pItem) {
	EXPECTED(pItem);

	mStrength -= pItem->_getStrength();
	mStamina -= pItem->_getStamina();
	mIntelligence -= pItem->_getIntelligence();
	mWisdom -= pItem->_getWisdom();
	mAgility -= pItem->_getAgility();
	mDexterity -= pItem->_getDexterity();
	mCharisma -= pItem->_getCharisma();

	mHeroicStrength -= pItem->_getHeroicStrength();
	mHeroicStamina -= pItem->_getHeroicStamina();
	mHeroicIntelligence -= pItem->_getHeroicIntelligence();
	mHeroicWisdom -= pItem->_getHeroicWisdom();
	mHeroicAgility -= pItem->_getHeroicAgility();
	mHeroicDexterity -= pItem->_getHeroicDexterity();
	mHeroicCharisma -= pItem->_getHeroicCharisma();

	mMagicResist -= pItem->_getMagicResist();
	mFireResist -= pItem->_getFireResist();
	mColdResist -= pItem->_getColdResist();
	mDiseaseResist -= pItem->_getDiseaseResist();
	mPoisonResist -= pItem->_getPoisonResist();
	mCorruptionResist -= pItem->getCorruptionResist();

	mHeroicMagicResist -= pItem->_getHeroicMagicResist();
	mHeroicFireResist -= pItem->_getHeroicFireResist();
	mHeroicColdResist -= pItem->_getHeroicColdResist();
	mHeroicDiseaseResist -= pItem->_getHeroicDiseaseResist();
	mHeroicPoisonResist -= pItem->_getHeroicPoisonResist();
	mHeroicCorruptionResist -= pItem->_getHeroicCorruptionResist();

	mHealth -= pItem->_getHealth();
	mMana -= pItem->_getMana();
	mEndurance -= pItem->_getEndurance();

	mHealthRegen -= pItem->_getHealthRegen();
	mManaRegen -= pItem->_getManaRegen();
	mEnduranceRegen -= pItem->_getEnduranceRegen();
}

void Inventoryy::updateConsumables() {
	Item* food = findFirst(ItemType::Food);
	
	// There was already food.
	if (mAutoFood) {
		// Now there is no food.
		if (!food) {
			_calculateRemove(mAutoFood);
			mAutoFood = nullptr;
		}
		// There is new food.
		else if (mAutoFood != food) {
			_calculateRemove(mAutoFood);
			mAutoFood = food;
			_calculateAdd(mAutoFood);
		}
	}
	// There was no food, now there is.
	else if(food) {
		mAutoFood = food;
		_calculateAdd(mAutoFood);
	}

	Item* drink = findFirst(ItemType::Drink);
	
	// There was already drink.
	if (mAutoDrink) {
		// Now there is no drink.
		if (!drink) {
			_calculateRemove(mAutoDrink);
			mAutoDrink = nullptr;
		}
		// There is new drink.
		else if (mAutoDrink != drink) {
			_calculateRemove(mAutoDrink);
			mAutoDrink = drink;
			_calculateAdd(mAutoDrink);
		}
	}
	// There was no drink, now there is.
	else if (drink) {
		mAutoDrink = drink;
		_calculateAdd(mAutoDrink);
	}
}

Item* Inventoryy::findFirst(const uint8 pItemType) {
	// Search Main Inventory slots first.
	for (int i = SlotID::MAIN_0; i <= SlotID::MAIN_7; i++) {
		if (mItems[i] && mItems[i]->getItemType() == pItemType)
			return mItems[i];
	}

	// Search: Slots inside each primary slot.
	for (int i = SlotID::MAIN_0; i <= SlotID::MAIN_7; i++) {
		if (mItems[i] && mItems[i]->isContainer()) {
			Item* item = mItems[i]->findFirst(pItemType);
			if (item) {
				return item;
			}
		}
	}
	return nullptr;
}
