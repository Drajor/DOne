#include "TaskController.h"
#include "TaskDataStore.h"
#include "Data.h"
#include "LogSystem.h"
#include "Character.h"
#include "ZoneConnection.h"

//TaskController::TaskController(Character* pCharacter, TaskDataStore* pTaskDataStore) : mOwner(pCharacter), mTaskDataStore(pTaskDataStore) {
//	for (auto i : mCurrentTasks)
//		i = new CurrentTask();
//
//	auto a = new Data::Task();
//	a->mID = 1;
//	a->mTitle = "Example Task A";
//	a->mDescription = "Description of Example Task A";
//	mAvailableTasks.push_back(a);
//
//	auto b = new Data::Task();
//	b->mID = 2;
//	b->mTitle = "Example Task B";
//	b->mDescription = "Description of Example Task B";
//	mAvailableTasks.push_back(b);
//}

TaskController::~TaskController() {
	for (auto i : mQuestTasks)
		delete i;
	mQuestTasks.clear();

	if (mLog) {
		delete mLog;
		mLog = nullptr;
	}
}

bool TaskController::initialise(Character* pCharacter, TaskDataStore* pTaskDataStore, ILogFactory* pLogFactory) {
	if (!pCharacter) return false;
	if (!pTaskDataStore) return false;
	if (!pLogFactory) return false;

	mOwner = pCharacter;
	mTaskDataStore = pTaskDataStore;
	mLog = pLogFactory->make();
	mLog->setContext("[TaskController (" + mOwner->getName() + ")]");

	for (auto i : mQuestTasks)
		i = new CurrentTask();

	return true;
}

bool TaskController::onLoad(CompletedTasksData& pCompletedTasksData) {
	
	for (auto i : pCompletedTasksData) {
		auto taskData = mTaskDataStore->getData(i.mTaskID);
		if (!taskData) {
			mLog->error("Could not find task.");
		}

		mCompletedTasks.push_back(CompletedTask(taskData, i.mCompleted));
	}

	return true;
}

void TaskController::onSave(CompletedTasksData& pCompletedTasksData) const {
	pCompletedTasksData.clear();

	for (auto i : mCompletedTasks) {
		Data::CompletedTask completedTask;
		completedTask.mTaskID = i.mTaskData->mID;
		completedTask.mCompleted = i.mTimeCompleted;
		pCompletedTasksData.push_back(completedTask);
	}
}

void TaskController::onHistoryRequest(const u32 pIndex) {
	// Check: Out of range.
	if (pIndex >= mCompletedTasks.size()) {
		mLog->error("Task index out of range: " + toString(pIndex));
		return;
	}

	// Create history packet.
	auto task = mCompletedTasks[pIndex];
	auto packet = Payload::updateTaskObjectiveHistory(pIndex, task.mTaskData);

	// Check: Packet is valid.
	if (!packet) {
		mLog->error("");
		return;
	}

	// Send history packet.
	mOwner->getConnection()->sendPacket(packet);
	delete packet;
}

bool TaskController::add(const u32 pTaskID, const bool pShowQuestJournal) {
	mLog->info("Adding Task ID: " + toString(pTaskID));

	// Check: Task exists.
	auto task = mTaskDataStore->getData(pTaskID);
	if (!task) {
		mLog->error("Task does not exist");
		return false;
	}

	const auto hasSolo = hasSoloTask();
	const auto hasShared = hasSharedTask();

	// Check: Character already has this Task.
	if (hasTask(pTaskID)) {
		mLog->error("Already has this Task");
		return false;
	}

	// Check: Already has a solo Task.
	if (task->mType == TaskType::Task && hasSolo) {
		mLog->error("Already has a solo Task");
		return false;
	}

	// Check: Already has a shared Task.
	if (task->mType == TaskType::Shared && hasShared) {
		mLog->error("Already has a shared Task");
		return false;
	}

	// Check: Already completed a non-repeatable Task.
	if (!task->mRepeatable && hasCompletedTask(pTaskID)) {
		mLog->error("Already completed non-repeatable Task");
		return false;
	}

	// TODO: Number of tasks. level etc.
	auto newTask = makeQuestTask(task);

	// Send description.
	auto packet = Payload::updateTask(newTask->mIndex, newTask);
	mOwner->getConnection()->sendPacket(packet);
	delete packet;

	// Send objectives.
	for (auto i : newTask->mObjectives) {
		packet = Payload::updateTaskObjective(newTask, i);
		mOwner->getConnection()->sendPacket(packet);
		delete packet;
	}

	return true;
}

bool TaskController::remove(const u32 pTaskID) {
	mLog->info("Removing Task ID: " + toString(pTaskID));

	// Find Task index/type and remove.
	for (auto& i : mQuestTasks) {
		if (i->mTaskData->mID == pTaskID)
			return remove(i->mIndex, i->mTaskData->mType);
	}

	return false;
}

bool TaskController::remove(const u32 pIndex, const u32 pTaskType) {
	mLog->info("Removing Task index: " + toString(pIndex) + " type: " + toString(pTaskType));

	auto found = false;
	if (pTaskType == TaskType::Quest) {
		for (auto i = mQuestTasks.begin(); i != mQuestTasks.end(); i++) {
			if ((*i)->mIndex == pIndex) {
				auto task = *i;
				mQuestTasks.erase(i);
				delete task;
				found = true;
				break;
			}
		}
	}

	if (!found) {
		mLog->error("Could not find task.");
		return false;
	}

	auto packet = Payload::Zone::RemoveTask::construct(pIndex, pTaskType);
	mOwner->getConnection()->sendPacket(packet);
	delete packet;

	return true;
}

bool TaskController::hasTask(const u32 pTaskID) const {
	for (auto i : mQuestTasks) {
		if (i->mTaskData->mID == pTaskID)
			return true;
	}

	return false;
}

CurrentTask* TaskController::makeQuestTask(Data::Task* pTask) {
	auto task = new CurrentTask();
	task->mIndex = mQuestTasks.size();
	task->mTaskData = pTask;
	mQuestTasks.push_back(task);

	for (auto i : pTask->mObjectives) {
		auto objective = new CurrentTaskObjective();
		objective->mObjectiveData = i;
		task->mObjectives.push_back(objective);
	}

	return task;
}

void TaskController::onEnterZone() {
	auto packet = Payload::updateTaskHistory(mCompletedTasks);
	mOwner->getConnection()->sendPacket(packet);
	delete packet;
}

