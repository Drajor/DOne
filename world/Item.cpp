#include "Item.h"
#include "ItemData.h"
#include "Payload.h"
#include <iostream>

Item::Item(ItemData* pItemData) : mItemData(pItemData) {
	for (auto& i : mAugments) i = nullptr;
	for (auto& i : mContents) i = nullptr;
}

Item* Item::copy() const {
	// TODO: Needs a new serial
	return new Item(mItemData);
}

Item::~Item() {
	for (auto i : mAugments) {
		if (i) delete i;
	}
	for (auto i : mContents) {
		if (i) delete i;
	}
}

Item* Item::getAugment(const uint8 pSlot) const {
	return mAugments[pSlot];
}

Item* Item::getContents(const uint8 pSlot) const {
	return mContents[pSlot];
}

void Item::getContents(std::list<Item*>& pItems) const {
	for (auto i : mContents) {
		if (i) pItems.push_back(i);
	}
}

const uint32 Item::getDataSize(const uint32 pCopyType) const {
	uint32 subItemSize = 0;

	// Add augments.
	for (auto i : mAugments)
		if (i) subItemSize += i->getDataSize(pCopyType);

	// Add contents (container)
	for (auto i : mContents)
		if (i) subItemSize += i->getDataSize(pCopyType);

	return _getDataSize(pCopyType) + subItemSize;
}

const uint32 Item::_getDataSize(const uint32 pCopyType) const {
	uint32 result = sizeof(ItemData);

	// NOTE: When an Item with a parent is sent with ItemPacketTrade, the sub-index is not sent.
	if (hasParent() && pCopyType != Payload::ItemPacketTrade) {
		// Sub-Items need 4 extra bytes to store sub-index.
		result += sizeof(uint32);
	}

	if (!isEvolvingItem()) {
		result -= sizeof(ItemData::EvolvingItem);
	}

	// Remove the maximum size of variable sized attributes.
	result -= sizeof(mItemData->mOrnamentationIDFile);
	result -= sizeof(mItemData->mName);
	result -= sizeof(mItemData->mLore);
	result -= sizeof(mItemData->mIDFile);
	result -= sizeof(mItemData->mCharmFile);
	result -= sizeof(mItemData->mFileName);
	result -= sizeof(mItemData->mClickName);
	result -= sizeof(mItemData->mProcName);
	result -= sizeof(mItemData->mWornName);
	result -= sizeof(mItemData->mFocusName);
	result -= sizeof(mItemData->mScrollName);
	result -= sizeof(mItemData->mBardName);

	// Add variable strings
	result += getOrnamentationIDFile().length() + 1;
	result += strlen(mItemData->mName) + 1;
	result += strlen(mItemData->mLore) + 1;
	result += strlen(mItemData->mIDFile) + 1;
	result += strlen(mItemData->mCharmFile) + 1;
	result += strlen(mItemData->mFileName) + 1;
	result += strlen(mItemData->mClickName) + 1;
	result += strlen(mItemData->mProcName) + 1;
	result += strlen(mItemData->mWornName) + 1;
	result += strlen(mItemData->mFocusName) + 1;
	result += strlen(mItemData->mScrollName) + 1;
	result += strlen(mItemData->mBardName) + 1;

	return result;
}

const bool Item::copyData(Utility::DynamicStructure& pStructure, const uint32 pCopyType) {
	// Check: This Item is either an augment or within a bag.
	// NOTE: When an Item with a parent is sent with ItemPacketTrade, the sub-index is not sent.
	if (hasParent() && pCopyType != Payload::ItemPacketTrade) {
		EXPECTED_BOOL(hasValidSubIndex());
		pStructure.write<uint32>(getSubIndex());
	}

	// NOTE: When an Item has augments, those augments are sent with the same slot ID as the parent item.

	// Write Item Header.
	pStructure.write(getStacks());
	pStructure.write(mItemData->__Unknown0);
	pStructure.write(getSlot());
	pStructure.write(getShopPrice());
	pStructure.write(getShopQuantity());
	pStructure.write(mItemData->__Unknown1);
	pStructure.write(getInstanceID());
	pStructure.write(mItemData->__Unknown2);
	pStructure.write(getLastCastTime());
	pStructure.write(getCharges());
	pStructure.write(isAttuned() ? 1 : 0);
	pStructure.write(getPower());
	pStructure.write(mItemData->__Unknown3);
	pStructure.write(mItemData->__Unknown4);
	pStructure.write(mItemData->mIsEvolvingItem);

	// Optional (Evolving Item)
	if (isEvolvingItem()) {
		pStructure.writeChunk((void*)&(mItemData->mEvolvingItem.__Unknown0), sizeof(mItemData->mEvolvingItem.__Unknown0));
		pStructure.write<int32>(getEvolvingLevel());
		pStructure.write<double>(getEvolvingProgress());
		pStructure.write<uint8>(getEvolvingActive());
		pStructure.write<int32>(mItemData->mEvolvingItem.mMaxLevel);
		pStructure.writeChunk((void*)&(mItemData->mEvolvingItem.__Unknown1), sizeof(mItemData->mEvolvingItem.__Unknown1));
	}

	pStructure.writeString(getOrnamentationIDFile());
	pStructure.write(getOrnamentationIcon());

	pStructure.write<uint8>(mItemData->__Unknown5);
	pStructure.write<uint8>(mItemData->__Unknown6);

	pStructure.write<uint8>(mItemData->mCopied);
	pStructure.write<uint8>(mItemData->mItemClass);
	
	// HEADER - END

	// Variable.
	pStructure.writeString(String(mItemData->mName));
	pStructure.writeString(String(mItemData->mLore));
	pStructure.writeString(String(mItemData->mIDFile));

	// Chunk One.
	std::size_t chunk1 = (unsigned int)&(mItemData->mCharmFile) - (unsigned int)&(mItemData->mID);
	pStructure.writeChunk((void*)&(mItemData->mID), chunk1);

	// Variable.
	pStructure.writeString(String(mItemData->mCharmFile));
	
	// Chunk Two.
	std::size_t chunk2 = (unsigned int)&(mItemData->mFileName) - (unsigned int)&(mItemData->mAugType);
	pStructure.writeChunk((void*)&(mItemData->mAugType), chunk2);
	
	// Variable.
	pStructure.writeString(String(mItemData->mFileName));

	// Chunk Three.
	std::size_t chunk3 = (unsigned int)&(mItemData->mClickEffect) - (unsigned int)&(mItemData->mLoreGroup);
	pStructure.writeChunk((void*)&(mItemData->mLoreGroup), chunk3);
	
	// Click Effect.
	pStructure.write<ItemData::ClickEffect>(mItemData->mClickEffect);
	pStructure.writeString(String(mItemData->mClickName));
	pStructure.write<int32>(mItemData->mClickUnknown);
	
	// Proc Effect.
	pStructure.write<ItemData::ProcEffectStruct>(mItemData->mProcEffect);
	pStructure.writeString(String(mItemData->mProcName));
	pStructure.write<int32>(mItemData->mProcUnknown);

	// Worn Effect.
	pStructure.write<ItemData::Effect>(mItemData->mWornEffect);
	pStructure.writeString(String(mItemData->mWornName));
	pStructure.write<int32>(mItemData->mWornUnknown);

	// Focus Effect.
	pStructure.write<ItemData::Effect>(mItemData->mFocusEffect);
	pStructure.writeString(String(mItemData->mFocusName));
	pStructure.write<int32>(mItemData->mWornUnknown);

	// Scroll Effect.
	pStructure.write<ItemData::Effect>(mItemData->mScrollEffect);
	pStructure.writeString(String(mItemData->mScrollName));
	pStructure.write<int32>(mItemData->mScrollUnknown);

	// Bard Effect.
	pStructure.write<ItemData::Effect>(mItemData->mBardEffect);
	pStructure.writeString(String(mItemData->mBardName));
	pStructure.write<int32>(mItemData->mBardUnknown);

	// Chunk Four.
	std::size_t chunk4 = (unsigned int)&(mItemData->mNumSubItems) - (unsigned int)&(mItemData->mScriptFileID);
	pStructure.writeChunk((void*)&(mItemData->mScriptFileID), chunk4);

	// Child Items.
	pStructure.write<uint32>(getSubItems());

	// Write augments
	for (auto i : mAugments)
		if (i) EXPECTED_BOOL(i->copyData(pStructure, pCopyType));		

	// Write contents
	for (auto i : mContents)
		if (i) EXPECTED_BOOL(i->copyData(pStructure, pCopyType));

	return true;
}

const unsigned char* Item::copyData(uint32& pSize, const uint32 pCopyType) {
	unsigned char * data = nullptr;
	pSize += getDataSize(pCopyType);

	pSize += sizeof(uint32); // Copy Type
	data = new unsigned char[pSize];

	Utility::DynamicStructure ds(data, pSize);
	ds.write<uint32>(pCopyType);

	// Copy Item data.
	copyData(ds, pCopyType);
	
	// Check: The amount of data written matches what was calculated.
	if (ds.check() == false) {
		Log::error("[Item] Bad Write: Written: " + std::to_string(ds.getBytesWritten()) + " Size: " + std::to_string(ds.getSize()));
	}

	return data;
}

uint32 Item::getSubItems() const {
	uint32 count = 0;

	for (auto i : mAugments) { if (i) count++; }
	for (auto i : mContents) { if (i) count++; }

	return count;
}

Item* Item::findFirst(const uint8 pItemType) const {
	for (auto i : mContents) {
		if (i && i->getItemType() == pItemType)
			return i;
	}

	return nullptr;
}

Item* Item::findStackable(const uint32 pItemID) const {
	for (auto i : mContents) {
		if (i && i->getID() == pItemID && i->getStacks() < i->getMaxStacks())
			return i;
	}

	return nullptr;
}

const bool Item::insertAugment(Item* pAugment) {
	EXPECTED_BOOL(pAugment);
	EXPECTED_BOOL(pAugment->isAugmentation());
	EXPECTED_BOOL(augmentAllowed(pAugment));

	// Find an empty augmentation slot with the correct type.
	int slotID = -1;
	for (int i = 0; i < 5; i++) {
		// Found a matching slot type.
		if (mItemData->mAugmentationSlots[i].mType == pAugment->getAugmentationType()) {
			// Found an empty, matching slot.
			if (mAugments[i] == nullptr) {
				slotID = i;
				break;
			}
		}
	}

	// No valid slot found.
	if (slotID < 0) return false;

	setAugmentation(slotID, pAugment);
	pAugment->setSlot(getSlot());
	pAugment->setParent(this);
	pAugment->setSubIndex(slotID);

	return true;
}

const bool Item::augmentAllowed(Item* pAugment) {
	EXPECTED_BOOL(pAugment);
	EXPECTED_BOOL(pAugment->isAugmentation());

	// Check: Restrictions.
	if (pAugment->getAugmentationRestriction()) {
		switch (pAugment->getAugmentationRestriction()) {
		case AugmentationRestriction::Armor:
			if (!isArmor()) return false;
			break;
		case AugmentationRestriction::Weapons:
			if (!isWeapon()) return false;
			break;
		case AugmentationRestriction::OneHandWeapons:
			if (!isOneHandWeapon()) return false;
			break;
		case AugmentationRestriction::TwoHandWeapons:
			if (!isTwoHandWeapon()) return false;
			break;
		case AugmentationRestriction::OneHandSlash:
			if (!isOneHandSlash()) return false;
			break;
		case AugmentationRestriction::OneHandBlunt:
			if (!isOneHandBlunt()) return false;
			break;
		case AugmentationRestriction::Piercing:
			if (!isOneHandPierce()) return false;
			break;
		case AugmentationRestriction::TwoHandSlash:
			if (!isTwoHandSlash()) return false;
			break;
		case AugmentationRestriction::TwoHandBlunt:
			if (!isTwoHandBlunt()) return false;
			break;
		case AugmentationRestriction::TwoHandPierce:
			if (!isTwoHandPierce()) return false;
			break;
		case AugmentationRestriction::Bow:
			if (!isBow()) return false;
			break;
		case AugmentationRestriction::Shield:
			if (!isShield()) return false;
			break;
		case AugmentationRestriction::HandtoHand:
			if (!isHandToHand()) return false;
			break;
		default:
			return false;
		}
	}

	// Check: Equipment Slot



	return true;
}

const bool Item::clearContents(const uint32 pSubIndex) {
	EXPECTED_BOOL(isContainer());
	EXPECTED_BOOL(SlotID::subIndexValid(pSubIndex));
	EXPECTED_BOOL(getContainerSlots() > pSubIndex);

	// Clean up where there is Item in the slot being cleared.
	Item* existingItem = mContents[pSubIndex];
	if (existingItem) {
		existingItem->clearParent();
		existingItem->clearSubIndex();
	}

	mContents[pSubIndex] = nullptr;

	return true;
}

const bool Item::setContents(Item* pItem, const uint32 pSubIndex) {
	EXPECTED_BOOL(pItem);
	EXPECTED_BOOL(SlotID::subIndexValid(pSubIndex));
	EXPECTED_BOOL(mContents[pSubIndex] == nullptr); // Prevent overriding Item pointer. Failure = bug.
	EXPECTED_BOOL(isContainer());
	EXPECTED_BOOL(getContainerSlots() > pSubIndex);
	EXPECTED_BOOL(getContainerSize() >= pItem->getSize());
	EXPECTED_BOOL(pItem->setParent(this));
	EXPECTED_BOOL(pItem->setSubIndex(pSubIndex));

	mContents[pSubIndex] = pItem;
	// Update the slot of the Item being set.
	pItem->setSlot(SlotID::getChildSlot(getSlot(), pSubIndex));

	return true;
}

void Item::updateContentsSlots() {
	for (auto i = 0; i < SlotID::MAX_CONTENTS; i++) {
		if (mContents[i])
			mContents[i]->setSlot(SlotID::getChildSlot(getSlot(), i));
	}
}

const bool Item::setParent(Item* pParent) {
	EXPECTED_BOOL(pParent);
	EXPECTED_BOOL(getParent() == nullptr);

	mParent = pParent;

	return true;
}

const bool Item::setSubIndex(const uint32 pSubIndex) {
	EXPECTED_BOOL(SlotID::subIndexValid(pSubIndex));
	EXPECTED_BOOL(getSubIndex() == -1);

	mSubIndex = pSubIndex;

	return true;
}

const bool Item::isTradeable() const {
	if (isNoDrop()) return false;
	if (isAttuned()) return false;
	// TODO: Test Copied
	if (hasOrnamentationIcon()) return false;
	if (hasOrnamentationIDFile()) return false;

	for (auto i : mAugments) {
		if (i && !i->isTradeable()) return false;
	}

	for (auto i : mContents) {
		if (i && !i->isTradeable()) return false;
	}

	return true;
}

const bool Item::isSellable() const {
	if (getPrice() == 0) return false;
	if (isNoDrop()) return false;
	if (isAttuned()) return false;
	if (isTemporary()) return false; // Tested.
	if (isCopied()) return false; // Tested.
	if (hasOrnamentationIcon()) return false; // Tested.
	if (hasOrnamentationIDFile()) return false; // Tested.
	// Tested: Artifact is not blocked by UF.
	// Tested: Summoned is not blocked by UF.
	// Tested: Evolving is not blocked by UF.
	// Tested: Heirloom is not blocked by UF.
	// Tested: No Transfer is not blocked by UF.

	// TODO: FVNoDrop

	// Containers with Items inside can not be sold.
	if (isContainer() && !isEmpty()) return false;

	// Items with augmentations can not be sold. // TODO: Double check this is true.
	if (hasAugmentations()) return false;

	return true;
}

const bool Item::addStacks(const uint32 pStacks) {
	EXPECTED_BOOL(isStackable());
	EXPECTED_BOOL(getStacks() + pStacks <= getMaxStacks()); // Over stacking. Bug!

	mStacks += pStacks;
	return true;
}

const bool Item::removeStacks(const uint32 pStacks) {
	EXPECTED_BOOL(isStackable());
	EXPECTED_BOOL(getStacks() > pStacks);

	mStacks -= pStacks;
	return true;
}

const bool Item::compare(Item * pItem, String& pText) {
	EXPECTED_BOOL(pItem);

	Utility::PopupHelper helper;

	// Strength
	int strength = getStrength() - pItem->getStrength();
	if (strength > 0) {
		helper.startColour(Utility::PopupHelper::Colour::RED);
		helper.writeBr("-" + std::to_string(strength) + " Strength" );
		helper.endColour();
	}
	else if (strength < 0) {
		helper.startColour(Utility::PopupHelper::Colour::GREEN);
		helper.writeBr("+" + std::to_string(strength) + " Strength");
		helper.endColour();
	}

	// Agility
	int agility = getAgility() - pItem->getAgility();
	if (agility > 0) {
		helper.startColour(Utility::PopupHelper::Colour::RED);
		helper.writeBr("-" + std::to_string(agility) + " Agility");
		helper.endColour();
	}
	else if (agility < 0) {
		helper.startColour(Utility::PopupHelper::Colour::GREEN);
		helper.writeBr("+" + std::to_string(agility) + " Agility");
		helper.endColour();
	}

	// Charisma
	int charisma = getCharisma() - pItem->getCharisma();
	if (charisma > 0) {
		helper.startColour(Utility::PopupHelper::Colour::RED);
		helper.writeBr("-" + std::to_string(charisma) + " Charisma");
		helper.endColour();
	}
	else if (charisma < 0) {
		helper.startColour(Utility::PopupHelper::Colour::GREEN);
		helper.writeBr("+" + std::to_string(charisma) + " Charisma");
		helper.endColour();
	}

	pText = helper.getText();
	return true;
}

const bool Item::isCombineContainer() const {
	switch (getContainerType()) {
	case ContainerType::Quest:
		return true;
		// TODO: Add the rest.
	default:
		break;
	}

	return false;
}

const bool Item::isEmpty() const {
	for (auto i : mContents) {
		if (i) return false;
	}

	return true;
}

const bool Item::forEachContents(std::function<const bool(Item*)> pFunction) const {
	for (auto i : mContents) {
		if (i && !pFunction(i))
			return false;
	}

	return true;
}

const uint32 Item::getSellPrice(const uint32 pStacks, const float pSellRate) const {
	return std::ceil(pStacks * getPrice() * pSellRate);
}

const bool Item::hasAugmentations() const {
	for (auto i : mAugments) {
		if (i) return false;
	}

	return false;
}

String Item::getLink() const {
	std::stringstream ss;
	ss << "\x12";
	ss << std::setw(1) << std::hex << 0; // Unknown ?
	ss << std::setw(5) << std::hex << std::setfill('0') << getID();

	for (auto i : mAugments) {
		if (i) {
			ss << std::setw(5) << std::hex << std::setfill('0') << i->getID();
		}
		else {
			ss << std::setw(5) << std::hex << std::setfill('0') << 0;
		}
	}

	ss << std::setw(1) << std::hex << std::setfill('0') << isEvolvingItem() ? 1 : 0;
	ss << std::setw(4) << std::hex << std::setfill('0') << getLoreGroup();
	ss << std::setw(1) << std::hex << std::setfill('0') << getEvolvingLevel();
	ss << std::setw(5) << std::hex << std::setfill('0') << 0; // Unknown ?
	ss << std::setw(8) << std::hex << std::setfill('0') << 0; // Hash
	ss << getName();
	ss << "\x12";

	return ss.str();
}

const uint32 Item::findEmptySlot() {
	EXPECTED_VAR(isContainer(), SlotID::None);

	for (int i = 0; i < SlotID::MAX_CONTENTS; i++) {
		if (!mContents[i])
			return SlotID::getChildSlot(getSlot(), i);
	}

	return SlotID::None;
}
