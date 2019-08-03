#pragma once

//STD
#include <random>

//LIBS
#include <pcg/pcg_random.hpp>

namespace enki
{
	class RNG
	{
	public:
		RNG();

		void setSeed(int s);

		[[nodiscard]]
		int getSeed() const;

		[[nodiscard]]
		int generateInt(int lower, int upper);

		[[nodiscard]]
		float generateFloat(float lower, float upper);

		template <typename Return, typename Distribution>
		[[nodiscard]]
		Return generateFromDistribution(Distribution random);

	private:
		int seed = 0;
		pcg32 rng;
	};

	template <typename Return, typename Distribution>
	Return RNG::generateFromDistribution(Distribution random)
	{
		return random(rng);
	}
}