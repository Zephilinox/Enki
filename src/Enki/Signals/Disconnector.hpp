#pragma once

namespace enki
{
	class Connection;

	/*
	Type Erasure:
	Signal defines a derived class of Disconnector with appropriate template types
	Connection has a weak_ptr to Disconnector, owned by the Signal.
	Disconnector is actually a SignalDisconnector which has a raw ptr to Signal
	When connection calls disconnector->disconnect(), it calls the derived one that signal passed in, which in turn calls signal->disconnect(connection)
	This means that Connection and ManagedConnection do not need to be templated, so this code is valid

		Signal<int> signal;
		ManagedConnection c = signal.connect([](int i)
		{
		std::cout << i;
		});

	instead of this code, which requires <int> for ManagedConnection

		Signal<int> signal;
		ManagedConnection<int> c = signal.connect([](int i)
		{
		std::cout << i;
		});

	Clients no longer need to care about what type signal is when storing connections, yay!
	*/

	class Disconnector
	{
	public:
		virtual ~Disconnector() noexcept = default;
		virtual bool disconnect(Connection& c) = 0;
	};
}