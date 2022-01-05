#pragma once

namespace Engine
{
	class Random
	{
	public:
		static void Seed(int seed);
		static void Initialize();
		static float RandomRange(float min, float max);
	};
}