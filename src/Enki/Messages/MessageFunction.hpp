#pragma once

//SELF
#include "Message.hpp"

//STD
#include <functional>

namespace enki
{
class MessageFunction : public MessageID<hash_constexpr("Function")>
{
public:
	MessageFunction(std::function<void()> func)
		: function(std::move(func))
	{
	}

	void execute() const
	{
		function();
	}

private:
	std::function<void()> function;
};
}	// namespace enki