#include "RNG.hpp"

//STD
#include <chrono>

namespace enki
{
	RNG::RNG()
	{
		seed = (int)std::chrono::system_clock::now().time_since_epoch().count();
	}

	void RNG::setSeed(int s)
	{
		seed = s;
		rng.seed(seed);
	}

	int RNG::getSeed()
	{
		return seed;
	}

	int RNG::generateInt(int lower, int upper)
	{
		std::uniform_int_distribution<int> random(lower, upper);
		return random(rng);
	}

	float RNG::generateFloat(float lower, float upper)
	{
		std::uniform_real_distribution<> random(lower, upper);
		return random(rng);
	}
}