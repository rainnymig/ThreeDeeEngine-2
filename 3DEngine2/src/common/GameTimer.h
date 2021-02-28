#pragma once

#include <cmath>
#include <cstdint>
#include <exception>
#include <profileapi.h>

namespace tde
{
	//	https://bell0bytes.eu/keeping-track-of-time/
	class GameTimer
	{
	public:
		GameTimer()
			: mStartTick(0)
			, mStopTick(0)
			, mPreviousTick(0)
			, mCurrentTick(0)
			, mTotalstoppedTick(0)
			, mStopped(true)
			, mSecondsPerTick(0.0)
			, mDeltaTimeSecond(0.0)
		{
			LARGE_INTEGER freq;
			if (!QueryPerformanceFrequency(&freq))
			{
				throw std::exception("QueryPerformanceFrequency failed");
			}
			mSecondsPerTick = 1.0 / static_cast<double>(freq.QuadPart);
		}

		double getTotalTime() const
		{
			if (mStopped)
			{
				return (mStopTick - mTotalstoppedTick - mStartTick) * mSecondsPerTick;
			}
			else
			{
				return (mCurrentTick - mTotalstoppedTick - mStartTick) * mSecondsPerTick;
			}
		}
		double getDeltaTime() const
		{
			if (mStopped)
			{
				return 0.0;
			}
			return (mCurrentTick - mPreviousTick) * mDeltaTimeSecond;
		}

		void reset()
		{
			mStartTick = 0;
			mCurrentTick = 0;
			mStopTick = 0;
			mTotalstoppedTick = 0;
			mPreviousTick = 0;
		}
		void start()
		{
			if (!mStopped)
			{
				return;
			}

			if (!QueryPerformanceCounter((LARGE_INTEGER*)&mStartTick))
			{
				throw std::exception("QueryPerformanceCounter failed by start");
			}

			mCurrentTick = mStartTick;
			mPreviousTick = mStartTick;
		}
		void stop()
		{
			if (mStopped)
			{
				return;
			}

			if (!QueryPerformanceCounter((LARGE_INTEGER*)&mStopTick))
			{
				throw std::exception("QueryPerformanceCounter failed by stop");
			}

			mStopped = true;
		}
		void tick()
		{
			mPreviousTick = mCurrentTick;
			if (!QueryPerformanceCounter((LARGE_INTEGER*)&mCurrentTick))
			{
				throw std::exception("QueryPerformanceCounter failed by tick");
			}
		}

	private:
		uint64_t mStartTick;
		uint64_t mCurrentTick;
		uint64_t mStopTick;
		uint64_t mTotalstoppedTick;
		uint64_t mPreviousTick;

		double mSecondsPerTick;
		double mDeltaTimeSecond;		//	in seconds

		bool mStopped = true;			//	whether the timer is stopped or paused
	};
}
