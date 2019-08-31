#pragma once

//SELF
#include "Enki/Hash.hpp"

namespace enki
{
class Message
{
public:
	virtual ~Message() noexcept = default;

	virtual HashedID getID() = 0;
};

template <HashedID Hash>
class MessageID : public Message
{
public:
	HashedID getID() final
	{
		return ID;
	}

	static constexpr HashedID ID = Hash;
};
}	// namespace enki
