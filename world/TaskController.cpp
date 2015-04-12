#include "TaskController.h"
#include "TaskDataStore.h"
#include "Data.h"
#include "Task.h"
#include "LogSystem.h"
#include "Character.h"
#include "ZoneConnection.h"

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
		mLog->error("Failed to create Task History packet.");
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
	auto packet = Payload::updateTask(newTask);
	mOwner->getConnection()->sendPacket(packet);
	delete packet;

	// Send objectives.
	for (auto i : newTask->getStages()) {
		for (auto j : i->getObjectives()) {
			packet = Payload::updateTaskObjective(j);
			mOwner->getConnection()->sendPacket(packet);
			delete packet;
		}
	}

	return true;
}

bool TaskController::remove(const u32 pTaskID) {
	mLog->info("Removing Task ID: " + toString(pTaskID));

	// Find Task index/type and remove.
	for (auto& i : mQuestTasks) {
		if (i->getID() == pTaskID)
			return remove(i->getIndex(), i->getType());
	}

	return false;
}

bool TaskController::remove(const u32 pIndex, const u32 pTaskType) {
	mLog->info("Removing Task index: " + toString(pIndex) + " type: " + toString(pTaskType));

	auto found = false;
	if (pTaskType == TaskType::Quest) {
		for (auto i = mQuestTasks.begin(); i != mQuestTasks.end(); i++) {
			if ((*i)->getIndex() == pIndex) {
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

	// Send removal.
	auto packet = Payload::Zone::RemoveTask::construct(pIndex, pTaskType);
	mOwner->getConnection()->sendPacket(packet);
	delete packet;

	return true;
}

bool TaskController::hasTask(const u32 pTaskID) const {
	for (auto i : mQuestTasks) {
		if (i->getID() == pTaskID)
			return true;
	}

	return false;
}

TaskSystem::Task* TaskController::makeQuestTask(Data::Task* pTask) {
	auto task = new TaskSystem::Task(pTask);
	task->setIndex(mQuestTasks.size());
	mQuestTasks.push_back(task);

	//for (auto i : pTask->mObjectives) {
	//	auto objective = new CurrentTaskObjective();
	//	objective->mObjectiveData = i;
	//	task->mObjectives.push_back(objective);
	//}

	return task;
}

void TaskController::onEnterZone() {
	auto packet = Payload::updateTaskHistory(mCompletedTasks);
	mOwner->getConnection()->sendPacket(packet);
	delete packet;
}

bool TaskController::onHail(const u32 pNPCTypeID, const u16 pZoneID) {

	//// Iterate active tasks.
	//for (auto& i : mQuestTasks) {
	//	if (!i->isActive()) continue; // Task has expired and the user has not yet removed it from their list.
	//	if (i->isFree()) continue;
	//	if (i->isComplete()) continue; // Task was completed earlier this update.

	//	// Iterate active objectives.
 //		for (auto& j : i->mObjectives) {
	//		if (j->mHidden) continue; // Objective is still hidden, keep looking.
	//		if (j->mComplete) continue; // Objective is complete, keep looking.

	//		// Check: Objective type matches.
	//		if (j->mObjectiveData->mType == ObjectiveType::Hail) {

	//			const bool zoneMatch = j->mObjectiveData->mZoneID != 0 && j->mObjectiveData->mZoneID == pZoneID;
	//			const bool npcMatch = j->mObjectiveData->mNPCTypeID == pNPCTypeID;

	//			if (zoneMatch && npcMatch) {
	//				//j->mValue++;
	//				incrementObjective(j);
	//			}

	//			//// Check: Objective Zone ID matches.
	//			//if (j->mObjectiveData->mZoneID != 0 && j->mObjectiveData->mZoneID == pZoneID) {
	//			//	// Check: Objective NPC Type ID matches.
	//			//	if (j->mObjectiveData->mNPCTypeID == pNPCTypeID) {
	//			//		//incrementObjective(j);

	//			//		j->mValue++;
	//			//	}
	//			//}
	//		}
	//	}
	//}

	return true;
}

bool TaskController::incrementObjective(const u32 pTaskIndex, const u32 pStageIndex, const u32 pObjectiveIndex) {

	// Find Task by index.
	auto task = getTask(pTaskIndex);
	if (!task) return false;

	// Find Stage by index.
	auto stage = task->getStage(pStageIndex);
	if (!stage) return false;

	// Find Objective by index.
	auto objective = stage->getObjective(pObjectiveIndex);
	if (!objective) return false;

	// Increment Objective.
	return incrementObjective(objective);
}

bool TaskController::incrementObjective(TaskSystem::Objective* pObjective) {
	if (!pObjective) return false;

	// Check: Objective is already complete.
	if (pObjective->isComplete()) return false;

	bool objectiveComplete = false;
	bool stageComplete = false;
	bool taskComplete = false;
	auto stage = pObjective->getParentStage();
	auto task = stage->getParentTask();

	pObjective->addValue(1);

	auto packet = Payload::updateTaskObjective(pObjective);
	mOwner->getConnection()->sendPacket(packet);
	delete packet;

	// Check: Objective is now complete.
	pObjective->checkComplete();
	objectiveComplete = pObjective->isComplete();

	if (objectiveComplete) {
		// Check: Stage is now complete.
		stage->checkComplete();
		stageComplete = stage->isComplete();

		// Notify player that the objective was completed.
		mOwner->message(MessageType::White, "Your task '" + task->getTitle() + "' has been updated.");
	}
	if (stageComplete) {
		// Check: Task is now complete.
		task->checkComplete();
		taskComplete = task->isComplete();
	}

	if (stageComplete && !taskComplete) {

		auto messagePacket = Payload::Zone::TaskMessage::construct(task->getIndex(), task->getType(), task->getID(), pObjective->getIndex(), 0, 1);
		mOwner->getConnection()->sendPacket(messagePacket);
		delete messagePacket;
	}

	if (taskComplete) {
		auto messagePacket = Payload::Zone::TaskMessage::construct(task->getIndex(), task->getType(), task->getID(), pObjective->getIndex(), 1, 0);
		mOwner->getConnection()->sendPacket(messagePacket);
		delete messagePacket;
	}

	return true;
}

TaskSystem::Task* TaskController::getTask(const u32 pTaskIndex) {
	for (auto i : mQuestTasks) {
		if (i->getIndex() == pTaskIndex)
			return i;
	}
	return nullptr;
}

void TaskController::onObjectiveComplete() {

}

void TaskController::onStageComplete() {

}

void TaskController::onTaskComplete() {

}
