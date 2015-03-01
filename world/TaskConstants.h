#pragma once

#include <array>
#include <list>
#include "Types.h"

struct CurrentTaskObjective;
struct CurrentTask;
struct CompleteTask;
namespace Data {
	struct Task;
	struct TaskObjective;
}

static const u32 MaxActiveTasks = 10;

typedef std::array<CurrentTask*, MaxActiveTasks> CurrentTasks;
typedef std::list<Data::Task*> AvailableTasks;
typedef std::list<CurrentTaskObjective*> CurrentTaskObjectives;
typedef std::list<CompleteTask*> CompleteTasks;
typedef std::list<Data::TaskObjective*> TaskObjectives;

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
}