#pragma once


namespace Engine
{
	float EaseIn(float t);
	float Spike(float t);
	float Clamp(float value, float min, float max);
	float Lerp(float start, float end, float t);
	float InverseLerp(float a, float b, float t);
}