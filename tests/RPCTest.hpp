#pragma once

#include <Enki/Networking/RPC.hpp>
#include <Enki/Networking/RPCManager.hpp>

struct ent
{
	void do_thing(int x, int y)
	{
		i = x + y;
	}

	int i = 0;
};

void test(int i, double d, float s, int ii)
{
	std::cout << "called function test(" << i << ", " << d << ", " << s << ", " << ii << ");\n";
}

TEST_CASE("RPC")
{
	enki::RPCManager rpcm;
	rpcm.add(enki::RPCType::All, "test", test);

	std::cout << "\n";

	rpcm.call(test, "test", 1, 2.0, 3.0f, 4);

	std::cout << "\n";

	rpcm.callUnsafe("test", 1.5, -2.0f, true, true);

	std::cout << "\n";

	//let's try and send a fake rpc
	rpcm.callUnsafe("two", true);
	//this checks the function is registered already, so it's fine

	std::cout << "\n";

	//let's do it manually
	enki::Packet p;
	p << std::string("two") << true;
	rpcm.receive(p);
	//this makes the client throw an exception, so we need to capture that exception rather than crash the game, now fixed

	std::cout << "\n";

	//now let's try a valid rpc, but one which has the wrong types
	enki::Packet p2;
	p2 << std::string("test") << true << true << true << true;
	rpcm.receive(p2);
	//this also causes an exception to be thrown and the client to crash, now fixed

	std::cout << "\n";

	//Now let's try sending one big arg to a valid function expecting 4 smaller args
	enki::Packet p3;
	p3 << std::string("test") << 50 << 50;
	rpcm.receive(p3);

	SUBCASE("Entity")
	{
		//ent e;
		//rpcm.add("do_thing", &ent::do_thing);
		//todo: update RPCManager and make it clear what's supported and how
		//registering member functions is doable alongside Entity derived member functions, some of the code is there, just need to make it happen

		//rpcm.call(&ent::do_thing, "do_thing", &e, 1, 2);
		//std::cout << e.i << "\n";
		//REQUIRE(e.i == 3);
	}
}