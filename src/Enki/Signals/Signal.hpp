#pragma once

//STD
#include <vector>

//SELF
#include "Enki/Signals/Connection.hpp"
#include "Enki/Signals/Disconnector.hpp"
#include "Enki/Signals/Slot.hpp"

//todo: fix signal implementation to be r-values?
//currently too much being copied around, use pointers for non-POD

/*
https://github.com/fr00b0/nod
https://simmesimme.github.io/tutorials/2015/09/20/signal-slot
*/

/*
Args determines the arguments that the signal can emit
which is also the parameters functions must accept
Automatically invalidates all Connections created by calling connect()
*/
namespace enki
{
template <typename... Args>
class Signal
{
public:
	using FunctionType = std::function<void(Args...)>;

	Signal() noexcept
		: dc(std::make_shared<SignalDisconnector<Args...>>(this))
	{
	}

	//Connect to a non-member function/lambda/etc.
	//Connecting to an invalid function, such as a nullptr, will throw an exception.
	Connection connect(FunctionType&& function)
	{
		if (!function)
			throw std::runtime_error("Invalid function");

		slot_id_counter++;
		slots.emplace_back(Connection(dc, slot_id_counter), std::forward<FunctionType>(function));
		return slots.back().connection;
	}

	//Connect to a member function.
	//Undefined behaviour if the object instance is destroyed before it is disconnected.
	//You should pass a ManagedConnection to the object instance for it to store.
	template <typename T>
	Connection connect(T* instance, void (T::*function)(Args...))
	{
		auto wrapper = [=](Args... args) {
			(instance->*function)(std::forward<Args>(args)...);
		};

		return connect(std::forward<decltype(wrapper)>(wrapper));
	}

	//Connect to a const member function.
	//Undefined behaviour if the object instance is destroyed before it is disconnected.
	//You should pass a ManagedConnection to the object instance for it to store.
	template <typename T>
	Connection connect(T* instance, void (T::*function)(Args...) const)
	{
		auto wrapper = [=](Args... args) {
			(instance->*function)(std::forward<Args>(args)...);
		};

		return connect(std::forward<decltype(wrapper)>(wrapper));
	}

	//True if disconnect was successful.
	//False if the Connection is invalid or if no disconnection occurred.
	//Resets the Connection passed in, so that it's no longer valid.
	bool disconnect(Connection& connection)
	{
		if (!connection)
			return false;

		bool valid_disconnect = false;

		//Can be replaced with the C++11 equivalent if required
		//This is the only C++17 code used for Signals & Slots
		//auto in lambda's is a C++14 feature
		std::erase_if(slots, [&](const auto& slot) {
			if (connection == slot.connection)
			{
				connection = Connection();
				valid_disconnect = true;
				return true;
			}

			return false;
		});

		return valid_disconnect;
	}

	//calls all functions connected to the signal using the passed params
	void emit(Args... args)
	{
		for (const auto& s : slots)
		{
			//guaranteed valid from connect();
			if (s.function)
				s.function(args...);
		}
	}

private:
	/*
		Part 2 of type erasure :D
		SignalDisconnector stores a raw pointer to signal
		signal stores a shared_ptr to the disconnector

		This means that the raw pointer should always point to signal correctly
		since signal only shares weak_ptrs of the disconnector, not ref-counted shared_ptr's.

		If the signal is destroyed so is the SignalDisconnector
		thus invalidating all weak_ptr's held in any Connections for the signal
		*/
	template <typename... Args2>
	class SignalDisconnector : public Disconnector
	{
	public:
		SignalDisconnector(Signal<Args2...>* signal) noexcept
			: signal(signal)
		{
		}

		bool disconnect(Connection& c) final
		{
			if (signal)
				return signal->disconnect(c);

			return false;
		}

		Signal<Args2...>* signal;
	};

	//0 is invalid(like a nullptr), anything higher is fine.
	unsigned slot_id_counter = 0;

	//We own this, only share weak_ptrs so that connections
	//know when this signal is destroyed
	std::shared_ptr<SignalDisconnector<Args...>> dc;

	//Store all the functions and connections
	std::vector<Slot<Args...>> slots;
};
}	// namespace enki