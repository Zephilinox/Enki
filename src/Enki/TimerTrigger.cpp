#include "TimerTrigger.hpp"

namespace enki
{
	void TimerTrigger::update(float dt)
	{
		time += dt;

		if (trigger_time > 0.0f && time >= trigger_time)
		{
			on_trigger.emit();
			time -= trigger_time;
		}
	}

	float TimerTrigger::getTime() const
	{
		return time;
	}
}