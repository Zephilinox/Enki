#pragma once

//STD
#include <functional>
#include <memory>

namespace enki
{
template <typename... Args>
class Signal;

class Disconnector;

/*
Connection is returned by signal.connect()
you use it to control disconnecting your function from the signal
*/
class Connection
{
public:
	Connection() noexcept = default;
	Connection(const Connection& c) = default;

	//Not Required?
	//Connection(Connection&& c) = default;

	virtual ~Connection() noexcept = default;

	//Not Required?
	//Connection& operator=(const Connection& c) = default;

	//Required
	Connection& operator=(Connection&& c) noexcept = default;

	//Check to see if the Connection is still valid
	operator bool() const noexcept;

	//Ensures both Connections are connected to the same Signal and referring to the same function
	//Might return false if both connections are invalid, depending on what made them invalid.
	friend bool operator==(const Connection& lhs, const Connection& rhs) noexcept
	{
		return lhs.slot_id == rhs.slot_id && lhs.dc.lock().get() == rhs.dc.lock().get();
	}

	//Returns true if disconnection was successful
	//False if it was not, or if it's already disconnected
	bool disconnect();

private:
	//Signal needs to be able to create Connection with specific params
	//Clients should not be able to, as it means they can do random shit to a signal
	//Note: this means that a Signal<int> is a friend of Connection<bool>
	//But this shouldn't be a problem, since it's created based on the Signal <Args...>
	template <typename...>
	friend class Signal;

	//Only meant to be accessed by Signal
	Connection(std::weak_ptr<Disconnector> dc, unsigned id) noexcept;

	std::weak_ptr<Disconnector> dc;
	unsigned slot_id = 0;
};

/*
Just a wrapper around Connection
Automatically disconnects on destruction
RAII
*/
class ManagedConnection : public Connection
{
public:
	ManagedConnection() noexcept = default;
	ManagedConnection(const Connection& c);

	//Possibly useful
	ManagedConnection(const ManagedConnection&) = default;

	//Not Required?
	//ManagedConnection(ManagedConnection&& c) noexcept = default;

	~ManagedConnection() noexcept final;

	//Not Required?
	//ManagedConnection& operator=(const ManagedConnection& c) = default;

	//Required, hides connection operator=? important?
	ManagedConnection& operator=(ManagedConnection&& c) noexcept;
};
}	// namespace enki