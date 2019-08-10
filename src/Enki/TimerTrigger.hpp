#pragma once

//STD
#include <chrono>

//LIBS
#include <Enki/Signals/Signal.hpp>

using namespace std::chrono_literals;

namespace enki
{
	class TimerTrigger
	{
	public:
		void update(float dt);
		[[nodiscard]] float getTimeInSeconds() const;

		Signal<> on_trigger;
		float trigger_time_seconds = 0.0f;

	private:
		float time_seconds = 0;
	};
}