#include "Connection.hpp"

//SELF
#include "Disconnector.hpp"

namespace enki
{
	Connection::Connection(std::weak_ptr<Disconnector> dc, unsigned id) noexcept
		: dc(std::move(dc))
		, slot_id(id)
	{}

	Connection::operator bool() const noexcept
	{
		return slot_id != 0 && !dc.expired();
	}

	bool Connection::disconnect()
	{
		auto disconnector = dc.lock();
		if (disconnector)
		{
			return disconnector->disconnect(*this);
		}

		return false;
	}

	ManagedConnection::ManagedConnection(Connection c)
		: Connection(std::move(c))
	{}

	ManagedConnection::~ManagedConnection()
	{
		disconnect();
	}

	ManagedConnection& ManagedConnection::operator=(ManagedConnection&& c) noexcept
	{
		//Don't disconnect if we're moving ourselfs in to ourselves.
		if (*this == c)
		{
			return *this;
		}

		//Disconnect whatever connection we may currently be managing
		disconnect();

		//Call base move assignment operator
		Connection::operator=(std::move(c));
		return *this;
	}
}