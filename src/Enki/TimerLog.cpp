#include "TimerLog.hpp"

TimerLog::TimerLog(std::string file, std::string function, int line)
	: file(file)
	, func(function)
	, line(line)
{
	std::string indent;
	for (int i = 0; i < indentation; ++i)
	{
		indent += '#';
	}

	spdlog::info("{}{}::{}", indent, func, line);
	indentation++;
	start = clock::now();
}

TimerLog::~TimerLog()
{
	auto now = clock::now();
	auto duration = std::chrono::duration<float, nanoseconds::period>(now - start).count();

	float avg = 0;
	avg = durations[func].avg * durations[func].times;
	avg += duration;
	durations[func].times++;
	avg /= durations[func].times;
	durations[func].avg = avg;

	indentation--;
	std::string indent;
	for (int i = 0; i < indentation; ++i)
	{
		indent += '#';
	}

	spdlog::info("{}{}::{} {}ms {}ms", indent, func, line,
		duration / 1'000'000, durations[func].avg / 1'000'000);
}
