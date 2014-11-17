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
	for (auto& i : mTrade) i = nullptr;
	
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

	put(ItemGenerator::makeTwoHandBlunt(0, Rarity::Common), 23);
	//put(ItemGenerator::makePowerSource(0, Rarity::Common), 23);
	//put(ItemGenerator::makeShield(0, Rarity::Common), 24);
	put(ItemGenerator::makeHead(0, Rarity::Common), 26);
	put(ItemGenerator::makeOneHandBlunt(0, Rarity::Common), 27);
	//put(ItemGenerator::makeChest(0, Rarity::Common), 28);
	//put(ItemGenerator::makeAugmentation(0, Rarity::Common), 29);
	put(ItemGenerator::makeDice(pCharacter->getName()), SlotID::POWER_SOURCE);
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

	Item* chest0 = ItemGenerator::makeChest(0, Rarity::Common);
	chest0->setStrength(54);
	chest0->setAgility(23);
	chest0->setCharisma(58);
	put(chest0, 28);

	Item* chest1 = ItemGenerator::makeChest(0, Rarity::Common);
	chest1->setStrength(91);
	chest1->setAgility(9);
	chest1->setCharisma(56);
	put(chest1, 29);
	
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

	// Getting: Trade.
	if (SlotID::isTrade(pSlot)) {
		const uint32 index = SlotID::getIndex(pSlot);
		EXPECTED_PTR(SlotID::isValidTradeIndex(index));
		Item* item = mTrade[index];
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
		// Check: Only tradeable Items can be stored in the Shared bank.
		EXPECTED_BOOL(pItem->isTradeable());
		Item* existingItem = getItem(pSlot);
		EXPECTED_BOOL(existingItem == nullptr); // Failure = desync.
		pItem->setSlot(pSlot);

		const uint32 index = SlotID::getIndex(pSlot);
		EXPECTED_BOOL(SlotID::isValidSharedBankIndex(index));
		mSharedBank[index] = pItem;
		return true;
	}

	// Putting: Trade.
	if (SlotID::isTrade(pSlot)) {
		// TODO: Not sure whether to check that the Item is tradeable here or not. I would need to be exposed to the trading context ><
		Item* existingItem = getItem(pSlot);
		EXPECTED_BOOL(existingItem == nullptr); // Failure = desync.
		pItem->setSlot(pSlot);

		const uint32 index = SlotID::getIndex(pSlot);
		EXPECTED_BOOL(SlotID::isValidTradeIndex(index));
		mTrade[index] = pItem;
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


	// Putting an Item down.
	if (SlotID::isCursor(pFromSlot)) {
		EXPECTED_BOOL(_putDown(pToSlot, pStackSize));
		return true;
	}

	// Picking an Item up.
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

Item* Inventoryy::_peekCursor() const {
	if (isCursorEmpty()) return nullptr;
	return mCursor.front();
}

const bool Inventoryy::pushCursor(Item* pItem) {
	EXPECTED_BOOL(pItem);
	mCursor.push(pItem);
	pItem->setSlot(SlotID::CURSOR);
	return true;
}

const bool Inventoryy::consume(const uint32 pSlot, const uint32 pStacks) {
	// NOTE: UF will only consume from the Main Inventory

	auto item = getItem(pSlot);
	EXPECTED_BOOL(item);

	// Consuming the whole Item.
	if (item->getStacks() == pStacks || !item->isStackable()) {
		EXPECTED_BOOL(_clear(pSlot));
		delete item;
		item = nullptr;
	}
	// Consuming stacks.
	else {
		EXPECTED_BOOL(item->removeStacks(pStacks));
	}

	return true;
}

void Inventoryy::_calculateAdd(Item* pItem) {
	EXPECTED(pItem);

	_addStrength(pItem->_getStrength());
	_addStamina(pItem->_getStamina());
	_addIntelligence(pItem->_getIntelligence());
	_addWisdom(pItem->_getWisdom());
	_addAgility(pItem->_getAgility());
	_addDexterity(pItem->_getDexterity());
	_addCharisma(pItem->_getCharisma());

	_addHeroicStrength(pItem->_getHeroicStrength());
	_addHeroicStamina(pItem->_getHeroicStamina());
	_addHeroicIntelligence(pItem->_getHeroicIntelligence());
	_addHeroicWisdom(pItem->_getHeroicWisdom());
	_addHeroicAgility(pItem->_getHeroicAgility());
	_addHeroicDexterity(pItem->_getHeroicDexterity());
	_addHeroicCharisma(pItem->_getHeroicCharisma());

	_addMagicResist(pItem->_getMagicResist());
	_addFireResist(pItem->_getFireResist());
	_addColdResist(pItem->_getColdResist());
	_addDiseaseResist(pItem->_getDiseaseResist());
	_addPoisonResist(pItem->_getPoisonResist());
	_addCorruptionResist(pItem->getCorruptionResist());

	_addHeroicMagicResist(pItem->_getHeroicMagicResist());
	_addHeroicFireResist(pItem->_getHeroicFireResist());
	_addHeroicColdResist(pItem->_getHeroicColdResist());
	_addHeroicDiseaseResist(pItem->_getHeroicDiseaseResist());
	_addHeroicPoisonResist(pItem->_getHeroicPoisonResist());
	_addHeroicCorruptionResist(pItem->_getHeroicCorruptionResist());

	_addHealth(pItem->_getHealth());
	_addMana(pItem->_getMana());
	_addEndurance(pItem->_getEndurance());

	_addHealthRegen(pItem->_getHealthRegen());
	_addManaRegen(pItem->_getManaRegen());
	_addEnduranceRegen(pItem->_getEnduranceRegen());

	// TODO: MOD2
}

void Inventoryy::_calculateRemove(Item* pItem) {
	EXPECTED(pItem);

	_removeStrength(pItem->_getStrength());
	_removeStamina(pItem->_getStamina());
	_removeIntelligence(pItem->_getIntelligence());
	_removeWisdom(pItem->_getWisdom());
	_removeAgility(pItem->_getAgility());
	_removeDexterity(pItem->_getDexterity());
	_removeCharisma(pItem->_getCharisma());

	_removeHeroicStrength(pItem->_getHeroicStrength());
	_removeHeroicStamina(pItem->_getHeroicStamina());
	_removeHeroicIntelligence(pItem->_getHeroicIntelligence());
	_removeHeroicWisdom(pItem->_getHeroicWisdom());
	_removeHeroicAgility(pItem->_getHeroicAgility());
	_removeHeroicDexterity(pItem->_getHeroicDexterity());
	_removeHeroicCharisma(pItem->_getHeroicCharisma());

	_removeMagicResist(pItem->_getMagicResist());
	_removeFireResist(pItem->_getFireResist());
	_removeColdResist(pItem->_getColdResist());
	_removeDiseaseResist(pItem->_getDiseaseResist());
	_removePoisonResist(pItem->_getPoisonResist());
	_removeCorruptionResist(pItem->getCorruptionResist());

	_removeHeroicMagicResist(pItem->_getHeroicMagicResist());
	_removeHeroicFireResist(pItem->_getHeroicFireResist());
	_removeHeroicColdResist(pItem->_getHeroicColdResist());
	_removeHeroicDiseaseResist(pItem->_getHeroicDiseaseResist());
	_removeHeroicPoisonResist(pItem->_getHeroicPoisonResist());
	_removeHeroicCorruptionResist(pItem->_getHeroicCorruptionResist());

	_removeHealth(pItem->_getHealth());
	_removeMana(pItem->_getMana());
	_removeEndurance(pItem->_getEndurance());

	_removeHealthRegen(pItem->_getHealthRegen());
	_removeManaRegen(pItem->_getManaRegen());
	_removeEnduranceRegen(pItem->_getEnduranceRegen());
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
	// Clearing: Cursor
	// NOTE: Only the first Item is removed.
	if (SlotID::isCursor(pSlot)) {
		EXPECTED_BOOL(_popCursor());
		return true;
	}
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
	// Clearing: Trade slot.
	//if (isSl)

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

	// Check: Existing Item in pToSlot.
	Item* existing = getItem(pToSlot);
	if (existing) {
		// Potential stack merge.
		if (pStackSize > 0) {
			Item* cursorItem = _peekCursor();
			EXPECTED_BOOL(cursorItem); // Failure = desync.
			EXPECTED_BOOL(cursorItem->isStackable());

			// Check: Both Items are the same.
			if (cursorItem->getID() == existing->getID()) {
				EXPECTED_BOOL(_stackMergeCursor(pToSlot, pStackSize));
				return true;
			}
		}
	}
	
	// Pop cursor head into limbo.
	Item* cursorItem = _popCursor();
	EXPECTED_BOOL(cursorItem); // Failure = desync.

	// Pick up the existing Item. (Push cursor)
	if (existing)
		EXPECTED_BOOL(_pickUp(pToSlot, 0)); // pStackSize is set to 0 because 0 means the whole Item.

	EXPECTED_BOOL(put(cursorItem, pToSlot));

	// Check: Worn Item being equipped.
	if (SlotID::isWorn(pToSlot)) {
		// Set Item to Attuned where it is Attunable.
		if (cursorItem->isAttunable())
			// TODO: Attuned augments in cursorItem.
			cursorItem->setIsAttuned(true);

		_calculateAdd(cursorItem);
	}

	// Check: Container being put down.
	if (cursorItem->isContainer())
		cursorItem->updateContentsSlots();

	return true;
}

const bool Inventoryy::_stackMergeCursor(const uint32 pToSlot, const uint32 pStackSize) {

	Item* cursorItem = _peekCursor();
	EXPECTED_BOOL(cursorItem); // Failure = desync.
	EXPECTED_BOOL(cursorItem->isStackable());

	Item* existing = getItem(pToSlot);
	EXPECTED_BOOL(existing);
	EXPECTED_BOOL(cursorItem->getID() == existing->getID());

	const uint32 preMergeStacks = cursorItem->getStacks() + existing->getStacks();

	// Full merge.
	if (pStackSize == cursorItem->getStacks()) {

		// Add stacks from cursor to the existing Item.
		EXPECTED_BOOL(existing->addStacks(pStackSize));

		// Delete cursor Item as it has been consumed in the full stack merge.
		_popCursor();
		delete cursorItem;
		cursorItem = nullptr;

		// Check: The total number of stacks remains the same.
		EXPECTED_BOOL(existing->getStacks() == preMergeStacks);

		return true;
	}
	// Partial merge.
	else {
		// Remove stacks from cursor.
		EXPECTED_BOOL(cursorItem->removeStacks(pStackSize));
		// Add stacks from cursor to the existing Item.
		EXPECTED_BOOL(existing->addStacks(pStackSize));

		// Check: The total number of stacks remains the same.
		EXPECTED_BOOL(existing->getStacks() + cursorItem->getStacks() == preMergeStacks);

		return true;
	}
}

const bool Inventoryy::_pickUp(const uint32 pFromSlot, const uint32 pStackSize) {
	Item* pickUp = getItem(pFromSlot);
	EXPECTED_BOOL(pickUp);

	// Splitting the stack.
	if (pStackSize > 0) {
		EXPECTED_BOOL(pickUp->isStackable());
		const uint32 preSplitStacks = pickUp->getStacks();
		EXPECTED_BOOL(pickUp->getStacks() > pStackSize);
		EXPECTED_BOOL(pickUp->removeStacks(pStackSize));

		Item* split = pickUp->copy();
		split->setStacks(pStackSize);

		// Check: The total number of stacks remains the same.
		EXPECTED_BOOL(preSplitStacks == (pickUp->getStacks() + split->getStacks()));

		EXPECTED_BOOL(pushCursor(split));
		return true;
	}

	// Regular pick up.
	EXPECTED_BOOL(pushCursor(pickUp));
	EXPECTED_BOOL(_clear(pFromSlot));
	pickUp->clearParent();
	pickUp->clearSubIndex();

	// Check: Worn Item being un-equipped.
	if (SlotID::isWorn(pFromSlot))
		_calculateRemove(pickUp);

	return true;
}

Item* Inventoryy::find(const uint32 pItemID, const uint32 pInstanceID) const {

	// Search Worn Slots
	for (int i = SlotID::CHARM; i <= SlotID::AMMO; i++) {
		if (mItems[i] && mItems[i]->getID() == pItemID && mItems[i]->getInstanceID() == pInstanceID)
			return mItems[i];
	}

	// Search Primary Main.
	for (int i = SlotID::MAIN_0; i <= SlotID::MAIN_7; i++) {
		if (mItems[i] && mItems[i]->getID() == pItemID && mItems[i]->getInstanceID() == pInstanceID) {
			return mItems[i];
		}
		// Search Primary Contents.
		else if (mItems[i] && mItems[i]->isContainer()) {
			for (int j = 0; j < SlotID::MAX_CONTENTS; j++) {
				if (mItems[j] && mItems[j]->getID() == pItemID && mItems[j]->getInstanceID() == pInstanceID) {
					return mItems[j];
				}
			}
		}
	}

	return nullptr;
}

const bool Inventoryy::removeRadiantCrystals(const uint32 pCrystals) {
	EXPECTED_BOOL(mRadiantCrystals >= pCrystals);

	mRadiantCrystals -= pCrystals;
	return true;
}

const bool Inventoryy::removeEbonCrystals(const uint32 pCrystals) {
	EXPECTED_BOOL(mEbonCrystals >= pCrystals);

	mEbonCrystals -= pCrystals;
	return true;
}