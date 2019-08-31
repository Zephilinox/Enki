#include "TimerTrigger.hpp"

namespace enki
{
void TimerTrigger::update(float dt)
{
	time_seconds += dt;

	if (trigger_time_seconds > 0.0f && time_seconds >= trigger_time_seconds)
	{
		time_seconds -= trigger_time_seconds;
		on_trigger.emit();
	}
}

float TimerTrigger::getTimeInSeconds() const
{
	return time_seconds;
}
}	// namespace enki