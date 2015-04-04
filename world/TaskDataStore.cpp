#include "TaskDataStore.h"
#include "LogSystem.h"
#include "IDataStore.h"
#include "Data.h"

TaskDataStore::~TaskDataStore() {
	mDataStore = nullptr;
	if (mLog) {
		mLog->status("Destroyed.");
		delete mLog;
		mLog = nullptr;
	}
}

bool TaskDataStore::initialise(IDataStore* pDataStore, ILogFactory* pLogFactory) {
	if (mInitialised) return false;
	if (!pDataStore) return false;
	if (!pLogFactory) return false;

	mDataStore = pDataStore;
	mLog = pLogFactory->make();

	mLog->setContext("[TaskDataStore]");
	mLog->status("Initialising.");

	for (auto& i : mData) i = new Data::Task();

	// Load data.
	u32 numTasksLoaded = 0;
	if (!mDataStore->loadTasks(mData, MaxTaskID, numTasksLoaded)){
		mLog->error("Failed to load data.");
		return false;
	}
	mLog->info("Loaded data for " + toString(numTasksLoaded) + " Tasks.");

	mLog->status("Finished initialising.");
	mInitialised = true;
	return true;

	return true;
}

Data::Task* TaskDataStore::getData(const u32 pTaskID) const {
	// Guard.
	if (pTaskID >= MaxTaskID) {
		mLog->error("Out of range Task ID" + toString(pTaskID) + " requested");
		return nullptr;
	}

	return mData[pTaskID];
}
