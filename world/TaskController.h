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


	UF replies with 0x43BC (size=4112) after sending OP_TaskDescription .. strange
	UF appears to recv this opcode as well.
*/

class Character;

namespace Data {
	struct TaskObjective;
	struct Task;
}

struct CurrentTaskObjective {
	u32 mValue = 0;
	bool mHidden = false;
	Data::TaskObjective* mObjectiveData;
};

struct CurrentTask {
	bool isActive() const { return mActive; } // TODO: Become inactive when timer runs out.
	bool mActive = true;
	u32 mStartTime = 0;
	Data::Task* mTaskData = nullptr;
	CurrentTaskObjectives mObjectives;
};

struct CompleteTask {
	Data::Task* mTask = nullptr;
	u32 mTimeCompleted = 0;
};

class TaskController {
public:

	TaskController(Character* pCharacter);
	~TaskController();

	inline Character* getOwner() const { return mOwner; }
	inline AvailableTasks& getAvailableTasks() { return mAvailableTasks; }
	inline CurrentTasks& getActiveTasks() { return mCurrentTasks; }

	void onLoad();
	void onSave() const;
	void onHistoryRequest(const u32 pIndex);

private:

	Character* mOwner = nullptr;

	AvailableTasks mAvailableTasks;
	CurrentTasks mCurrentTasks;
	CompleteTasks mCompleteTasks;
};