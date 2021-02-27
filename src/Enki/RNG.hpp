#pragma once

#ifdef WIN32
#	undef min
#	undef max
#endif

//LIBS
#include <pcg/pcg_random.hpp>

//STD
#include <random>

namespace enki
{
class RNG
{
public:
	RNG();

	void setSeed(int s);

	[[nodiscard]] int getSeed() const;
	[[nodiscard]] int generateInt(int lower, int upper);
	[[nodiscard]] float generateFloat(float lower, float upper);
	[[nodiscard]] double generateDouble(double lower, double upper);

	template <typename Return, typename Distribution>
	[[nodiscard]] Return generateFromDistribution(Distribution random);

private:
	int seed = 0;
	pcg32 rng;
};

template <typename Return, typename Distribution>
Return RNG::generateFromDistribution(Distribution random)
{
	return random(rng);
}
}	// namespace enki