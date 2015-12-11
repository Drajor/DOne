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

const bool CharacterFactory::initialise(IDataStore* pDataStore, ILogFactory* pLogFactory, ItemFactory* pItemFactory, TaskDataStore* pTaskDataStore) {
	if (mInitialised) return false;
	if (!pDataStore) return false;
	if (!pLogFactory) return false;
	if (!pItemFactory) return false;
	if (!pTaskDataStore) return false;

	mDataStore = pDataStore;
	mTaskDataStore = pTaskDataStore;
	mLogFactory = pLogFactory;
	mItemFactory = pItemFactory;
	mLog = mLogFactory->make();

	mLog->setContext("[CharacterFactory]");
	mLog->status("Initialising.");

	mLog->status("Finished initialising.");
	mInitialised = true;
	return true;
}

SharedPtr<Character> CharacterFactory::make(const String& pCharacterName, SharedPtr<Account> pAccount) {
	if (!pAccount) return false;

	auto character = std::make_shared<Character>();

	// Associate Account.
	character->setAccount(pAccount);

	// Associate Experience::Controller.
	auto expController = new Experience::Controller();
	character->setExperienceController(expController);

	if (!mDataStore->characterLoad(pCharacterName, character.get())) {
		mLog->error("Failed to load Character: " + pCharacterName);
		return nullptr;
	}

	//// Check: Data::Character loaded.
	////auto characterData = new Data::Character();
	//if (!mDataStore->characterLoad(pCharacterName, characterData)) {
	//	mLog->error("Failed to load data for Character: " + pCharacterName);
	//	delete characterData;
	//	return nullptr;
	//}

	// Create and initialise Inventory.
	auto inventory = std::make_shared<Inventory>();
	character->setInventory(inventory);
	// Load Inventory.

	auto taskController = std::make_shared<TaskController>();
	character->setTaskController(taskController);
	// Load Tasks.

	// Load Buffs.

	// Load Skills

	// Load Languages

	// Load Spell Book

	// Load Spell Bar

	// Load Disciplines

	// Load AA

	//if (!inventory->initialise(&characterData->mInventory, mItemFactory, mLogFactory, pCharacterName)) {
	//	mLog->error("Failed to initialise Inventory for Character: " + pCharacterName);
	//	return nullptr;
	//}

	// Create and initialise Experience::Controller.
	//auto experienceController = new Experience::Controller();
	//if (!experienceController->onLoad(&characterData->mExperience)) {
	//	mLog->error("Failed to initialise Experience::Controller for Character: " + pCharacterName);
	//	delete inventory;
	//	delete characterData;
	//	delete experienceController;
	//	return nullptr;
	//}

	//auto character = new Character(characterData);

	//// Create and initialise TaskController.
	//auto taskController = new TaskController();
	//if (!taskController->initialise(character, mTaskDataStore, mLogFactory)) {
	//	// TODO: Delete
	//	return false;
	//}
	//if (!taskController->onLoad(characterData->mCompletedTasks)) {
	//	// TODO: Delete.
	//	return false;
	//}

	//// Initialise Character.
	//if (!character->initialise(pAccount, inventory, experienceController, taskController)) {
	//	mLog->error("Failed to initialise Character: " + pCharacterName);
	//	delete inventory;
	//	delete characterData;
	//	delete experienceController;
	//	delete character;
	//	return nullptr;
	//}

	// Check: Is this the first time the Character has logged in.
	if (character->isNew()) {
		//mLog->info("")
	}

	return character;
}
