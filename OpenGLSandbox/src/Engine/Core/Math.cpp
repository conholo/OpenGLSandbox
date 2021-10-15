#include "Engine/Core/Math.h"


namespace Engine
{
	float EaseIn(float t)
	{
		return t * t;
	}

	float Spike(float t)
	{
		return t <= 0.5f ? EaseIn(t / 0.5f) : t;
	}

	float Clamp(float value, float min, float max)
	{
		if (value > max)
			value = max;
		if (value < min)
			value = min;

		return value;
	}

	float Lerp(float start, float end, float t)
	{
		t = Clamp(t, 0, 1);
		return start + t * (end - start);
	}

	float InverseLerp(float a, float b, float t)
	{
		return Clamp((t - a) / (b - a), 0, 1); // Clamp01 makes sure the result is between [0, 1]
	}
}