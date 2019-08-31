#pragma once

//STD
#include <queue>

//SELF
#include "Enki/Signals/Signal.hpp"
#include "Enki/Timer.hpp"
#include "Message.hpp"

namespace enki
{
class MessageQueue
{
public:
	using FunctionType = Signal<Message*>::FunctionType;

	//Note: This is not a hard limit.
	//Highly dependent on the number of listeners and priority messages.
	void processMessages(Timer::nanoseconds_float max_processing_time);

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
		priority_message_queue.emplace(std::make_unique<T>(args...));
	}

private:
	Signal<Message*> messenger;
	std::queue<std::unique_ptr<Message>> message_queue;
	std::queue<std::unique_ptr<Message>> priority_message_queue;
	Timer timer;
};
}	// namespace enki