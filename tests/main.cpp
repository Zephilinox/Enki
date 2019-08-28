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
	context.setOption("-v", true);
	context.setOption("-c", true);
	context.setOption("-ltc", true);
	context.setOption("-lts", true);
	context.setOption("-s", false);
	context.setOption("-d", true);
	int result = context.run();
	if (context.shouldExit())
	{
		return result;
	}
#endif

	fmt::print("Press enter to start benchmark");

	std::cin.ignore();

	benchmark();

	return 0;
}