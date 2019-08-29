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
	enki::RPCManager rpcm(nullptr);
	rpcm.registerGlobalRPC(enki::RPCType::All, "test", test);

	rpcm.callGlobalRPC(test, "test", 1, 2.0, 3.0f, 4);
	rpcm.callGlobalRPCUnsafe("test", 1.5, -2.0f, true, true);

	//let's try and send a fake rpc
	//this checks the function is registered already, so it's fine
	rpcm.callGlobalRPCUnsafe("two", true);

	//let's do it manually
	enki::Packet p;
	p << std::string("two") << true;
	rpcm.receive(p);

	//now let's try a valid rpc, but one which has the wrong types
	enki::Packet p2;
	p2 << std::string("test") << true << true << true << true;
	rpcm.receive(p2);
	
	//Now let's try sending one big arg to a valid function expecting 4 smaller args
	enki::Packet p3;
	p3 << std::string("test") << 50 << 50;
	rpcm.receive(p3);

	SUBCASE("Entity")
	{
		ent e;
		rpcm.registerClassRPC(enki::RPCType::Local, "do_thing", &ent::do_thing);
		rpcm.callClassRPC(&ent::do_thing, "do_thing", &e, 1, 2);
		REQUIRE(e.i == 3);
	}
}

TEST_CASE("RPC Global Lambda")
{
	enki::RPCManager rpcm(nullptr);

	int lol = 0;

	std::function<void(int)> lambda = [&](int inc)
	{
		lol += inc;
	};

	rpcm.registerGlobalRPC(enki::RPCType::All, "lambda", &lambda);
	rpcm.callGlobalRPCUnsafe("lambda", 1);
	REQUIRE(lol == 1);
	rpcm.callGlobalRPC(&lambda, "lambda", 2);
	REQUIRE(lol == 3);
}