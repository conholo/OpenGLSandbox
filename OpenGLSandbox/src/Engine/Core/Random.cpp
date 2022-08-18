#include "epch.h"
#include "Engine/Core/Random.h"

namespace Engine
{
	void Random::Seed(int seed)
	{
		srand(static_cast <unsigned> (seed));
	}

	void Random::Initialize()
	{
		srand(static_cast <unsigned> (time(0)));
	}

	float Random::RandomRange(float min, float max)
	{
		return min + static_cast <float> (rand()) / (RAND_MAX / (max - min));
	}
}