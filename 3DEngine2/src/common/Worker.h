#pragma once

namespace tde
{
	class Job;
	class WorkDispatcher;

	class Worker
	{
	public:
		Worker(WorkDispatcher* apDispatcher);
		Worker(Worker&& aOther) noexcept;
		~Worker();

		bool HasPendingJobs();

	private:
		void WorkingRoutine();
		
		std::thread mThread;
		std::vector<Job> mPendingJobs;
		WorkDispatcher* mpDispatcher;
		bool mIsWorking = false;
	};


}


