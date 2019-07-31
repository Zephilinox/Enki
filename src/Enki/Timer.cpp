#include "Timer.hpp"

namespace enki
{
	Timer::Timer()
	{
		restart();
	}

	float Timer::getElapsedTime() const
	{
		return getElapsedTime<seconds>();
	}

	Timer::nanoseconds Timer::getChronoElapsedTime() const
	{
		return clock::now() - start_time;
	}

	void Timer::restart()
	{
		start_time = clock::now();
	}
}