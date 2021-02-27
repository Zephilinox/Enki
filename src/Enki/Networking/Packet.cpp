#include "Packet.hpp"

//STD
#include <algorithm>
#include <cmath>
#include <cstring>
#include <stdexcept>

namespace enki
{
Packet::Packet()
	: bytes(sizeof(PacketHeader))
	, m_bytes_written(sizeof(PacketHeader))
	, m_bytes_read(sizeof(PacketHeader))
	, m_bits_written(8)
	, m_bits_read(8)
{
	bytes.reserve(1400);
	memcpy(bytes.data(), &m_header, sizeof(PacketHeader));
}

Packet::Packet(PacketHeader p_header)
	: bytes(sizeof(PacketHeader))
	, m_header(p_header)
	, m_bytes_written(sizeof(PacketHeader))
	, m_bytes_read(sizeof(PacketHeader))
	, m_bits_written(8)
	, m_bits_read(8)
{
	bytes.reserve(1400);
	memcpy(bytes.data(), &m_header, sizeof(PacketHeader));
}

Packet::Packet(const unsigned char* data, std::size_t size)
	: bytes(size)
	, m_bytes_written(size)
	, m_bytes_read(sizeof(PacketHeader))
	, m_bits_written(8)
	, m_bits_read(8)
{
	bytes.reserve(1400);
	memcpy(bytes.data(), data, size);
	memcpy(&m_header, bytes.data(), sizeof(PacketHeader));
}

Packet::Packet(std::byte* data, std::size_t size)
	: bytes(size)
	, m_bytes_written(size)
	, m_bytes_read(sizeof(PacketHeader))
	, m_bits_written(8)
	, m_bits_read(8)
{
	bytes.reserve(1400);
	memcpy(bytes.data(), data, size);
	memcpy(&m_header, bytes.data(), sizeof(PacketHeader));
}

void Packet::writeBits(int data, int bits_to_write, int offset)
{
	//ensure our bit/byte count is up to date
	if (m_bits_written >= 8)
	{
		m_bytes_written += m_bits_written / 8;
		m_bits_written = m_bits_written % 8;
	}

	if (static_cast<int>(sizeof(data)) * 8 < bits_to_write + offset)
	{
		throw std::runtime_error(
			"Writing these bits with this offset "
			"would cause an overflow of the data passed in");
	}

	//calculate how much space we need and then resize the buffer
	//to accomodate it
	int bits_available = 8 - static_cast<int>(m_bits_written);
	int bits_needed = bits_to_write - bits_available;
	int bytes_needed = static_cast<int>(std::ceil(static_cast<float>(bits_needed) / 8.0f));

	if (m_bytes_written + bytes_needed > bytes.size())
		bytes.resize(m_bytes_written + bytes_needed);

	const auto write_bits = [&](int bits, int extra_offset) {
		//maybe there is a better way to do it than a loop
		for (int i = 0; i < bits; ++i)
		{
			//sorry for ugly casts, blame GCC and Clang
			const int shift = 1 << (i + offset + extra_offset);
			if (data & (shift))
			{
				bytes.data()[m_bytes_written - 1] = static_cast<std::byte>(static_cast<unsigned char>(bytes.data()[m_bytes_written - 1]) | static_cast<unsigned char>((1 << (i + m_bits_written))));
			}
			else
			{
				bytes.data()[m_bytes_written - 1] = static_cast<std::byte>(static_cast<unsigned char>(bytes.data()[m_bytes_written - 1]) & static_cast<unsigned char>(~(1 << (i + m_bits_written))));
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
			m_bytes_written++;
			m_bits_written = 0;
			bits_left -= bits_available;
			bits_available = 8;
		}
		else if (bits_left / 8 >= 1)
		{
			write_bits(8, bits_to_write - bits_left);
			m_bytes_written++;
			m_bits_written = 0;
			bits_left -= 8;
		}
		else
		{
			write_bits(bits_left % 8, bits_to_write - bits_left);
			m_bits_written += bits_left % 8;
			bits_left = 0;
		}
	}
}

int Packet::readBits(int bits_to_read, int offset)
{
	int data = 0;

	if (m_bits_read >= 8)
	{
		m_bytes_read += m_bits_read / 8;
		m_bits_read = m_bits_read % 8;
	}

	int bits_available = 8 - static_cast<int>(m_bits_read);
	const int bits_needed = bits_to_read - bits_available;
	const int bytes_needed = static_cast<int>(std::ceil(static_cast<float>(bits_needed) / 8.0f));

	if (static_cast<int>(sizeof(data)) * 8 < bits_to_read + offset)
	{
		throw std::runtime_error(
			"Reading these bits with this offset "
			"would cause an overflow of the return value");
	}

	if (m_bytes_read + bytes_needed - 1 > bytes.size())
	{
		throw std::runtime_error(
			"Tried to read past the packet buffer, "
			"not enough bytes written");
	}

	const auto read_bits = [&](int bits, int extra_offset) {
		for (int i = 0; i < bits; ++i)
		{
			//If this bit is 1
			const int byte = static_cast<int>(bytes.data()[m_bytes_read - 1]);
			const int shift = 1 << (m_bits_read + i);
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
			m_bytes_read++;
			m_bits_read = 0;
			bits_left -= bits_available;
			bits_available = 8;
		}
		else if (bits_left / 8 >= 1)
		{
			read_bits(8, bits_to_read - bits_left);
			m_bytes_read++;
			m_bits_read = 0;
			bits_left -= 8;
		}
		else
		{
			read_bits(bits_left % 8, bits_to_read - bits_left);
			m_bits_read += bits_left % 8;
			bits_left = 0;
		}
	}

	return data;
}

void Packet::writeCompressedFloat(float data, float min, float max, float resolution)
{
	//todo: how do I want to handle div by 0? resolution of 0 is invalid (should be 10, 1, 0.1, 0.01, etc)

	const float delta = max - min;
	const float total_possible_values = delta / resolution;
	const int max_possible_values = static_cast<int>(std::ceil(total_possible_values));
	const int bits_required = static_cast<int>(std::ceil(std::log(max_possible_values) / std::log(2)));

	const float normalized_data = std::clamp((data - min) / delta, 0.0f, 1.0f);
	const int final_value = static_cast<int>(std::round(normalized_data * static_cast<float>(max_possible_values)));

	writeBits(final_value, bits_required);
}

float Packet::readCompressedFloat(float min, float max, float resolution)
{
	const float delta = max - min;
	const float total_possible_values = delta / resolution;
	const int max_possible_values = static_cast<int>(std::ceil(total_possible_values));
	const int bits_required = static_cast<int>(std::ceil(std::log(max_possible_values) / std::log(2)));

	const int final_value = readBits(bits_required);
	const float normalized_data = static_cast<float>(final_value) / static_cast<float>(max_possible_values);

	return normalized_data * delta + min;
}

void Packet::resetReadPosition()
{
	m_bytes_read = sizeof(PacketHeader);
	m_bits_read = 8;
}

void Packet::resetWritePosition()
{
	m_bytes_written = sizeof(PacketHeader);
	m_bits_written = 8;
}

void Packet::clear()
{
	bytes.clear();
	bytes.resize(sizeof(PacketHeader));
	memcpy(bytes.data(), &m_header, sizeof(PacketHeader));
	m_bytes_read = sizeof(PacketHeader);
	m_bits_read = 8;
	m_bytes_written = sizeof(PacketHeader);
	m_bits_written = 8;
}

bool Packet::isEmpty() const
{
	return bytes.size() == sizeof(PacketHeader);
}

void Packet::setHeader(PacketHeader p_header)
{
	m_header = p_header;
	memcpy(bytes.data(), &m_header, sizeof(PacketHeader));
}

const PacketHeader& Packet::getHeader() const
{
	return m_header;
}

const std::vector<std::byte>& Packet::getBytes() const
{
	return bytes;
}

std::size_t Packet::getBytesWritten() const
{
	return m_bytes_written;
}

std::size_t Packet::getBytesRead() const
{
	return m_bytes_read;
}

Packet& Packet::operator<<(Packet data)
{
	*this << std::move(data.bytes);
	return *this;
}

Packet& Packet::operator>>(Packet& data)
{
	std::vector<std::byte> b;
	*this >> b;
	data = Packet(b.data(), b.size());
	return *this;
}

Packet& Packet::operator<<(std::string data)
{
	*this << data.length();
	serialize(data.data(), data.length());
	return *this;
}

Packet& Packet::operator>>(std::string& data)
{
	std::size_t length;
	*this >> length;
	data.resize(length);
	deserialize(data.data(), length);
	return *this;
}


Packet& Packet::operator<<(std::string_view data)
{
	*this << data.length();
	serialize(data.data(), data.length());
	return *this;
}

Packet& Packet::operator>>(std::string_view& data)
{
	std::size_t length;
	*this >> length;

	if (m_bytes_read + length > bytes.size())
	{
		throw std::runtime_error(
			"Failed to deserialize data in packet, "
			"would be reading past end of packet buffer");
	}

	char* ptr = reinterpret_cast<char*>(bytes.data()) + m_bytes_read;
	m_bytes_read += length;
	m_bits_read = 8;

	data = std::string_view(ptr, length);
	
	return *this;
}
}	// namespace enki