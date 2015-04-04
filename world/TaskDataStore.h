#pragma once

#include "TaskConstants.h"

namespace Data {
	struct Task;
}
class ILog;
class ILogFactory;
class IDataStore;

class TaskDataStore {
public:

	~TaskDataStore();

	bool initialise(IDataStore* pDataStore, ILogFactory* pLogFactory);
	Data::Task* getData(const u32 pTaskID) const;

private:

	bool mInitialised = false;
	ILog* mLog = nullptr;
	IDataStore* mDataStore = nullptr;

	std::array<Data::Task*, MaxTaskID> mData;
};