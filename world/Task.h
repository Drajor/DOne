#pragma once

#include "TaskConstants.h"

namespace Data {
	struct TaskObjective;
	struct Task;
}

namespace TaskSystem {
	class Task;
	class Stage;
	class Objective;

	class Task {
	public:
		Task(Data::Task* pTaskData);;
		inline const Data::Task* getData() const { return mData; }

		const std::list<Stage*>& getStages() const { return mStages; }

		inline const u32 getIndex() const { return mIndex; }
		inline void setIndex(const u32 pIndex) { mIndex = pIndex; }

		// Checks whether this Task is complete.
		void checkComplete();

		// Returns whether this Task is complete or not.
		inline bool isComplete() const { return mComplete; }

		// Returns whether this Task is expired or not.
		inline bool isExpired() const { return false; }

		u32 getID() const;
		u32 getType() const;
		u32 getRewardType() const;
		u32 getDuration() const;
		u32 getStartTime() const;
		i32 getRewardPoints() const;
		const String& getTitle() const;
		const String& getDescription() const;
		const String& getRewardText() const;

		// Returns a Stage by index.
		Stage* getStage(const u32 pStageIndex);

	private:

		const Data::Task* mData;
		u32 mIndex = 0;
		bool mComplete = false;
		u32 mStartTime = 0;
		std::list<Stage*> mStages;
	};

	class Stage {
	public:
		Stage(Task* pParentTask, const Data::TaskStage* pStageData);
		Task* getParentTask() const { return mParent; }

		const std::list<Objective*>& getObjectives() const { return mObjectives; }

		u32 getIndex() const;

		// Checks whether this Task Stage is complete.
		void checkComplete();

		// Returns whether this Task Stage is complete or not.
		inline bool isComplete() const { return mComplete; }

		// Returns an Objective by index.
		Objective* getObjective(const u32 pObjectiveIndex);

	private:

		const Data::TaskStage* mData;
		Task* mParent = nullptr;
		bool mComplete = false;
		std::list<Objective*> mObjectives;
	};

	class Objective {
	public:

		Objective(Stage* pParentStage, const Data::TaskObjective* pObjectiveData);

		Stage* getParentStage() const { return mParent; }
		Task* getParentTask() const { return mParent->getParentTask(); }

		void addValue(const u32 pAmount);
		inline u32 getValue() const { return mValue; }
		void checkComplete();
		inline bool isComplete() const { return mComplete; }

		// Returns the ObjectiveType
		u32 getType() const;
		u32 getZoneID() const;
		u32 getRequired() const;
		u32 getIndex() const;
		// Returns whether this Task Objective is optional or not.
		bool isOptional() const;

		const String& getTextA() const;
		const String& getTextB() const;
		const String& getTextC() const;

	private:

		const Data::TaskObjective* mData;
		Stage* mParent = nullptr;
		bool mComplete = false;
		u32 mValue = 0;
	};
}