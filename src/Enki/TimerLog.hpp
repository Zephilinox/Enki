#pragma once

//STD
#include <chrono>
#include <map>
#include <string>

//LIBS
#include <spdlog/spdlog.h>

struct Log
{
	int times = 0;
	float avg = 0;
};

static inline std::map<std::string, Log> durations;

class TimerLog
{
public:
	using clock = std::chrono::high_resolution_clock;
	using nanoseconds = std::chrono::nanoseconds;
	using nanoseconds_float = std::chrono::duration<float, nanoseconds::period>;

	TimerLog(std::string file, std::string function, int line);
	~TimerLog();

private:
	std::string file;
	std::string func;
	int line;
	clock::time_point start;

	inline static int indentation = 0;
};

#define TIMER_LOG() TimerLog log##__LINE__(__FILE__, __FUNCTION__, __LINE__);