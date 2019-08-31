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
	return clock::now() - start_time - paused_duration;
}

void Timer::restart()
{
	start_time = clock::now();
	paused_duration = 0ns;
	is_paused = false;
}

void Timer::pause(bool pause)
{
	if (!is_paused && pause)
	{
		pause_start_time = clock::now();
	}
	else if (is_paused && !pause)
	{
		pause_end_time = clock::now();
		paused_duration += pause_end_time - pause_start_time;
	}

	is_paused = pause;
}

bool Timer::isPaused() const
{
	return is_paused;
}
}	// namespace enki