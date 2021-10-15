#include "Engine/Core/Random.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

namespace Engine
{
	void Random::Initialize()
	{
		srand(static_cast <unsigned> (time(0)));
	}

	float Random::RandomRange(float min, float max)
	{
		return min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max - min)));
	}
}