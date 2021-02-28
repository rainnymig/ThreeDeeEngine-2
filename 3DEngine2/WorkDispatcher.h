#pragma once

namespace tde
{
	class Job;
	class Worker;

	class WorkDispatcher
	{
	public:
		WorkDispatcher(const std::string& aName);
		~WorkDispatcher();

		void Dispatch(Job aJob);
		bool HasPendingJobs();

		std::mutex mJobMutex;

	private:
		std::string mName;
		std::vector<Job> mPendingJobs;
		std::vector<Worker> mWorkers;

		friend Worker;
	};
}
