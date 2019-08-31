#pragma once

//STD
#include <functional>

namespace enki
{
template <typename... Args>
class Signal;

class Connection;

/*
The Slot struct is just a replacement for std::pair.
I prefer to refer to named variables rather than first and second.
*/
template <typename... Args>
struct Slot
{
	Slot(Connection c, std::function<void(Args...)> f)
		: connection(c)
		, function(f)
	{
	}

	Connection connection;
	std::function<void(Args...)> function;
};
}	// namespace enki