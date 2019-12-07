#pragma once

//SELF
#include "Enki/Hash.hpp"

namespace enki
{
class Message
{
public:
	explicit Message(HashedID id) : id(id) {}
	virtual ~Message() noexcept = default;

	const HashedID id;
};

template <HashedID Hash>
class MessageID : public Message
{
public:
	MessageID() : Message(Hash) {}
	static constexpr HashedID ID = Hash;
};
}	// namespace enki
