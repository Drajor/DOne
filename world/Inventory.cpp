#include "Inventory.h"
#include "Item.h"
#include "ItemData.h"
#include "Character.h"
#include "Utility.h"
#include "ItemGenerator.h"

Inventoryy::Inventoryy(Character* pCharacter) : mCharacter(pCharacter) {
	for (auto& i : mItems) i = nullptr;
	for (auto& i : mBank) i = nullptr;
	for (auto& i : mSharedBank) i = nullptr;
	
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

	//put(ItemGenerator::makeOneHandBlunt(0, Rarity::Common), 23);
	//put(ItemGenerator::makePowerSource(0, Rarity::Common), 23);
	//put(ItemGenerator::makeShield(0, Rarity::Common), 24);
	put(ItemGenerator::makeHead(0, Rarity::Common), 26);
	put(ItemGenerator::makeOneHandBlunt(0, Rarity::Common), 27);
	put(ItemGenerator::makeChest(0, Rarity::Common), 28);
	put(ItemGenerator::makeAugmentation(0, Rarity::Common), 29);
	//put(ItemGenerator::makeHead(0, Rarity::Common, 5.0), 27);
	//put(ItemGenerator::makeHead(0, Rarity::Common, 6.0), 28);
	//put(ItemGenerator::makeHead(0, Rarity::Common, 7.0), 29);
	//put(ItemGenerator::makeHead(0, Rarity::Common, 8.0), 30);
	//put(ItemGenerator::makeRandomContainer(Rarity::Common), 26);
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

	if (ds.check() == false) {
		Log::error("[Inventory] Bad Write: Written: " + std::to_string(ds.getBytesWritten()) + " Size: " + std::to_string(ds.getSize()));
	}

	return data;
}

Item* Inventoryy::getItem(const uint32 pSlot) const {
	// Getting: Cursor
	if (SlotID::isCursor(pSlot)) {
		Item* item = mCursor.empty() ? nullptr : mCursor.front();
		if (item) EXPECTED_BOOL(item->getSlot() == pSlot); // Testing / Debug.
		return item;
	}
		

	// Getting: Main Inventory or Worn.
	if (SlotID::isMainInventory(pSlot) || SlotID::isWorn(pSlot)) {
		Item* item = mItems[pSlot];
		if (item) EXPECTED_BOOL(item->getSlot() == pSlot); // Testing / Debug.
		return item;
	}
		

	// Getting: Main Contents.
	if (SlotID::isMainContents(pSlot)) {
		const uint32 parentSlotID = SlotID::getParentSlot(pSlot);
		Item* parentContainer = getItem(parentSlotID);
		EXPECTED_PTR(parentContainer);
		const uint32 subIndex = SlotID::getSubIndex(pSlot);
		Item* item = parentContainer->getContents(subIndex);
		if (item) EXPECTED_BOOL(item->getSlot() == pSlot); // Testing / Debug.
		return item;
	}
	
	// Getting: Bank.
	if (SlotID::isBank(pSlot)) {
		const uint32 index = SlotID::getIndex(pSlot);
		EXPECTED_PTR(SlotID::isValidBankIndex(index));
		Item* item = mBank[index];
		if (item) EXPECTED_BOOL(item->getSlot() == pSlot); // Testing / Debug.
		return item;
	}

	// Getting: Bank Contents.
	if (SlotID::isBankContents(pSlot)) {
		const uint32 parentSlotID = SlotID::getParentSlot(pSlot);
		Item* parentContainer = getItem(parentSlotID);
		EXPECTED_PTR(parentContainer);
		const uint32 subIndex = SlotID::getSubIndex(pSlot);
		Item* item = parentContainer->getContents(subIndex);
		if (item) EXPECTED_BOOL(item->getSlot() == pSlot); // Testing / Debug.
		return item;
	}

	// Getting: Shared Bank.
	if (SlotID::isSharedBank(pSlot)) {
		const uint32 index = SlotID::getIndex(pSlot);
		EXPECTED_PTR(SlotID::isValidSharedBankIndex(index));
		Item* item = mSharedBank[index];
		if (item) EXPECTED_BOOL(item->getSlot() == pSlot); // Testing / Debug.
		return item;
	}

	// Getting: Shared Bank Contents.
	if (SlotID::isSharedBankContents(pSlot)) {
		const uint32 parentSlotID = SlotID::getParentSlot(pSlot);
		Item* parentContainer = getItem(parentSlotID);
		EXPECTED_PTR(parentContainer);
		const uint32 subIndex = SlotID::getSubIndex(pSlot);
		Item* item = parentContainer->getContents(subIndex);
		if (item) EXPECTED_BOOL(item->getSlot() == pSlot); // Testing / Debug.
		return item;
	}

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

	// Putting item in Main Inventory or Worn.
	if (SlotID::isMainInventory(pSlot) || SlotID::isWorn(pSlot)) {
		Item* existingItem = getItem(pSlot);
		EXPECTED_BOOL(existingItem == nullptr); // Failure = desync.
		pItem->setSlot(pSlot);
		mItems[pSlot] = pItem;
		return true;
	}

	// Putting: Bank.
	if (SlotID::isBank(pSlot)) {
		Item* existingItem = getItem(pSlot);
		EXPECTED_BOOL(existingItem == nullptr); // Failure = desync.
		pItem->setSlot(pSlot);

		const uint32 index = SlotID::getIndex(pSlot);
		EXPECTED_BOOL(SlotID::isValidBankIndex(index));
		mBank[index] = pItem;
		return true;
	}

	// Putting: Shared Bank.
	if (SlotID::isSharedBank(pSlot)) {
		// TODO: Check No Drop / Attuned.
		Item* existingItem = getItem(pSlot);
		EXPECTED_BOOL(existingItem == nullptr); // Failure = desync.
		pItem->setSlot(pSlot);

		const uint32 index = SlotID::getIndex(pSlot);
		EXPECTED_BOOL(SlotID::isValidSharedBankIndex(index));
		mSharedBank[index] = pItem;
		return true;
	}

	// Putting: Main Contents.
	if (SlotID::isMainContents(pSlot)) {
		EXPECTED_BOOL(putContainer(pItem, pSlot));
		EXPECTED_BOOL(pItem->getSlot() == pSlot); // Testing / Debug.
		return true;
	}

	// Putting: Bank Contents.
	if (SlotID::isBankContents(pSlot)) {
		EXPECTED_BOOL(putContainer(pItem, pSlot));
		EXPECTED_BOOL(pItem->getSlot() == pSlot); // Testing / Debug.
		return true;
	}

	// Putting: Shared Bank Contents.
	if (SlotID::isSharedBankContents(pSlot)) {
		EXPECTED_BOOL(putContainer(pItem, pSlot));
		EXPECTED_BOOL(pItem->getSlot() == pSlot); // Testing / Debug.
		return true;
	}

	return false;
}

const bool Inventoryy::putContainer(Item* pItem, const uint32 pSlot) {
	EXPECTED_BOOL(pItem);

	// Determine the parent slot.
	const uint32 parentSlotID = SlotID::getParentSlot(pSlot);
	// Check: Parent is a valid Item.
	Item* parentContainer = getItem(parentSlotID);
	EXPECTED_BOOL(parentContainer);
	// Check: Parent is a valid container.
	EXPECTED_BOOL(parentContainer->isContainer());
	const uint32 subIndex = SlotID::getSubIndex(pSlot);
	// Check: Index is valid for container size.
	EXPECTED_BOOL(parentContainer->getContainerSlots() > subIndex);
	// Check: No existing Item where pItem is being put
	EXPECTED_BOOL(parentContainer->getContents(subIndex) == nullptr);
	// Finally try and put the Item inside the container.
	EXPECTED_BOOL(parentContainer->setContents(pItem, subIndex));

	return true;
}

const bool Inventoryy::move(const uint32 pFromSlot, const uint32 pToSlot, const uint32 pStackSize) {

	// NOTE: The client sends this when an item is summoned to their cursor.
	if (pFromSlot == pToSlot) {
		// Debug logging until I understand any other circumstances this can occur in.
		Log::info("MoveItem: From: " + std::to_string(pFromSlot) + " To: " + std::to_string(pToSlot) + " Stack: " + std::to_string(pStackSize));
		return true;
	}

	// Deleting an Item.
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

	// Picking an Item up.
	if (SlotID::isCursor(pFromSlot)) {
		EXPECTED_BOOL(_putDown(pToSlot, pStackSize));
		return true;
	}

	// Putting an Item down.
	if (SlotID::isCursor(pToSlot)) {
		EXPECTED_BOOL(_pickUp(pFromSlot, pStackSize));
		return true;
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

const bool Inventoryy::_clear(const uint32 pSlot) {
	if (SlotID::isMainInventory(pSlot) || SlotID::isWorn(pSlot)) {
		mItems[pSlot] = nullptr;
		return true;
	}
	// Clearing: Bank slot.
	if (SlotID::isBank(pSlot)) {
		uint32 index = SlotID::getIndex(pSlot);
		EXPECTED_BOOL(SlotID::isValidBankIndex(index));
		mBank[index] = nullptr;
		return true;
	}
	// Clearing: Shared Bank slot.
	if (SlotID::isSharedBank(pSlot)) {
		uint32 index = SlotID::getIndex(pSlot);
		EXPECTED_BOOL(SlotID::isValidSharedBankIndex(index));
		mSharedBank[index] = nullptr;
		return true;
	}

	// Clearing: Main Contents.
	if (SlotID::isMainContents(pSlot)) {
		EXPECTED_BOOL(_clearContainerSlot(pSlot));
		return true;
	}

	// Clearing: Bank Contents.
	if (SlotID::isBankContents(pSlot)) {
		EXPECTED_BOOL(_clearContainerSlot(pSlot));
		return true;
	}

	// Clearing: Shared Bank Contents.
	if (SlotID::isSharedBankContents(pSlot)) {
		EXPECTED_BOOL(_clearContainerSlot(pSlot));
		return true;
	}

	return false;
}

const bool Inventoryy::_clearContainerSlot(const uint32 pSlot) {
	const uint32 parentSlot = SlotID::getParentSlot(pSlot);
	const uint32 subIndex = SlotID::getSubIndex(pSlot);

	if (SlotID::isMainContents(pSlot)) EXPECTED_BOOL(SlotID::isMainInventory(parentSlot));
	if (SlotID::isBankContents(pSlot)) EXPECTED_BOOL(SlotID::isBank(parentSlot));
	if (SlotID::isSharedBankContents(pSlot)) EXPECTED_BOOL(SlotID::isSharedBank(parentSlot));

	EXPECTED_BOOL(SlotID::subIndexValid(subIndex));

	Item* container = getItem(parentSlot);
	EXPECTED_BOOL(container);
	EXPECTED_BOOL(container->isContainer());
	EXPECTED_BOOL(container->getContainerSlots() > subIndex);
	EXPECTED_BOOL(container->clearContents(subIndex));

	return true;
}

const bool Inventoryy::_putDown(const uint32 pToSlot, const uint32 pStackSize) {
	Item* cursorItem = _popCursor();
	EXPECTED_BOOL(cursorItem); // Failure = desync.

	// Check: Existing Item in pToSlot.
	Item* existing = getItem(pToSlot);
	if (existing) {
		// Pick up the existing Item.
		EXPECTED_BOOL(_pickUp(pToSlot, existing->getStacks()));

		// TODO: This is where stacks are merged.
	}

	EXPECTED_BOOL(put(cursorItem, pToSlot));

	// Check: Worn Item being equipped.
	if (SlotID::isWorn(pToSlot))
		_calculateAdd(cursorItem);

	// Check: Container being put down.
	if (cursorItem->isContainer())
		cursorItem->updateContentsSlots();

	return true;
}

const bool Inventoryy::_pickUp(const uint32 pFromSlot, const uint32 pStackSize) {
	Item* pickUp = getItem(pFromSlot);
	EXPECTED_BOOL(pickUp);
	EXPECTED_BOOL(pushCursor(pickUp));
	EXPECTED_BOOL(_clear(pFromSlot));
	pickUp->clearParent();
	pickUp->clearSubIndex();

	// Check: Worn Item being un-equipped.
	if (SlotID::isWorn(pFromSlot))
		_calculateRemove(pickUp);

	// TODO: This is where stacks are split.

	return true;
}
