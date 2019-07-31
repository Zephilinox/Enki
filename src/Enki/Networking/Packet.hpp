#pragma once

//STD
#include <vector>
#include <memory>
#include <string>
#include <array>
#include <numeric>

namespace enki
{
	//enetpp is expecting this type.
	//todo: change it to std::uint8_t and fix enetpp to stop warnings
	using ClientID = unsigned int;

	enum PacketType : std::uint8_t {
		//Default type, unused by Enki
		NONE,

		//unused by Enki
		COMMAND,

		//Server sends this to a client to inform them of their ID.
		//This is sent before PacketType::CONNECTED
		CLIENT_INITIALIZED, 

		//Server sends this to a client when they connect
		//Client sends this to a server when it knows it has connected
		CONNECTED,

		//Server sends this to a client when they disconnect
		//Client sends this to the server when they disconnect
		DISCONNECTED,

		//Used by the RPCManager for free-standing, global function RPC's
		GLOBAL_RPC,

		//Used by the RPCManager and Scenegraph for Entity RPC's
		ENTITY_RPC,

		//Used by the RPCManager for member function RPC's
		CLASS_RPC,

		//Used by the Scenegraph
		//Server sends this to a client to tell them a new networked entity is created
		//Client sends this to request a new networked entity is created
		ENTITY_CREATION,

		//Used by the Scenegraph
		//Server sends this to a client when they first connect
		//One sent per entity
		ENTITY_CREATION_ON_CONNECTION,

		//Used by the Scenegraph
		//Server sends this to a client to inform them of an entity update
		//Client sends this to a server to inform it that an owned entity has updated
		ENTITY_UPDATE,

		//Used by the Scenegraph
		//Server sends this to a client to inform them that an entity has been deleted
		//Client sends this to a server to inform it that an owned entity has been deleted
		ENTITY_DELETION
	};

	//Contains the PacketType and the time the packet was sent
	//Automatically serialized when sending a packet
	struct PacketHeader
	{
		PacketType type = PacketType::NONE;
		std::uint32_t timeSent = 0;
	};

	//Contains the packet sender ID and when it was received
	//Is not serialized when sending a packet
	struct PacketInfo
	{
		ClientID senderID = 0;
		std::uint32_t timeReceived = 0;
	};

	class Packet
	{
	public:
		Packet();
		//Construct a packet from a packet header
		explicit Packet(PacketHeader);
		//Construct a packet from enet packet data
		explicit Packet(const unsigned char* data, std::size_t size);
		//Construct a packet from Enki packet data
		explicit Packet(std::byte* data, std::size_t size);

		//Write the specified number of bits to the packet's buffer
		//Bits written are based on those of data, modified by offset
		void writeBits(int data, int bits_to_write, int offset = 0);


		//Read the specified number of bits from the packet's buffer
		//Bits read can be offset within the last packet buffer byte
		//Returns an int with the read bits
		[[nodiscard]] int readBits(int bits_to_read, int offset = 0);

		//Write a float as a fixed-point integer given the known value bounds and resolution
		//Will result in data loss beyond specified resolution, but with reduced bit size
		void writeCompressedFloat(float data, float min, float max, float resolution);

		//Read a float that was written as a fixed-point integer.
		//The value bounds and resolution must match.
		[[nodiscard]] float readCompressedFloat(float min, float max, float resolution);

		//Reset the packets internal mark of how much has been deserialized
		//Used before sending the packet to some other function which may want to deserialize all the data itself
		void resetReadPosition();

		//Clear the packet of all written data
		//More performant than constructing a new packet
		void clear();

		bool isEmpty() const;
		
		//If this function is true, the specified types can be deserialized without throwing an exception
		//No guarantee on the correctness of the deserialized data, just that enough data is there to do so
		template <typename... Args>
		bool canDeserialize();

		//Set a new packet header and have it serialized automatically
		void setHeader(PacketHeader header);

		const PacketHeader& getHeader() const;
		const std::vector<std::byte>& getBytes() const;

		std::size_t getBytesWritten() const;
		std::size_t getBytesRead() const;

		Packet& operator <<(Packet& data);
		Packet& operator >>(Packet& data);

		Packet& operator <<(std::string data);
		Packet& operator >>(std::string& data);

		//Serialize and deserialize arithmetic types
		template <typename T>
		Packet& operator <<(T data);

		template <typename T>
		Packet& operator >>(T& data);

		template <typename T>
		Packet& operator <<(std::vector<T> data);

		template <typename T>
		Packet& operator >>(std::vector<T>& data);

		template <typename T, std::size_t size>
		Packet& operator <<(std::array<T, size> data);

		template <typename T, std::size_t size>
		Packet& operator >>(std::array<T, size>& data);

		//Useful when deserializing to a local variable that should be initialized with this data
		template <typename T>
		T read();

		PacketInfo info;
		std::vector<std::byte> bytes;

	private:
		template <typename T>
		void serialize(T* data, std::size_t size);

		template <typename T>
		void deserialize(T* data, std::size_t size);

		PacketHeader header;
		std::size_t bytes_written;
		std::size_t bytes_read;
		std::size_t bits_written;
		std::size_t bits_read;
	};

	//will need to adjust this in the future for endianess concerns
	//this could potentially be dangerous depending on platform-specific sizes, not sure.
	template <typename T>
	Packet& Packet::operator <<(T data)
	{
		static_assert(std::is_arithmetic_v<T>);
		serialize(&data, sizeof(T));
		return *this;
	}

	template <typename T>
	Packet& Packet::operator >>(T& data)
	{
		static_assert(std::is_arithmetic_v<T>);
		deserialize(&data, sizeof(T));
		return *this;
	}

	template <typename T>
	Packet& Packet::operator <<(std::vector<T> data)
	{
		*this << data.size();
		serialize(data.data(), sizeof(T) * data.size());
		return *this;
	}

	template <typename T>
	Packet& Packet::operator >>(std::vector<T>& data)
	{
		std::size_t size;
		*this >> size;
		data.resize(size);
		deserialize(data.data(), sizeof(T) * size);
		return *this;
	}

	template <typename T, std::size_t size>
	Packet& Packet::operator <<(std::array<T, size> data)
	{
		*this << size;
		serialize(data.data(), sizeof(T) * size);
		return *this;
	}

	template <typename T, std::size_t size>
	Packet& Packet::operator >>(std::array<T, size>& data)
	{
		std::size_t size_of_stored_array;
		*this >> size_of_stored_array;

		if (size_of_stored_array <= size)
		{
			deserialize(data.data(), size_of_stored_array);
		}
		else
		{
			throw std::runtime_error(
				"Failed to read std::array from packet, sizes don't match");
		}

		return *this;
	}

	template <typename T>
	T Packet::read()
	{
		T t;
		*this >> t;
		return t;
	}

	template <typename T>
	void Packet::serialize(T* data, std::size_t size)
	{
		static_assert(std::is_trivially_copyable_v<T>,
			"You can only serialize trivially copyable types");

		if (data == nullptr)
		{
			throw std::runtime_error(
				"Failed to serialize data into packet, data is a nullptr");
		}

		if (bytes_written + size > bytes.size())
		{
			bytes.resize((bytes_written + size) * 2);
		}

		memcpy(bytes.data() + bytes_written, data, size);
		bytes_written += size;
		bits_written = 8;
	}

	template <typename T>
	void Packet::deserialize(T* data, std::size_t size)
	{
		static_assert(std::is_trivially_copyable_v<T>,
			"You can only deserialize trivially copyable types");

		if (bytes_read + size > bytes.size())
		{
			throw std::runtime_error(
				"Failed to deserialize data in packet, "
				"would be reading past end of packet buffer");
		}
		
		if (data == nullptr)
		{
			throw std::runtime_error(
				"Failed to deserialize data in packet, data is a nullptr");
		}

		memcpy(data, bytes.data() + bytes_read, size);
		bytes_read += size;
		bits_read = 8;
	}

	template <typename... Args>
	bool Packet::canDeserialize()
	{
		//Use list intialization to fill array of n size
		//with the size of each arg through parameter pack expansion
		constexpr std::array<std::size_t, sizeof...(Args)> sizes = { sizeof(Args)... };
		int size = std::accumulate(sizes.begin(), sizes.end(), 0);

		if (bytes_read + size > bytes.size())
		{
			return false;
		}

		return true;
	}
}