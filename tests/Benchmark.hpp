﻿#pragma once

//LIBS
#include <Enki/Timer.hpp>
#include <Enki/Networking/RPC.hpp>

void zero()
{
	
}

void one([[maybe_unused]]int i)
{
}

void two([[maybe_unused]]int x, [[maybe_unused]]int y)
{
}

void three(int clientID, [[maybe_unused]]int channelID, std::string message)
{
	static volatile int i = 0;
	i = clientID;
}

void math(int x_vel, int y_vel)
{
	static float xpos = 0;
	static float ypos = 0;
	static float xvel = 0;
	static float yvel = 0;
	xvel += x_vel * (1 / 60.0f);
	yvel += y_vel * (1 / 60.0f);

	xpos += xvel;
	ypos += yvel;

	if (xpos > 100)
	{
		ypos = xpos;
		xpos = 0;
	}
}

void print_time(enki::Timer& t, int count)
{
	auto time = t.getChronoElapsedTime();
	std::cout << count << " calls took " <<
		std::chrono::duration_cast<enki::Timer::seconds_float>(time).count() << " seconds\n";
	std::cout << "each call took " <<
		std::chrono::duration_cast<enki::Timer::nanoseconds_float>(time).count() / count << " nanoseconds\n";
}

void benchmark_entity(enki::Timer&t, int count)
{
	enki::RPCManager rpcm(nullptr);
	ent e;

	t.restart();
	for (int i = 0; i < count; ++i)
	{
		e.do_thing(1, i);
	}

	std::cout << "###[Entity::do_thing]\n";
	print_time(t, count);
	std::cout << e.i << "\n";

	t.restart();
	for (int i = 0; i < count; ++i)
	{
		rpcm.callClassRPC(&ent::do_thing, "do_thing", &e, 1, i);
	}
	std::cout << "###[Entity::do_thing] RPC\n";
	print_time(t, count);
	std::cout << "\n";
}

template <typename F, typename... Args>
void benchmark(F* function, std::string name, enki::Timer t, int count, Args... args)
{
	//todo: rpcmanagers stuff shouldn't be static
	enki::RPCManager rpcm(nullptr);
	t.restart();
	for (int i = 0; i < count; ++i)
	{
		function(args...);
	}
	std::cout << "### [" << name << "]\n";
	print_time(t, count);

	t.restart();
	for (int i = 0; i < count; ++i)
	{
		rpcm.callGlobalRPC(function, name, args...);
	}
	std::cout << "###[" << name << "] RPC\n";
	print_time(t, count);
	std::cout << "\n";
}

void benchmark()
{
	enki::RPCManager rpcm(nullptr);
	rpcm.registerGlobalRPC(enki::RPCType::ALL, "zero", &zero);
	rpcm.registerGlobalRPC(enki::RPCType::ALL, "one", &one);
	rpcm.registerGlobalRPC(enki::RPCType::ALL, "two", &two);
	rpcm.registerGlobalRPC(enki::RPCType::ALL, "three", &three);
	rpcm.registerGlobalRPC(enki::RPCType::ALL, "math", &math);

	enki::Timer timer;

#ifdef _DEBUG
	int count = 500'000;
#else
	int count = 50'000'000;
#endif

	std::cout << "\n";
	//benchmark(&zero, "zero", timer, count);
	benchmark(&one, "one", timer, count, count);
	benchmark(&two, "two", timer, count, count * 2, count * 2);
	std::string s = "hello";
	benchmark(&three, "three", timer, count, 0, 0, s);
	benchmark(&math, "math", timer, count, count, count);

	benchmark_entity(timer, count);
}