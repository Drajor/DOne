#include "CharacterFactory.h"
#include "IDataStore.h"
#include "Data.h"
#include "Character.h"
#include "Inventory.h"

CharacterFactory::~CharacterFactory() {
	mDataStore = nullptr;
	mLogFactory = nullptr;
	mItemFactory = nullptr;

	if (mLog) {
		delete mLog;
		mLog = nullptr;
	}
}

const bool CharacterFactory::initialise(IDataStore* pDataStore, ILogFactory* pLogFactory, ItemFactory* pItemFactory) {
	if (mInitialised) return false;
	if (!pDataStore) return false;
	if (!pLogFactory) return false;
	if (!pItemFactory) return false;

	mDataStore = pDataStore;
	mLogFactory = pLogFactory;
	mItemFactory = pItemFactory;
	mLog = mLogFactory->make();

	mLog->setContext("[CharacterFactory]");
	mLog->status("Initialising.");

	mLog->status("Finished initialising.");
	mInitialised = true;
	return true;
}

Character* CharacterFactory::make(const String& pCharacterName, Account* pAccount) {
	if (!pAccount) return false;

	// Check: Data::Character loaded.
	auto characterData = new Data::Character();
	if (!mDataStore->loadCharacter(pCharacterName, characterData)) {
		mLog->error("Failed to load data for Character: " + pCharacterName);
		delete characterData;
		return nullptr;
	}

	// Create and initialise Inventory.
	auto inventory = new Inventoryy();
	if (!inventory->initialise(&characterData->mInventory, mItemFactory, mLogFactory, pCharacterName)) {
		mLog->error("Failed to initialise Inventory for Character: " + pCharacterName);
		delete inventory;
		return nullptr;
	}

	// Create and initialise Character.
	auto character = new Character(characterData);
	if (!character->initialise(pAccount, inventory)) {
		mLog->error("Failed to initialise Character: " + pCharacterName);
		delete character;
		delete inventory;
		delete characterData;
		return nullptr;
	}

	return character;
}
