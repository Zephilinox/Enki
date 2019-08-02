#pragma once

//STD
#include <chrono>

namespace enki
{
	using namespace std::chrono_literals;

	class Timer
	{
	public:
		using clock = std::chrono::high_resolution_clock;
		using nanoseconds = std::chrono::nanoseconds;
		using microseconds = std::chrono::microseconds;
		using milliseconds = std::chrono::milliseconds;
		using seconds = std::chrono::seconds;

		using nanoseconds_float = std::chrono::duration<float, nanoseconds::period>;
		using microseconds_float = std::chrono::duration<float, microseconds::period>;
		using milliseconds_float = std::chrono::duration<float, milliseconds::period>;
		//Most likely to be used duration. Used by Enki for delta time.
		using seconds_float = std::chrono::duration<float, seconds::period>;

		Timer();

		//in seconds
		float getElapsedTime() const;
		//for further chrono operations, returns std::chrono::nanoseconds for std::chrono::duration functionality
		nanoseconds getChronoElapsedTime() const;
		void restart();
		void pause(bool pause);
		bool isPaused();

		//use with various chrono durations for desired time period, returns a float.
		template <class T>
		float getElapsedTime() const;

	private:
		bool is_paused = false;
		clock::time_point start_time;
		clock::time_point pause_start_time;
		clock::time_point pause_end_time;
		nanoseconds paused_duration = 0ns;
	};

	template <class T>
	float Timer::getElapsedTime() const
	{
		return std::chrono::duration<float, typename T::period>(clock::now() - start_time - paused_duration).count();
	}
}
