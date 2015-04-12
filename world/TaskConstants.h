#pragma once

#include <array>
#include <list>
#include <vector>
#include "Types.h"

struct CurrentTaskObjective;
struct CurrentTask;
struct CompletedTask;

namespace Data {
	struct CompletedTask;
	struct Task;
	struct TaskStage;
	struct TaskObjective;
}

static const u32 MaxTaskID = 5000;
static const u32 MaxSoloTasks = 1;
static const u32 MaxSharedTasks = 1;
static const u32 MaxQuestTasks = 19;
//static const u32 MaxActiveTasks = MaxSoloTasks + MaxSharedTasks + MaxQuestTasks;

struct CompletedTask {
	CompletedTask(const Data::Task* pTaskData, const u32 pTimeCompleted) : mTaskData(pTaskData), mTimeCompleted(pTimeCompleted) {};
	const Data::Task* mTaskData;
	const u32 mTimeCompleted;
};

//typedef std::array<CurrentTask*, MaxQuestTasks> CurrentTasks;
typedef std::list<CurrentTask*> CurrentTasks;
typedef std::vector<CompletedTask> CompletedTasks;

typedef std::list<Data::Task*> AvailableTasks;
typedef std::list<CurrentTaskObjective*> CurrentTaskObjectives;

typedef std::list<Data::CompletedTask> CompletedTasksData;
typedef std::list<Data::TaskStage*> TaskStagesData;
typedef std::list<Data::TaskObjective*> TaskObjectivesData;

namespace TaskType {
	enum : u32 {
		Task = 0,
		Shared = 1,
		Quest = 2,
	};

	static bool isValid(const u32 pTaskType) {
		switch (pTaskType) {
			case TaskType::Task:
				return true;
			case TaskType::Shared:
				return true;
			case TaskType::Quest:
				return true;
			default:
				break;
		}
		return false;
	}
}

namespace TaskRewardType {
	enum : u8 {
		Complex = 0,
		Simple = 1,
	};

	static bool isValid(const u8 pTaskRewardType) {
		switch (pTaskRewardType) {
		case TaskRewardType::Complex:
			return true;
		case TaskRewardType::Simple:
			return true;
		default:
			break;
		}
		return false;
	}
}

namespace ObjectiveType {
	enum : u32 {
		Deliver = 1,
		Kill = 2,
		Loot = 3,
		Hail = 4,
		Explore = 5,
		Tradeskill = 6,
	};

	static bool isValid(const u32 pObjectiveType) {
		switch (pObjectiveType) {
		case ObjectiveType::Deliver:
			return true;
		case ObjectiveType::Kill:
			return true;
		case ObjectiveType::Loot:
			return true;
		case ObjectiveType::Hail:
			return true;
		case ObjectiveType::Explore:
			return true;
		case ObjectiveType::Tradeskill:
			return true;
		default:
			break;
		}
		return false;
	}

	static const String toString(const u32 pObjectiveType) {
		switch (pObjectiveType) {
		case ObjectiveType::Deliver:
			return "Deliver";
		case ObjectiveType::Kill:
			return "Kill";
		case ObjectiveType::Loot:
			return "Loot";
		case ObjectiveType::Hail:
			return "Speak to";
		case ObjectiveType::Explore:
			return "Explore";
		case ObjectiveType::Tradeskill:
			return "Tradeskill";
		default:
			break;
		}
		return "Unknown";
	}
}