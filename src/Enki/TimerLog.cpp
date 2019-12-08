#include "TimerLog.hpp"

TimerLog::TimerLog(std::string file, std::string function, int line)
	: file(file)
	, func(function)
	, line(line)
{
	const std::string_view indent(indents, indentation);

	spdlog::info("{}{}::{}", indent, func, line);
	indentation++;
	start = clock::now();
}

TimerLog::~TimerLog()
{
	const auto now = clock::now();
	const auto duration = std::chrono::duration<float, nanoseconds::period>(now - start).count();

	float avg = durations[func].avg * durations[func].times;
	avg += duration;
	durations[func].times++;
	avg /= durations[func].times;
	durations[func].avg = avg;

	indentation--;
	const std::string_view indent(indents, indentation);

	spdlog::info("{}{}::{} {}ms vs {}avg ms", indent, func, line,
		duration / 1'000'000, avg / 1'000'000);
}
