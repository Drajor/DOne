#include "Inventory.h"
#include "ServiceLocator.h"
#include "Item.h"
#include "ItemData.h"
#include "Data.h"
#include "Character.h"
#include "Utility.h"
#include "ItemGenerator.h"
#include "ItemFactory.h"
#include "Limits.h"

Inventory::Inventory() : Bonuses("Inventory") {
	mItems.fill(nullptr);
	memset(mCurrency, 0, sizeof(mCurrency));
}

Inventory::~Inventory() {
	mItemFactory = nullptr;
	mData = nullptr;

	if (mLog) {
		delete mLog;
		mLog = nullptr;
	}
}

const bool Inventory::initialise(Data::Inventory* pData, ItemFactory* pItemFactory, ILogFactory* pLogFactory, const String& pCharacterName) {
	if (mInitialised) return false;
	if (!pData) return false;
	if (!pItemFactory) return false;
	if (!pLogFactory) return false;

	mData = pData;
	mItemFactory = pItemFactory;
	mLog = pLogFactory->make();

	mLog->setContext("[Inventory (" + pCharacterName + ")]");
	mLog->status("Initialising.");

	for (auto i : mData->mItems) {
		// Create Item.
		auto item = loadItem(i);
		if (!item) {
			mLog->error("Failed to load Item: ");
			return false;
		}

		// Put Item in Inventory.
		if (!put(item, i.mSlot, InventoryReason::Initalisation)) {
			mLog->error("Failed to put Item: ");
			return false;
		}

		// Item is a container.
		if (item->isContainer()) {
			// Load container contents.
			for (auto& j : i.mSubItems) {
				// Create Item.
				auto subItem = loadItem(j);
				if (!subItem) {
					mLog->error("Failed to load Item: ");
					return false;
				}

				// Put Item in container.
				if (!item->setContents(subItem, j.mSlot)){
					mLog->error("Failed to setContents Item: ");
					return false;
				}
			}
		}
		// Item has augmentations.
		else if (i.mSubItems.size() > 0) {
			for (auto& j : i.mSubItems) {
				// Create Item.
				auto subItem = loadItem(j);
				if (!subItem) {
					mLog->error("Failed to load Item: ");
					return false;
				}
				item->setAugmentation(j.mSlot, subItem);
			}
		}
	}

	mLog->status("Finished initialising.");
	mInitialised = true;
	return true;
}

Item* Inventory::loadItem(Data::Item& pItem) {
	auto item = mItemFactory->make(pItem.mItemID, pItem.mStacks);
	if (!item)
		return nullptr;

	item->setIsAttuned(pItem.mAttuned == 1 ? true : false);
	item->setLastCastTime(pItem.mLastCastTime);
	item->setCharges(pItem.mCharges);

	return item;
}

const unsigned char* Inventory::getData(u32& pSize) {
	unsigned char * data = nullptr;
	u32 numItems = 0;

	// Calculate size.
	for (auto i : mItems) {
		if (i) {
			numItems++;
			pSize += i->getDataSize(Payload::ItemPacketCharInventory, true);
		}
	}

	auto cursorItem = peekCursor();
	if (cursorItem) {
		numItems++;
		pSize += cursorItem->getDataSize(Payload::ItemPacketCharInventory, true);
	}

	// Allocate the required memory.
	pSize += sizeof(u32); // Item Count
	data = new unsigned char[pSize];

	Utility::MemoryWriter writer(data, pSize);
	
	// Write Items.
	writer.write<u32>(numItems); // Item count.
	for (auto i : mItems) {
		if (i) {
			i->copyData(writer, Payload::ItemPacketCharInventory, true);
		}
	}

	if (cursorItem)
		cursorItem->copyData(writer, Payload::ItemPacketCharInventory, true);

	// Check: The amount of data written matches what was calculated.
	if (!writer.check()) mLog->error("Bad Write: Written: " + toString(writer.getBytesWritten()) + " Size: " + toString(writer.getSize()));
	return data;
}

Item* Inventory::get(const u32 pSlot) const {
	// Handle: Item being put io cursor.
	if (SlotID::isCursor(pSlot)) return mCursorItems.empty() ? nullptr : mCursorItems.front();

	// Calculate primary index.
	const auto index = SlotID::getPrimarySlotIndex(pSlot);
	if (SlotID::isNone(pSlot)) {
		mLog->error("Failed to calculate primary index for slot ID: " + toString(pSlot));
		return false;
	}

	if (SlotID::isPrimarySlot(pSlot))
		return mItems[index];

	// Handle: Item being put io a container slot.
	else if (SlotID::isContainerSlot(pSlot)) {
		// Calculate container index.
		const auto containerIndex = getContainerSlotIndex(pSlot);
		return mItems[index] ? mItems[index]->getContents(containerIndex) : nullptr;
	}

	mLog->error("Failed to get Item, slot ID: " + toString(pSlot));
	return nullptr;
}

const bool Inventory::put(Item* pItem, const u32 pSlot, const String& pReason) {
	if (!pItem) return false;

	// All puts are logged.
	StringStream ss;
	ss << "Put item (" << pReason << ") Slot ID: " << pSlot << "| Name: " << pItem->getName() << " | ID: " << pItem->getID() << " | Stacks: " << pItem->getStacks();
	mLog->info(ss.str());

	// Handle: Item being put io cursor.
	if (SlotID::isCursor(pSlot)) {
		mCursorItems.push_front(pItem);
		pItem->setSlot(pSlot);
		return true;
	}

	// Calculate primary index.
	const auto index = SlotID::getPrimarySlotIndex(pSlot);
	if (SlotID::isNone(pSlot)) {
		mLog->error("Failed to calculate primary index for slot ID: " + toString(pSlot));
		return false;
	}
	
	// Handle: Item being put io primary slot.
	if (SlotID::isPrimarySlot(pSlot)) {
		// Check: There is no Item already in this slot.
		if (mItems[index] != nullptr) {
			mLog->error("Attempted to put Item io io occupied slot.");
			return false;
		}
		mItems[index] = pItem;
		pItem->setSlot(pSlot);

		// If a container is being put, we need to update the contents.
		if (pItem->isContainer())
			pItem->updateContentsSlots();

		return true;
	}
	// Handle: Item being put io a container slot.
	else if (SlotID::isContainerSlot(pSlot)) {
		// Calculate container index.
		const auto containerIndex = getContainerSlotIndex(pSlot);
		if (SlotID::isNone(containerIndex)) {
			mLog->error("Failed to calculate container index for slot ID: " + toString(pSlot));
			return false;
		}

		// Check: There is a valid container Item in this slot.
		if (!mItems[index] || !mItems[index]->isContainer()) {
			mLog->error("Attempted to put Item io io invalid container slot.");
			return false;
		}
		// Check: There is no Item already in this slot.
		if (mItems[index]->getContents(containerIndex)) {
			mLog->error("Attempted to put Item io io occupied container slot.");
			return false;
		}

		// Try putting pItem io the container.
		if (!mItems[index]->setContents(pItem, containerIndex)) {
			return false;
		}

		return true;
	}

	mLog->error("Failed to put Item!");
	return false;
}


const bool Inventory::moveItem(const u32 pFromSlot, const u32 pToSlot, const u32 pStacks) {
	mLog->info("MOVE " + toString(pFromSlot) + " to " + toString(pToSlot) + " | Stacks: " + toString(pStacks));

	// NOTE: The client sends this when an item is summoned to their cursor.
	if (pFromSlot == pToSlot) {
		return true;
	}

	// Handle: Deleting an Item.
	if (SlotID::isDelete(pToSlot)) {
		// Check: Delete is happening from the cursor.
		if (!SlotID::isCursor(pFromSlot)) {
			mLog->error("Attempt to delete Item from non cursor slot.");
			return false;
		}

		// Check: Deleting a valid Item.
		auto item = _popCursor();
		if (!item) {
			mLog->error("Attempt to delete invalid Item.");
			return false;
		}

		delete item;
		return true;
	}

	// Handle: Picking an Item up.
	if (SlotID::isCursor(pFromSlot)) {
		if (!_putDown(pToSlot, pStacks)) {
			mLog->error("Failed to put Item down.");
			return false;
		}

		return true;
	}

	// Handle: Picking an Item up.
	if (SlotID::isCursor(pToSlot)) {
		if (!_pickUp(pFromSlot, pStacks)) {
			mLog->error("Failed to pick Item up.");
			return false;
		}

		return true;
	}

	mLog->error("Unknown circumstances occurred.");
	return false;
}

Item* Inventory::_popCursor() {
	if (mCursorItems.empty()) {
		mLog->error("Attempt to pop cursor when it is empty.");
		return nullptr;
	}

	auto item = mCursorItems.front();
	mCursorItems.pop_front();
	return item;
}

Item* Inventory::_peekCursor() const {
	if (isCursorEmpty()) return nullptr;
	return mCursorItems.front();
}

const bool Inventory::pushCursor(Item* pItem) {
	if (!pItem) return false;

	mCursorItems.push_back(pItem);
	pItem->setSlot(SlotID::CURSOR);
	return true;
}

const bool Inventory::consume(const u32 pSlot, const u32 pStacks) {
	// NOTE: UF will only consume from the Main Inventory

	auto item = get(pSlot);
	if (!item) {
		return false;
	}

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

void Inventory::_calculateAdd(Item* pItem) {
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

	_addArmorClass(pItem->_getArmorClass());
	_addAttack(pItem->_getAttack());
	_addHealth(pItem->_getHealth());
	_addMana(pItem->_getMana());
	_addEndurance(pItem->_getEndurance());

	_addHealthRegen(pItem->_getHealthRegen());
	_addManaRegen(pItem->_getManaRegen());
	_addEnduranceRegen(pItem->_getEnduranceRegen());

	// TODO: MOD2
}

void Inventory::_calculateRemove(Item* pItem) {
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

	_removeArmorClass(pItem->_getArmorClass());
	_removeAttack(pItem->getAttack());
	_removeHealth(pItem->_getHealth());
	_removeMana(pItem->_getMana());
	_removeEndurance(pItem->_getEndurance());

	_removeHealthRegen(pItem->_getHealthRegen());
	_removeManaRegen(pItem->_getManaRegen());
	_removeEnduranceRegen(pItem->_getEnduranceRegen());
}

void Inventory::updateConsumables() {
	auto food = findFirst(ItemType::Food);
	
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

	auto drink = findFirst(ItemType::Drink);
	
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

Item* Inventory::findFirst(const u8 pItemType) const {
	// Search: Main Inventory slots first.
	for (u32 i = SlotID::MAIN_0; i <= SlotID::MAIN_7; i++) {
		if (mItems[i] && mItems[i]->getItemType() == pItemType)
			return mItems[i];
	}

	// Search: Slots inside each primary slot.
	for (u32 i = SlotID::MAIN_0; i <= SlotID::MAIN_7; i++) {
		if (mItems[i] && mItems[i]->isContainer()) {
			Item* item = mItems[i]->findFirst(pItemType);
			if (item) {
				return item;
			}
		}
	}
	return nullptr;
}

Item* Inventory::findPartialStack(const u32 pItemID) const {
	// Search: Main slots.
	for (u32 i = SlotID::MAIN_0; i <= SlotID::MAIN_7; i++) {
		auto item = mItems[i];
		if (item && item->getID() == pItemID && item->getEmptyStacks() > 0)
			return item;
	}

	// Search: Containers in main slots.
	for (u32 i = SlotID::MAIN_0; i <= SlotID::MAIN_7; i++) {
		auto container = mItems[i];
		if (container && container->isContainer()) {
			Item* item = container->findStackable(pItemID);
			if (item) {
				return item;
			}
		}
	}
	return nullptr;
}

const bool Inventory::_clear(const u32 pSlot) {
	
	// Handle: Clearing cursor.
	if (SlotID::isCursor(pSlot)) {
		if (!_popCursor()) {
			mLog->error("Attempted to clear cursor but it is empty.");
			return false;
		}
		return true;
	}

	// Calculate primary index.
	const auto index = SlotID::getPrimarySlotIndex(pSlot);
	if (SlotID::isNone(pSlot)) {
		mLog->error("Failed to calculate primary index for slot ID: " + toString(pSlot));
		return false;
	}

	// Handle: Clearing primary slot.
	if (SlotID::isPrimarySlot(pSlot)) {
		// Check: There is an in this slot.
		if (!mItems[index]) {
			mLog->error("Attempted to clear empty primary slot: " + toString(pSlot));
			return false;
		}
		mItems[index] = nullptr;
		return true;
	}
	// Handle: Clearing container slot.
	else if (SlotID::isContainerSlot(pSlot)) {
		// Calculate container index.
		const auto containerIndex = getContainerSlotIndex(pSlot);
		if (SlotID::isNone(containerIndex)) {
			mLog->error("Failed to calculate container index for slot ID: " + toString(pSlot));
			return false;
		}

		// Check: There is a valid container Item in this slot.
		if (!mItems[index]) {
			mLog->error("Attempted to clear container slot but the primary slot is empty.");
			return false;
		}

		// Try clearing the slot.
		if (!mItems[index]->clearContents(containerIndex)) {
			mLog->error("Failed to clear container slot.");
			return false;
		}

		return true;
	}

	mLog->error("Failed to clear slot!");
	return false;
}

const bool Inventory::_putDown(const u32 pToSlot, const u32 pStacks) {

	// Check: Existing Item in pToSlot.
	Item* existing = get(pToSlot);
	if (existing) {
		// Potential stack merge.
		if (pStacks > 0) {
			// Check: Valid cursor Item.
			auto cursorItem = _peekCursor();
			if (!cursorItem || !cursorItem->isStackable()) {
				mLog->error("Invalid cursor Item in " + String(__FUNCTION__));
				return false;
			}

			// Check: Both Items are the same.
			if (cursorItem->getID() == existing->getID()) {
				EXPECTED_BOOL(_stackMergeCursor(pToSlot, pStacks));
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

	EXPECTED_BOOL(put(cursorItem, pToSlot, InventoryReason::Internal));

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

const bool Inventory::_stackMergeCursor(const u32 pToSlot, const u32 pStackSize) {

	Item* cursorItem = _peekCursor();
	EXPECTED_BOOL(cursorItem); // Failure = desync.
	EXPECTED_BOOL(cursorItem->isStackable());

	Item* existing = get(pToSlot);
	EXPECTED_BOOL(existing);
	EXPECTED_BOOL(cursorItem->getID() == existing->getID());

	const u32 preMergeStacks = cursorItem->getStacks() + existing->getStacks();

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

const bool Inventory::_pickUp(const u32 pFromSlot, const u32 pStackSize) {
	Item* pickUp = get(pFromSlot);
	EXPECTED_BOOL(pickUp);

	// Splitting the stack.
	if (pStackSize > 0) {
		EXPECTED_BOOL(pickUp->isStackable());
		const u32 preSplitStacks = pickUp->getStacks();
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

Item* Inventory::find(const u32 pItemID, const u32 pInstanceID) const {

	// Search Worn Slots
	for (u32 i = SlotID::CHARM; i <= SlotID::AMMO; i++) {
		if (mItems[i] && mItems[i]->getID() == pItemID && mItems[i]->getInstanceID() == pInstanceID)
			return mItems[i];
	}

	// Search Primary Main.
	for (u32 i = SlotID::MAIN_0; i <= SlotID::MAIN_7; i++) {
		if (mItems[i] && mItems[i]->getID() == pItemID && mItems[i]->getInstanceID() == pInstanceID) {
			return mItems[i];
		}
		// Search Primary Contents.
		else if (mItems[i] && mItems[i]->isContainer()) {
			for (u32 j = 0; j < SlotID::MAX_CONTENTS; j++) {
				if (mItems[j] && mItems[j]->getID() == pItemID && mItems[j]->getInstanceID() == pInstanceID) {
					return mItems[j];
				}
			}
		}
	}

	return nullptr;
}

const bool Inventory::removeRadiantCrystals(const u32 pCrystals) {
	EXPECTED_BOOL(mRadiantCrystals >= pCrystals);

	mRadiantCrystals -= pCrystals;
	return true;
}

const bool Inventory::removeEbonCrystals(const u32 pCrystals) {
	EXPECTED_BOOL(mEbonCrystals >= pCrystals);

	mEbonCrystals -= pCrystals;
	return true;
}

const bool Inventory::moveCurrency(const u32 pFromSlot, const u32 pToSlot, const u32 pFromType, const u32 pToType, const i32 pAmount) {
	EXPECTED_BOOL(Limits::General::moneySlotIDValid(pFromSlot));
	EXPECTED_BOOL(Limits::General::moneySlotIDValid(pToSlot));
	EXPECTED_BOOL(Limits::General::moneyTypeValid(pFromType));
	EXPECTED_BOOL(Limits::General::moneyTypeValid(pToType));
	EXPECTED_BOOL(pAmount > 0);

	const i32 currencyAtFrom = _getCurrency(pFromSlot, pFromType);
	EXPECTED_BOOL(currencyAtFrom >= pAmount);

	i32 addAmount = 0;
	i32 removeAmount = 0;

	// Trivial move (e.g. Gold to Gold)
	if (pFromType == pToType) {
		addAmount = pAmount;
		removeAmount = pAmount;
	}
	// (Conversion) Moving larger currency to smaller currency
	else if (pFromType > pToType) {
		u32 diff = pFromType - pToType;
		addAmount = static_cast<i32>(pAmount * std::pow(10, diff)); // Convert large to small
		removeAmount = pAmount;
	}
	// (Conversion) Moving smaller currency to larger currency
	else {
		u32 diff = pToType - pFromType;
		u32 denominator = static_cast<u32>(std::pow(10, diff));
		addAmount = pAmount / denominator; // Convert small to large
		// NOTE: The remainder of the above division is ignored, the Client will keep it on their cursor.
		removeAmount = pAmount - (pAmount % denominator);
	}

	EXPECTED_BOOL(removeCurrency(pFromSlot, pFromType, removeAmount));
	EXPECTED_BOOL(addCurrency(pToSlot, pToType, addAmount));

	return true;
}

const bool Inventory::addCurrency(const u32 pSlot, const i32 pPlatinum, const i32 pGold, const i32 pSilver, const i32 pCopper, const String& pReason) {
	StringStream ss;
	ss << "Add currency (" << pReason << ")  Copper: " << pCopper << " | Silver: " << pSilver << " | Gold: " << pGold << " | Platinum: " << pPlatinum;
	mLog->info(ss.str());

	if (!addCurrency(pSlot, CurrencyType::Platinum, pPlatinum)) return false;
	if (!addCurrency(pSlot, CurrencyType::Gold, pGold)) return false;
	if (!addCurrency(pSlot, CurrencyType::Silver, pSilver)) return false;
	if (!addCurrency(pSlot, CurrencyType::Copper, pCopper)) return false;
	return true;
}

const u64 Inventory::getTotalCurrency() const {
	u64 total = 0;
	for (auto i = 0; i < CurrencySlot::MAX; i++) {
		for (auto j = 0; j < CurrencyType::MAX; j++) {
			total += _getCurrency(i, j) * static_cast<u64>(std::pow(10, j));
		}
	}
	return total;
}

const bool Inventory::currencyValid() const {
	for (auto i = 0; i < CurrencySlot::MAX; i++) {
		for (auto j = 0; j < CurrencyType::MAX; j++) {
			if (_getCurrency(i, j) < 0)
				return false;
		}
	}

	return true;
}

const u32 Inventory::findEmptySlot(Item* pItem) const {
	if (!pItem) return SlotID::CURSOR; // If null gets passed in here, we have a big problem.

	// Check: Main
	for (u32 i = SlotID::MAIN_0; i <= SlotID::MAIN_7; i++) {
		auto item = get(i);
		if (!item) return i;
	}

	// If pItem is a container and there are no main slots free, it needs to go to the cursor.
	if (pItem->isContainer()) return SlotID::CURSOR;
	
	// Check: Main Contents
	for (u32 i = SlotID::MAIN_0; i <= SlotID::MAIN_7; i++) {
		auto item = get(i);
		if (item && item->isContainer() && item->getContainerSize() >= pItem->getSize()) {
			// Check if this container has an empty slot.
			const auto slotID = item->findEmptySlot();
			if (SlotID::isNone(slotID) == false)
				return slotID;
		}
	}

	return SlotID::CURSOR;
}

const u32 Inventory::findSlotFor(const bool pContainer, const u8 pItemSize) const {
	// Check: Main
	for (u32 i = SlotID::MAIN_0; i <= SlotID::MAIN_7; i++) {
		auto item = get(i);
		if (!item) return i;
	}

	if (pContainer) return SlotID::None;

	// Check: Main Contents
	for (u32 i = SlotID::MAIN_0; i <= SlotID::MAIN_7; i++) {
		auto item = get(i);
		if (item && item->isContainer() && item->getContainerSize() >= pItemSize) {
			const u32 slotID = item->findEmptySlot();
			if (SlotID::isNone(slotID) == false)
				return slotID;
		}
	}

	return SlotID::None;
}

const bool Inventory::addCurrency(const i32 pPlatinum, const i32 pGold, const i32 pSilver, const i32 pCopper, const String& pReason) {
	StringStream ss;
	ss << "Add currency (" << pReason << ")  Copper: " << pCopper << " | Silver: " << pSilver << " | Gold: " << pGold << " | Platinum: " << pPlatinum;
	mLog->info(ss.str());

	// Get current currency as copper.
	i64 currentCurrency = 0;
	EXPECTED_BOOL(Utility::convertCurrency(currentCurrency, getPersonalPlatinum(), getPersonalGold(), getPersonalSilver(), getPersonalCopper()));

	// Get amount being added as copper.
	i64 addedCurrency = 0;
	EXPECTED_BOOL(Utility::convertCurrency(addedCurrency, pPlatinum, pGold, pSilver, pCopper));

	i64 newCurrency = currentCurrency + addedCurrency;
	i32 newPlatinum = 0, newGold = 0, newSilver = 0, newCopper = 0;
	Utility::convertFromCopper<u64>(newCurrency, newPlatinum, newGold, newSilver, newCopper);

	// Set new currency amounts.
	setCurrency(CurrencySlot::Personal, CurrencyType::Platinum, newPlatinum);
	setCurrency(CurrencySlot::Personal, CurrencyType::Gold, newGold);
	setCurrency(CurrencySlot::Personal, CurrencyType::Silver, newSilver);
	setCurrency(CurrencySlot::Personal, CurrencyType::Copper, newCopper);

	return true;
}

const bool Inventory::addCurrency(const u32 pSlot, const u32 pType, const i32 pAmount) {
	if (!CurrencySlot::isValid(pSlot)) return false;
	if (!CurrencyType::isValid(pType)) return false;
	if (pAmount < 0) return false;

	mCurrency[pSlot][pType] += pAmount;
	return true;
}

const bool Inventory::removeCurrency(const i32 pPlatinum, const i32 pGold, const i32 pSilver, const i32 pCopper, const String& pReason) {
	StringStream ss;
	ss << "Remove currency (" << pReason << ")  Copper: " << pCopper << " | Silver: " << pSilver << " | Gold: " << pGold << " | Platinum: " << pPlatinum;
	mLog->info(ss.str());

	// Get current currency as copper.
	i64 currentCurrency = 0;
	EXPECTED_BOOL(Utility::convertCurrency(currentCurrency, getPersonalPlatinum(), getPersonalGold(), getPersonalSilver(), getPersonalCopper()));

	// Get amount being removed as copper.
	i64 removedCurrency = 0;
	EXPECTED_BOOL(Utility::convertCurrency(removedCurrency, pPlatinum, pGold, pSilver, pCopper));

	EXPECTED_BOOL(currentCurrency >= removedCurrency);

	i64 newCurrency = currentCurrency - removedCurrency;
	i32 newPlatinum = 0, newGold = 0, newSilver = 0, newCopper = 0;
	Utility::convertFromCopper<u64>(newCurrency, newPlatinum, newGold, newSilver, newCopper);

	// Set new currency amounts.
	setCurrency(CurrencySlot::Personal, CurrencyType::Platinum, newPlatinum);
	setCurrency(CurrencySlot::Personal, CurrencyType::Gold, newGold);
	setCurrency(CurrencySlot::Personal, CurrencyType::Silver, newSilver);
	setCurrency(CurrencySlot::Personal, CurrencyType::Copper, newCopper);

	return true;
}

const u64 Inventory::getTotalCursorCurrency() const {
	i64 value = 0;
	EXPECTED_VAR(Utility::convertCurrency(value, getCursorPlatinum(), getCursorGold(), getCursorSilver(), getCursorCopper()), 0);
	return value;
}

const u64 Inventory::getTotalPersonalCurrency() const {
	i64 value = 0;
	EXPECTED_VAR(Utility::convertCurrency(value, getPersonalPlatinum(), getPersonalGold(), getPersonalSilver(), getPersonalCopper()), 0);
	return value;
}

const u64 Inventory::getTotalBankCurrency() const {
	i64 value = 0;
	EXPECTED_VAR(Utility::convertCurrency(value, getBankPlatinum(), getBankGold(), getBankSilver(), getBankCopper()), 0);
	return value;
}

const u64 Inventory::getTotalTradeCurrency() const {
	i64 value = 0;
	EXPECTED_VAR(Utility::convertCurrency(value, getTradePlatinum(), getTradeGold(), getTradeSilver(), getTradeCopper()), 0);
	return value;
}

const u64 Inventory::getTotalSharedBankCurrency() const {
	i64 value = 0;
	// NOTE: The shared bank only supports storing platinum.
	EXPECTED_VAR(Utility::convertCurrency(value, getSharedBankPlatinum(), 0, 0, 0), 0);
	return value;
}

const u32 Inventory::getAlternateCurrencyQuantity(const u32 pCurrencyID) const {
	auto search = mAlternateCurrency.find(pCurrencyID);
	if (search == mAlternateCurrency.end()) return 0;
	return search->second;
}

void Inventory::addAlternateCurrency(const u32 pCurrencyID, const u32 pQuantity) {
	setAlternateCurrencyQuantity(pCurrencyID, getAlternateCurrencyQuantity(pCurrencyID) + pQuantity);
}

void Inventory::removeAlternateCurrency(const u32 pCurrencyID, const u32 pQuantity) {
	const u32 currentQuantity = getAlternateCurrencyQuantity(pCurrencyID);
	if (currentQuantity >= pQuantity)
		setAlternateCurrencyQuantity(pCurrencyID, currentQuantity - pQuantity);
	else {
		setAlternateCurrencyQuantity(pCurrencyID, 0);
	}
}

const bool save(Item* pItem, Data::Item* pDataItem) {
	return true;
}

const bool saveItem(Item* pItem, std::list<Data::Item>& pList) {
	EXPECTED_BOOL(pItem);

	// Copy data from Item to Data::Item.
	Data::Item item;
	item.mItemID = pItem->getID();
	item.mSlot = pItem->hasParent() ? pItem->getSubIndex() : pItem->getSlot();
	item.mStacks = pItem->getStacks();
	item.mCharges = pItem->getCharges();
	item.mAttuned = pItem->isAttuned();
	item.mLastCastTime = pItem->getLastCastTime();

	// Copy container contents.
	if (pItem->isContainer()) {
		std::list<Item*> contents;
		pItem->getContents(contents);
		
		for (auto i : contents)
			EXPECTED_BOOL(saveItem(i, item.mSubItems));
	}
	// Copy Item augmentations.
	else if (pItem->hasAugmentations()) {
		std::list<Item*> augmentations;
		pItem->getAugmentations(augmentations);

		for (auto i : augmentations)
			EXPECTED_BOOL(saveItem(i, item.mSubItems));
	}

	pList.push_back(item);
	return true;
}

const bool Inventory::updateForSave(Data::Inventory& pInventoryData) {
	// Worn / Primary Inventory.
	for (auto i : mItems) {
		if (i) {
			EXPECTED_BOOL(saveItem(i, pInventoryData.mItems));
		}
	}

	// Cursor
	for (auto i : mCursorItems)
		EXPECTED_BOOL(saveItem(i, pInventoryData.mItems));

	return true;
}

const u32 Inventory::getContainerSlotIndex(const u32 pSlotID) {
	if (SlotID::isMainContents(pSlotID)) return (pSlotID - SlotID::MainContentsBegin) % 10;
	if (SlotID::isBankContents(pSlotID)) return (pSlotID - SlotID::BankContentsBegin) % 10;
	if (SlotID::isSharedBankContents(pSlotID)) return (pSlotID - SlotID::SharedBankContentsBegin) % 10;

	return SlotID::None;
}

void Inventory::clearTradeItems() {
	static const auto tradeBegin = SlotID::getPrimarySlotIndex(SlotID::TradeBegin);
	static const auto tradeEnd = SlotID::getPrimarySlotIndex(SlotID::TradeEnd);
	for (auto i = tradeBegin; i < tradeEnd; i++)
		mItems[i] = nullptr;
}

void Inventory::clearTradeCurrency() {
	setCurrency(CurrencySlot::Trade, CurrencyType::Platinum, 0);
	setCurrency(CurrencySlot::Trade, CurrencyType::Gold, 0);
	setCurrency(CurrencySlot::Trade, CurrencyType::Silver, 0);
	setCurrency(CurrencySlot::Trade, CurrencyType::Copper, 0);
}

void Inventory::getTradeItems(std::list<Item*>& pItems) const {
	static const auto tradeBegin = SlotID::getPrimarySlotIndex(SlotID::TradeBegin);
	static const auto tradeEnd = SlotID::getPrimarySlotIndex(SlotID::TradeEnd);
	for (auto i = tradeBegin; i < tradeEnd; i++) {
		if (mItems[i])
			pItems.push_back(mItems[i]);
	}
}
