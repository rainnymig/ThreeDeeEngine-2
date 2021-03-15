#include "pch.h"
#include "WorkDispatcher.h"

#include "Job.h"
#include "Worker.h"

namespace tde
{
	WorkDispatcher::WorkDispatcher(const std::string& aName)
		: mName(aName)
	{
		mWorkers.emplace_back(Worker(this));
	}
	
	WorkDispatcher::~WorkDispatcher()
	{
		mWorkers.clear();
	}

	void WorkDispatcher::Dispatch(Job aJob)
	{
		std::lock_guard<std::mutex> lock(mJobMutex);
		mPendingJobs.emplace_back(aJob);
	}

	bool WorkDispatcher::HasPendingJobs()
	{
		return !mPendingJobs.empty();
	}
}

