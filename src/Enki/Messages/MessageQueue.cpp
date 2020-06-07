#include "MessageQueue.hpp"

//STD
#include <iomanip>
#include <iostream>

//SELF
#include "Enki/TimerLog.hpp"

namespace enki
{
std::size_t MessageQueue::processMessages(Timer::nanoseconds_float max_processing_time)
{
	/*todo: Optimise this more by having listeners choose which events they care about
	 * thus less listeners being called back.*/

    timer.restart();

    const auto priority_msg_count = processPriorityMessages();
	const auto normal_msg_count = processNormalMessages(max_processing_time);
	const auto total_msg_count = priority_msg_count + normal_msg_count;

	if (total_msg_count > 0)
		std::cout << "INFO:\t MessageQueue took "<< std::fixed << std::setprecision(2) << std::setfill('0') << std::setw(3)<< timer.getElapsedTime<Timer::milliseconds>() << "ms processing " << std::setw(4) << total_msg_count << " messages. " << std::setw(4) << message_queue.size() << " remain\n";

	return total_msg_count;
}

Connection MessageQueue::addListener(FunctionType&& function)
{
	return messenger.connect(std::forward<FunctionType>(function));
}

bool MessageQueue::removeListener(Connection c)
{
	return messenger.disconnect(c);
}

std::size_t MessageQueue::processPriorityMessages()
{
	const auto msg_count = priority_messages.size();

	for (const auto& msg : priority_messages)
		messenger.emit(msg.get());
	
	priority_messages.clear();

	if (msg_count > 0)
	{
		const auto elapsed = timer.getElapsedTime<Timer::milliseconds>();
		//todo: use fmt::format
		//todo: log it
		std::cout << "INFO:\t MessageQueue took " << std::fixed << std::setprecision(2) << std::setfill('0') << std::setw(3) << elapsed << "ms processing " << std::setw(4) << msg_count << " priority messages.\n";
	}

	return msg_count;
}

std::size_t MessageQueue::processNormalMessages(Timer::nanoseconds_float max_processing_time)
{
	//If we have any extra time, process any other messages we can
	int msg_count = 0;

	while (!message_queue.empty() && timer.getChronoElapsedTime() < max_processing_time)
	{
		//do 100 messages at a time as getChronoElapsedTime() is super expensive
		for (int i = 0; i < 100 && !message_queue.empty(); ++i)
		{
			msg_count++;
			messenger.emit(message_queue.front().get());
			message_queue.pop();
		}
	}

	return msg_count;
}
}	// namespace enki