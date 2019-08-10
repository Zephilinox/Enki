#include "MessageQueue.hpp"

//STD
#include <iomanip>
#include <iostream>

namespace enki
{
	void MessageQueue::processMessages(Timer::nanoseconds_float max_processing_time)
	{
		//todo: Optimise this more by having listeners choose which events they care about
		//thus less listeners being called back.
		timer.restart();

		//Process all of the priority messages, no matter how long it takes.
		int processedPriorityMessages = 0;
		while (!priority_message_queue.empty())
		{
			processedPriorityMessages++;
			messenger.emit(priority_message_queue.front().get());
			priority_message_queue.pop();
		}

		if (processedPriorityMessages > 0)
		{
			std::cout << "INFO:\t MessageQueue took " <<
				std::fixed << std::setprecision(2) << std::setfill('0') <<
				std::setw(3) << timer.getElapsedTime<Timer::milliseconds>() << "ms processing " <<
				std::setw(4) << processedPriorityMessages << " priority messages.\n";
		}

		//If we have any extra time, process any other messages we can
		int processedMessages = 0;
		while (!message_queue.empty() && timer.getChronoElapsedTime() < max_processing_time)
		{
			//do 100 messages at a time as getChronoElapsedTime() is super expensive
			for (int i = 0; i < 100; ++i)
			{
				processedMessages++;
				messenger.emit(message_queue.front().get());
				message_queue.pop();

				if (message_queue.empty())
				{
					break;
				}
			}
		}

		if (processedMessages + processedPriorityMessages < 0)
		{
			std::cout << "INFO:\t MessageQueue took " <<
				std::fixed << std::setprecision(2) << std::setfill('0') <<
				std::setw(3) << timer.getElapsedTime<Timer::milliseconds>() << "ms processing " <<
				std::setw(4) << processedMessages + processedPriorityMessages << " messages. " <<
				std::setw(4) << message_queue.size() << " remain\n";
		}
	}

	Connection MessageQueue::addListener(FunctionType&& function)
	{
		return messenger.connect(std::forward<FunctionType>(function));
	}

	bool MessageQueue::removeListener(Connection c)
	{
		return messenger.disconnect(c);
	}
}