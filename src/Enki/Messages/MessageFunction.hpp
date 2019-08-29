#pragma once

//STD
#include <functional>

//SELF
#include "Message.hpp"

namespace enki
{
	class MessageFunction : public MessageID<hash_constexpr("Function")>
	{
	public:
		MessageFunction(std::function<void()> func)
			: function(std::move(func))
		{}

		void execute() const
		{
			function();
		}

	private:
		std::function<void()> function;
	};
}