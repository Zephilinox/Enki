#include "Packet.hpp"

//STD
#include <cstring>
#include <cmath>
#include <algorithm>

namespace enki
{
	Packet::Packet()
		: bytes(sizeof(PacketHeader))
		, bytes_written(sizeof(PacketHeader))
		, bytes_read(sizeof(PacketHeader))
		, bits_written(8)
		, bits_read(8)
	{
		bytes.reserve(1400);
		memcpy(bytes.data(), &header, sizeof(PacketHeader));
	}

	Packet::Packet(PacketHeader p_header)
		: header(p_header)
		, bytes(sizeof(PacketHeader))
		, bytes_written(sizeof(PacketHeader))
		, bytes_read(sizeof(PacketHeader))
		, bits_written(8)
		, bits_read(8)
	{
		bytes.reserve(1400);
		memcpy(bytes.data(), &header, sizeof(PacketHeader));
	}

	Packet::Packet(const unsigned char* data, std::size_t size)
		: bytes(size)
		, bytes_written(size)
		, bytes_read(sizeof(PacketHeader))
		, bits_written(8)
		, bits_read(8)
	{
		bytes.reserve(1400);
		memcpy(bytes.data(), data, size);
		memcpy(&header, bytes.data(), sizeof(PacketHeader));
	}

	Packet::Packet(std::byte* data, std::size_t size)
		: bytes(size)
		, bytes_written(size)
		, bytes_read(sizeof(PacketHeader))
		, bits_written(8)
		, bits_read(8)
	{
		bytes.reserve(1400);
		memcpy(bytes.data(), data, size);
		memcpy(&header, bytes.data(), sizeof(PacketHeader));
	}

	void Packet::writeBits(int data, int bits_to_write, int offset)
	{
		//ensure our bit/byte count is up to date
		if (bits_written >= 8)
		{
			bytes_written += bits_written / 8;
			bits_written = bits_written % 8;
		}

		if (sizeof(data) * 8 < bits_to_write + offset)
		{
			throw std::runtime_error(
				"Writing these bits with this offset "
				"would cause an overflow of the data passed in");
		}

		//calculate how much space we need and then resize the buffer
		//to accomodate it
		int bits_available = 8 - bits_written;
		int bits_needed = bits_to_write - bits_available;
		int bytes_needed = static_cast<int>(std::ceil(static_cast<float>(bits_needed) / 8.0f));

		if (bytes_written + bytes_needed > bytes.size())
		{
			bytes.resize(bytes_written + bytes_needed);
		}

		const auto write_bits = [&](int bits, int extra_offset)
		{
			//maybe there is a better way to do it than a loop
			for (int i = 0; i < bits; ++i)
			{
				//sorry for ugly casts, blame GCC and Clang
				int shift = 1 << (i + offset + extra_offset);
				if (data & (shift))
				{
					bytes.data()[bytes_written - 1] = static_cast<std::byte>(static_cast<unsigned char>(bytes.data()[bytes_written - 1]) | static_cast<unsigned char>((1 << (i + bits_written))));
				}
				else
				{
					bytes.data()[bytes_written - 1] = static_cast<std::byte>(static_cast<unsigned char>(bytes.data()[bytes_written - 1]) & static_cast<unsigned char>(~(1 << (i + bits_written))));
				}
			}
		};

		/*This seems a bit messy, there's probably a better way
		We keep track of how many bits we have left to write
		and how many bits are available

		If there isn't a full byte available in the buffer
		because we've got a half-written byte right now
		then write as many bits as we can to fill it

		If the number of bits left is a byte or more
		then write a byte at a time

		If the number of bits left is less than a byte
		then write all the remaining bits in to our last byte of free space
		*/
		int bits_left = bits_to_write;
		while (bits_left > 0)
		{
			if (bits_available < 8)
			{
				write_bits(bits_available, 0);
				bytes_written++;
				bits_written = 0;
				bits_left -= bits_available;
				bits_available = 8;
			}
			else if (bits_left / 8 >= 1)
			{
				write_bits(8, bits_to_write - bits_left);
				bytes_written++;
				bits_written = 0;
				bits_left -= 8;
			}
			else
			{
				write_bits(bits_left % 8, bits_to_write - bits_left);
				bits_written += bits_left % 8;
				bits_left = 0;
			}
		}
	}

	int Packet::readBits(int bits_to_read, int offset)
	{
		int data = 0;

		if (bits_read >= 8)
		{
			bytes_read += bits_read / 8;
			bits_read = bits_read % 8;
		}

		int bits_available = 8 - bits_read;
		int bits_needed = bits_to_read - bits_available;
		int bytes_needed = static_cast<int>(std::ceil(static_cast<float>(bits_needed) / 8.0f));

		if (sizeof(data) * 8 < bits_to_read + offset)
		{
			throw std::runtime_error(
				"Reading these bits with this offset "
				"would cause an overflow of the return value");
		}

		if (bytes_read + bytes_needed - 1 > bytes.size())
		{
			throw std::runtime_error(
				"Tried to read past the packet buffer, "
				"not enough bytes written");
		}

		const auto read_bits = [&](int bits, int extra_offset)
		{
			for (int i = 0; i < bits; ++i)
			{
				//If this bit is 1
				int byte = static_cast<int>(bytes.data()[bytes_read - 1]);
				int shift = 1 << (bits_read + i);
				if ((byte) & (shift))
				{
					//set the other numbers bit to 1
					data |= (1 << (i + offset + extra_offset));
				}
				else
				{
					//otherwise set it to 0
					data &= ~(1 << (i + offset + extra_offset));
				}
			}
		};

		int bits_left = bits_to_read;
		while (bits_left > 0)
		{
			if (bits_available < 8)
			{
				read_bits(bits_available, 0);
				bytes_read++;
				bits_read = 0;
				bits_left -= bits_available;
				bits_available = 8;
			}
			else if (bits_left / 8 >= 1)
			{
				read_bits(8, bits_to_read - bits_left);
				bytes_read++;
				bits_read = 0;
				bits_left -= 8;
			}
			else
			{
				read_bits(bits_left % 8, bits_to_read - bits_left);
				bits_read += bits_left % 8;
				bits_left = 0;
			}
		}

		return data;
	}

	void Packet::writeCompressedFloat(float data, float min, float max, float resolution)
	{
		float delta = max - min;
		float total_possible_values = delta / resolution;
		int max_possible_values = static_cast<int>(std::ceil(total_possible_values));
		int bits_required = static_cast<int>(std::ceil(std::log(max_possible_values) / std::log(2)));

		float normalized_data = std::clamp((data - min) / delta, 0.0f, 1.0f);
		int final_value = static_cast<int>(std::round(normalized_data * max_possible_values));

		writeBits(final_value, bits_required);
	}

	float Packet::readCompressedFloat(float min, float max, float resolution)
	{
		float delta = max - min;
		float total_possible_values = delta / resolution;
		int max_possible_values = static_cast<int>(std::ceil(total_possible_values));
		int bits_required = static_cast<int>(std::ceil(std::log(max_possible_values) / std::log(2)));

		int final_value = readBits(bits_required);
		float normalized_data = static_cast<float>(final_value) / static_cast<float>(max_possible_values);

		return normalized_data * delta + min;
	}

	void Packet::resetReadPosition()
	{
		bytes_read = sizeof(PacketHeader);
		bits_read = 8;
	}

	void Packet::clear()
	{
		bytes.clear();
		memcpy(bytes.data(), &header, sizeof(PacketHeader));
		bytes_read = sizeof(PacketHeader);
		bits_read = 8;
		bytes_written = sizeof(PacketHeader);
		bits_written = 8;
	}

	bool Packet::isEmpty() const
	{
		return bytes.size() == sizeof(PacketHeader);
	}

	void Packet::setHeader(PacketHeader p_header)
	{
		header = p_header;
		memcpy(bytes.data(), &header, sizeof(PacketHeader));
	}

	const PacketHeader& Packet::getHeader() const
	{
		return header;
	}

	const std::vector<std::byte>& Packet::getBytes() const
	{
		return bytes;
	}

	std::size_t Packet::getBytesWritten() const
	{
		return bytes_written;
	}

	std::size_t Packet::getBytesRead() const
	{
		return bytes_read;
	}

	Packet& Packet::operator<<(Packet& data)
	{
		*this << data.bytes;
		return *this;
	}

	Packet& Packet::operator>>(Packet& data)
	{
		std::vector<std::byte> b;
		*this >> b;
		data = Packet(b.data(), b.size());
		return *this;
	}

	Packet& Packet::operator <<(std::string data)
	{
		*this << data.length();
		serialize(data.data(), data.length());
		return *this;
	}

	Packet& Packet::operator >>(std::string& data)
	{
		std::size_t length;
		*this >> length;
		data.resize(length);
		deserialize(data.data(), length);
		return *this;
	}
}