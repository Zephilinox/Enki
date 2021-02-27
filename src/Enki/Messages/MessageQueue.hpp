#pragma once

//SELF
#include "Enki/Signals/Signal.hpp"
#include "Enki/Timer.hpp"
#include "Message.hpp"

//STD
#include <queue>

namespace enki
{

class MessageQueue
{
public:
	using FunctionType = Signal<Message*>::FunctionType;

	//Note: This is not a hard limit.
	//Highly dependent on the number of listeners and priority messages.
	std::size_t processMessages(Timer::nanoseconds_float max_processing_time);

	Connection addListener(FunctionType&& function);
	bool removeListener(Connection c);

	template <typename T, typename... Args>
	void sendMessage(Args... args)
	{
		message_queue.emplace(std::make_unique<T>(args...));
	}

	template <typename T, typename... Args>
	void sendPriorityMessage(Args... args)
	{
		priority_messages.push_back(std::make_unique<T>(args...));
	}

	template <typename T>
	Connection addListener(typename Signal<T>::FunctionType&& function)
	{
		using derived = typename std::remove_reference<T>::type;
		static_assert(std::is_base_of_v<Message, derived>);

		auto lambda = [function](Message* msg) -> void {
			if (msg->id == derived::ID)
				function(*static_cast<derived*>(msg));
		};

		return messenger.connect(std::move(lambda));
	}

private:
	//Process all of the priority messages, no matter how long it takes.
	std::size_t processPriorityMessages();
	std::size_t processNormalMessages(Timer::nanoseconds_float max_processing_time);

	Signal<Message*> messenger;
	std::queue<std::unique_ptr<Message>> message_queue;
	std::vector<std::unique_ptr<Message>> priority_messages;
	Timer timer;
};
}	 // namespace enki