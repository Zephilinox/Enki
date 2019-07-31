//LIBS
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest.h>

//TESTS
#include "PacketTest.hpp"
#include "RPCTest.hpp"
#include "Benchmark.hpp"

int main(int argc, char** argv)
{
#if _DEBUG
	doctest::Context context;
	context.setOption("abort-after", 5);
	context.applyCommandLine(argc, argv);
	context.setOption("no-breaks", true);
	int result = context.run();
	if (context.shouldExit())
	{
		return result;
	}
#endif
	
	benchmark();

	return 0;
}