#include "CharacterFactory.h"
#include "IDataStore.h"
#include "Data.h"
#include "Character.h"
#include "Inventory.h"
#include "ExperienceController.h"
#include "TaskController.h"

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
		delete characterData;
		return nullptr;
	}

	// Create and initialise Experience::Controller.
	auto experienceController = new Experience::Controller();
	if (!experienceController->onLoad(&characterData->mExperience)) {
		mLog->error("Failed to initialise Experience::Controller for Character: " + pCharacterName);
		delete inventory;
		delete characterData;
		delete experienceController;
		return nullptr;
	}

	auto character = new Character(characterData);

	// Create and initialise TaskController.
	auto taskController = new TaskController(character);
	taskController->onLoad();

	// Initialise Character.
	if (!character->initialise(pAccount, inventory, experienceController, taskController)) {
		mLog->error("Failed to initialise Character: " + pCharacterName);
		delete inventory;
		delete characterData;
		delete experienceController;
		delete character;
		return nullptr;
	}

	// Check: Is this the first time the Character has logged in.
	if (character->isNew()) {
		//mLog->info("")
	}

	return character;
}
