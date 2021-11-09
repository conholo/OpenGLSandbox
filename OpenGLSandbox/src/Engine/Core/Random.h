#pragma once

namespace Engine
{
	class Random
	{
	public:
		static void Initialize();
		static float RandomRange(float min, float max);
	};
}