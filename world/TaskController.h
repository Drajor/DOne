#pragma once

#include "TaskConstants.h"

/*

Quest Journal Window:
	- 'Current Tasks'
	- 'Shared Tasks'
	- 'Quest History'


	https://www.everquest.com/news/imported-eq-enus-50720

	OP_TaskQuit = 27322(0x6ABA) (size= 8) /taskquit
	OP_TaskAddPlayer = 25173(0x6255) (size=72) /taskaddplayer tits
	OP_TaskRemovePlayer = 6370(0x18E2) (size=72) /taskremoveplayer tits
	OP_TaskMakeLeader = 22835(0x5933) (size=72) /taskmakeleader tits
	OP_TaskPlayerListRequest = ? /taskplayerlist
	OP_PreviewReward = 13671(0x3567) (size=8) 

	UF replies with 0x43BC (size=4112) after sending OP_TaskDescription .. strange
	UF appears to recv this opcode as well.
	- If you send this back to the client (tested with memset(1)) it will stop sending it.


	Client supports 19 quest type tasks and 1 solo type task OR 1 shared type task. Therefore a total of 20 tasks.

	Missing;
	- Min/Max level
	- Shrouded
	- Monster Mission
*/

class Character;
class TaskDataStore;
class ILog;
class ILogFactory;

namespace Data {
	struct TaskObjective;
	struct Task;
	struct CompletedTask;
}

namespace TaskSystem {
	class Task;
	class Stage;
	class Objective;
}

//struct CurrentTaskObjective {
//	u32 mValue = 0;
//	bool mComplete = false;
//	bool mHidden = false;
//	Data::TaskObjective* mObjectiveData;
//};
//
//struct CurrentTask {
//	u32 mIndex = 0;
//	bool isActive() const { return mActive; } // TODO: Become inactive when timer runs out.
//	bool mActive = true;
//	bool isComplete()  const { return mComplete; }
//	bool mComplete = false;
//
//	u32 mStartTime = 0;
//	bool isFree() const { return mTaskData == nullptr; }
//	Data::Task* mTaskData = nullptr;
//	CurrentTaskObjectives mObjectives;
//};


class TaskController {
public:

	//TaskController(Character* pCharacter, TaskDataStore* pTaskDataStore);
	~TaskController();

	bool initialise(Character* pCharacter, TaskDataStore* pTaskDataStore, ILogFactory* pLogFactory);

	inline Character* getOwner() const { return mOwner; }
	inline const std::list<TaskSystem::Task*>& getTasks() const { return mQuestTasks; }
	inline AvailableTasks& getAvailableTasks() { return mAvailableTasks; }
	//inline CurrentTasks& getActiveTasks() { return mQuestTasks; }

	bool onLoad(CompletedTasksData& pCompletedTasksData);
	void onSave(CompletedTasksData& pCompletedTasksData) const;

	void onEnterZone();
	void onHistoryRequest(const u32 pIndex);

	// Objective Events.
	bool onHail(const u32 pNPCTypeID, const u16 pZoneID);
	bool onDeath(const u32 pNPCTypeID, const u16 pZoneID);
	bool onDeliver(const u32 pNPCTypeID, const u16 pZoneID);

	bool hasTask(const u32 pTaskID) const;
	bool hasSharedTask() const { return false; } // TODO:
	bool hasSoloTask() const { return false; } // TODO:
	bool hasCompletedTask(const u32 pTaskID) const { return false; } // TODO:

	bool add(const u32 pTaskID, const bool pShowQuestJournal);

	// Removes a Task by ID.
	bool remove(const u32 pTaskID);

	// Removes a Task by index and type.
	bool remove(const u32 pIndex, const u32 pTaskType);

	bool incrementObjective(const u32 pTaskIndex, const u32 pStageIndex, const u32 pObjectiveIndex);
	bool incrementObjective(TaskSystem::Objective* pObjective);

	// Returns a Task by index.
	TaskSystem::Task* getTask(const u32 pTaskIndex);

private:

	const bool mInitialised = false;
	ILog* mLog = nullptr;
	Character* mOwner = nullptr;
	TaskDataStore* mTaskDataStore = nullptr;

	AvailableTasks mAvailableTasks;
	TaskSystem::Task* mSoloTask = nullptr;
	TaskSystem::Task* mSharedTask = nullptr;
	std::list<TaskSystem::Task*> mQuestTasks;

	CompletedTasks mCompletedTasks;

	TaskSystem::Task* makeQuestTask(Data::Task* pTask);

	void onObjectiveComplete();
	void onStageComplete();
	void onTaskComplete();
};