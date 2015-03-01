#include "TaskController.h"
#include "Data.h"

TaskController::TaskController(Character* pCharacter) : mOwner(pCharacter) {
	for (auto i : mCurrentTasks)
		i = new CurrentTask();

	auto a = new Task();
	a->mID = 1;
	a->mTitle = "Example Task A";
	a->mDescription = "Description of Example Task A";
	mAvailableTasks.push_back(a);

	auto b = new Task();
	b->mID = 2;
	b->mTitle = "Example Task B";
	b->mDescription = "Description of Example Task B";
	mAvailableTasks.push_back(b);
}

TaskController::~TaskController() {
	for (auto i : mCurrentTasks)
		delete i;
	mCurrentTasks.fill(nullptr);
}

void TaskController::onLoad()
{

}

void TaskController::onSave() const
{

}

void TaskController::onHistoryRequest(const u32 pIndex) {
}

