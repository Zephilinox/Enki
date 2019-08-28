#pragma once

#include <Enki/Networking/Packet.hpp>

struct vector2
{
	float x = 0;
	float y = 0;

	friend enki::Packet& operator <<(enki::Packet& p, const vector2& v);
	friend enki::Packet& operator >>(enki::Packet& p, vector2& v);
};

enki::Packet& operator <<(enki::Packet& p, const vector2& v)
{
	return p << v.x << v.y;
}

enki::Packet& operator >>(enki::Packet& p, vector2& v)
{
	return p >> v.x >> v.y;
}

TEST_CASE("Packet")
{
	SUBCASE("Strings")
	{
		enki::Packet p;
		std::string s = "hey";
		p << s;
		auto s2 = p.read<std::string>();
		CHECK(s2 == s);
		p.resetReadPosition();
		p >> s2;
		CHECK(s2 == s);
	}

	SUBCASE("Custom Class")
	{
		enki::Packet p;
		vector2 v{ 1, 3 };
		p << v;
		vector2 v2;
		p >> v2;
		CHECK(v2.x == v.x);
		CHECK(v2.y == v.y);
	}

	SUBCASE("Vector")
	{
		enki::Packet p;
		std::vector<int> vecs{ 1, 2 };
		p << vecs;
		std::vector<int> vecs2{};
		p >> vecs2;
		CHECK(vecs2 == vecs);
	}

	SUBCASE("Array")
	{
		enki::Packet p;
		std::array<bool, 10> bools = { true, true, false, true, true, true, true, true, true, true };
		p << bools;
		std::array<bool, 10> bools2;
		p >> bools2;
		CHECK(bools2 == bools);
	}

	SUBCASE("Header")
	{
		enki::Packet p;
		int oldtype = p.getHeader().type;
		int oldtype_bytes = static_cast<int>(p.getBytes().data()[0]);
		CHECK(static_cast<int>(oldtype) == oldtype_bytes);

		p.setHeader({ enki::PacketType::GLOBAL_RPC });

		int type = p.getHeader().type;
		int type_bytes = static_cast<int>(p.getBytes().data()[0]);
		CHECK(static_cast<int>(type) == type_bytes);
	}

	SUBCASE("Write Bits")
	{
		//todo: remove new enki::Packet to check for accurate byte boundary changes
		enki::Packet p2;
		int num = 0b00001111;
		p2.writeBits(num, 8);
		int resulting_num = static_cast<int>(p2.getBytes().data()[sizeof(enki::PacketHeader)]);
		CHECK(num == resulting_num);

		int num2 = 0b01001100;
		p2.writeBits(num2, 8);
		int resulting_num2 = static_cast<int>(p2.getBytes().data()[sizeof(enki::PacketHeader) + 1]);
		CHECK(num2 == resulting_num2);

		int num3 = 0b00001111;
		p2.writeBits(num3, 4);
		int resulting_num3 = static_cast<int>(p2.getBytes().data()[sizeof(enki::PacketHeader) + 2]);
		CHECK(0b00001111 == resulting_num3);

		int num4 = 0b11110000;
		p2.writeBits(num4, 4, 4);
		int resulting_num4 = static_cast<int>(p2.getBytes().data()[sizeof(enki::PacketHeader) + 2]);
		CHECK(0b11111111 == resulting_num4);
	}

	SUBCASE("Read Bits")
	{
		enki::Packet p2;
		p2 << 0b00001111;
		int input = static_cast<int>(p2.getBytes().data()[sizeof(enki::PacketHeader)]);
		int output = p2.readBits(8);
		CHECK(input == output);

		p2 << 0b11110000;
		int input2 = static_cast<int>(p2.getBytes().data()[sizeof(enki::PacketHeader) + 1]);
		int output2 = p2.readBits(8);
		CHECK(input2 == output2);

		enki::Packet p3;
		int num = 0b11111111;
		p3.writeBits(num, 8);
		int output3 = p3.readBits(4);
		int output4 = p3.readBits(4, 4);
		
		CHECK(output3 == 0b00001111);
		CHECK(output4 == 0b11110000);
		CHECK(output3 + output4 == num);
	}

	SUBCASE("Write Bits Overflowing Bytes")
	{
		enki::Packet p;
		try
		{
			int num = 0b000001111;
			p.writeBits(num, 4);
			p.writeBits(num, 4);

			p.writeBits(num, 6);
			p.writeBits(num, 4);

			p.writeBits(num, 6);
		}
		catch (...)
		{

		}

		CHECK(p.getBytes().size() == sizeof(enki::PacketHeader) + 3);
		CHECK(static_cast<int>(p.getBytes()[sizeof(enki::PacketHeader)]) == 0b11111111);
		CHECK(static_cast<int>(p.getBytes()[sizeof(enki::PacketHeader) + 1]) == 0b11001111);
		CHECK(static_cast<int>(p.getBytes()[sizeof(enki::PacketHeader) + 2]) == 0b00111111);
	}

	SUBCASE("Read Bits Overflowing Bytes")
	{
		enki::Packet p;
		p << 1000000;
		int num = p.readBits(32);
		CHECK(num == 1000000);
	}

	SUBCASE("Compressed Range")
	{
		enki::Packet p;
		float f1 = 0.5f;
		p.writeCompressedFloat(f1, 0, 1, 0.01f);
		float f2 = p.readCompressedFloat(0, 1, 0.01f);
		CHECK(f1 == f2);

		//will fail due to not supporting > 8 bits on read/write
		/*enki::Packet p2;
		float f3 = 5;
		p2.writeCompressedFloat(f3, -10, 10, 0.01f);
		float f4;
		p2.readCompressedFloat(f4, -10, 10, 0.01f);
		CHECK(f3 == f4);

		enki::Packet p3;
		float f5 = 1.0f / 60.0f;
		p3.writeCompressedFloat(f5, 0, 1, 0.001f);
		float f6;
		p3.readCompressedFloat(f6, 0, 1, 0.001f);
		CHECK(f5 == f6);*/
	}

	SUBCASE("Real Test")
	{
		enki::Packet p({ enki::PacketType::ENTITY_UPDATE });
		std::array<float, 2> position = { 300, 400 };
		float rotation = 27.27f;
		bool isplayer = true;
		int isplayer_int = static_cast<int>(isplayer);

		p.writeCompressedFloat(position[0], 0, 1280, 0.01f);
		p.writeCompressedFloat(position[1], 0, 720, 0.01f);
		p.writeCompressedFloat(rotation, 0, 360, 0.01f);
		p.writeBits(isplayer_int, 1); //todo: write bits not just for ints

		std::array<float, 2> received_pos;
		
		received_pos[0] = p.readCompressedFloat(0, 1280, 0.01f);
		received_pos[1] = p.readCompressedFloat(0, 720, 0.01f);
		float received_rot = p.readCompressedFloat(0, 360, 0.01f);
		int received_isplayer = p.readBits(1);
		bool actual_isplayer = actual_isplayer = static_cast<bool>(received_isplayer);

		CHECK(p.getBytes().size() < sizeof(enki::PacketHeader) + sizeof(position) + sizeof(rotation) + sizeof(isplayer));
		std::cout << p.getBytes().size() << " < " << sizeof(enki::PacketHeader) + sizeof(position) + sizeof(rotation) + sizeof(isplayer) << ", yay bytes saved!\n";
		CHECK(position[0] == doctest::Approx(received_pos[0]));
		CHECK(position[1] == doctest::Approx(received_pos[1]));
		CHECK(rotation == received_rot);
		CHECK(isplayer == actual_isplayer);
	}
}