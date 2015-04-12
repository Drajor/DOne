#include "Task.h"
#include "Data.h"
#include "Utility.h"

using namespace TaskSystem;

TaskSystem::Task::Task(Data::Task* pTaskData) : mData(pTaskData) {
	for (auto i : mData->mStages) {
		auto stage = new Stage(this, i);
		mStages.push_back(stage);
	}
}

Stage* TaskSystem::Task::getStage(const u32 pStageIndex) {
	for (auto i : mStages) {
		if (i->getIndex() == pStageIndex)
			return i;
	}
	return nullptr;
}

Stage::Stage(Task* pParentTask, const Data::TaskStage* pStageData) : mParent(pParentTask), mData(pStageData) {
	for (auto i : mData->mObjectives) {
		auto objective = new Objective(this, i);
		mObjectives.push_back(objective);
	}
}

Objective::Objective(Stage* pParentStage, const Data::TaskObjective* pObjectiveData) : mData(pObjectiveData), mParent(pParentStage) {

}

void Task::checkComplete() {
	if (isComplete()) return;

	for (auto& i : mStages) {
		i->checkComplete();
		if (!i->isComplete()) return;
	}

	mComplete = true;
}

u32 Task::getID() const { return mData->mID; }
u32 Task::getType() const { return mData->mType; }
u32 Task::getRewardType() const { return mData->mRewardType; }
u32 Task::getDuration() const { return mData->mDuration; }
u32 Task::getStartTime() const { return mStartTime; }
i32 Task::getRewardPoints() const { return mData->mPointsReward; }
const String& Task::getTitle() const { return mData->mTitle; }
const String& Task::getDescription() const { return mData->mDescription; }
const String& Task::getRewardText() const { return mData->mRewardText; }

void Stage::checkComplete() {
	if (isComplete()) return;

	for (auto& i : mObjectives) {
		i->checkComplete();
		if (!i->isComplete()) return;
	}

	mComplete = true;
}

u32 TaskSystem::Stage::getIndex() const {return mData->mIndex; }

Objective* TaskSystem::Stage::getObjective(const u32 pObjectiveIndex) {
	for (auto i : mObjectives) {
		if (i->getIndex() == pObjectiveIndex)
			return  i;
	}
	return nullptr;
}

void Objective::checkComplete() {
	mComplete = mValue == mData->mRequired;
}

const String& Objective::getTextA() const { return mData->mTextA; }
const String& Objective::getTextB() const { return mData->mTextB; }
const String& Objective::getTextC() const { return mData->mTextC; }
u32 Objective::getRequired() const { return mData->mRequired; }
bool Objective::isOptional() const { return mData->mOptional; }
u32 Objective::getIndex() const { return mData->mIndex; }

u32 Objective::getType() const { return mData->mType; }
u32 Objective::getZoneID() const { return mData->mZoneID; }

void TaskSystem::Objective::addValue(const u32 pAmount) {
	if (isComplete()) return;

	mValue += pAmount;
	mValue = Utility::clamp<u32>(mValue, 0, mData->mRequired);
}
