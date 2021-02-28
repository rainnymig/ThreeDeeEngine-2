#include "pch.h"
#include "Worker.h"

#include "WorkDispatcher.h"
#include "Job.h"

namespace tde
{
	Worker::Worker(WorkDispatcher* apDispatcher)
		: mpDispatcher(apDispatcher)
	{
		mIsWorking = true;
		mThread = std::thread(std::bind(&Worker::WorkingRoutine, this));
	}

	// note: probably need much more refinement to handle movement when not empty
	Worker::Worker(Worker&& aOther) noexcept
		: mThread(std::move(aOther.mThread)), mpDispatcher(aOther.mpDispatcher), 
		mIsWorking(aOther.mIsWorking), mPendingJobs(std::move(aOther.mPendingJobs))
	{
	}

	Worker::~Worker()
	{
		mIsWorking = false;
		if (mThread.joinable())
		{
			mThread.join();
		}
	}

	bool Worker::HasPendingJobs()
	{
		return !mPendingJobs.empty();
	}

	void Worker::WorkingRoutine()
	{
		while (mIsWorking)
		{
			if (HasPendingJobs())
			{
				for (auto& job : mPendingJobs)
				{
					job.run();
				}

				mPendingJobs.clear();
			}

			if (!mIsWorking)
			{
				break;
			}

			if (mpDispatcher->HasPendingJobs())
			{
				std::lock_guard<std::mutex> lock(mpDispatcher->mJobMutex);
				mPendingJobs = std::move(mpDispatcher->mPendingJobs);
				mpDispatcher->mPendingJobs.clear();
			}
		}
	}
}
