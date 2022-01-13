#include <string>
#include <vector>
#include "Engine/Core//Random.h"
#include <glm/glm.hpp>

#include "Layers/410/CloudsUtility/HelperFunctions.h"


std::vector<glm::vec4> CreateWorleyPoints(uint32_t cells, int seed)
{
	std::vector<glm::vec4> points;
	points.resize(cells * cells * cells);

	Engine::Random::Seed(seed);
	float cellSize = 1.0f / cells;

	for (uint32_t x = 0; x < cells; x++)
	{
		for (uint32_t y = 0; y < cells; y++)
		{
			for (uint32_t z = 0; z < cells; z++)
			{
				float randomX = Engine::Random::RandomRange(0, 1);
				float randomY = Engine::Random::RandomRange(0, 1);
				float randomZ = Engine::Random::RandomRange(0, 1);

				glm::vec3 cellCorner = glm::vec3(x, y, z) * cellSize;
				glm::vec3 offset = glm::vec3(randomX, randomY, randomZ) * cellSize;

				uint32_t index = x + cells * (y + z * cells);
				points[index] = glm::vec4(cellCorner + offset, 0.0f);
			}
		}
	}

	return points;
}

std::vector<glm::vec4> GeneratePerlinOffsets(int octaves)
{
	std::vector<glm::vec4> result;
	result.resize(octaves);

	for (int i = 0; i < octaves; i++)
	{
		glm::vec4 random = { Engine::Random::RandomRange(0, 1), Engine::Random::RandomRange(0, 1), Engine::Random::RandomRange(0, 1), Engine::Random::RandomRange(0, 1) };
		result[i] = (random * glm::vec4(2.0) - glm::vec4(1.0)) * glm::vec4(1000.0);
	}

	return result;
}

std::string NameFromUIType(CloudUIType type)
{
	switch (type)
	{
	case CloudUIType::BaseShape:		return "Base Shape";
	case CloudUIType::DetailShape:		return "Detail Noise";
	case CloudUIType::Perlin:			return "Perlin";
	case CloudUIType::Curl:				return "Curl";
	}

	return "";
}

std::string NameFromWorleyChannel(WorleyChannelMask type)
{
	switch (type)
	{
	case WorleyChannelMask::R:	return "R";
	case WorleyChannelMask::G:	return "G";
	case WorleyChannelMask::B:	return "B";
	case WorleyChannelMask::A:	return "A";
	}

	return "";
}

glm::vec4 ColorFromMask(WorleyChannelMask mask)
{
	switch (mask)
	{
	case WorleyChannelMask::R: return { 1.0f, 0.0f, 0.0f, 0.0f };
	case WorleyChannelMask::G: return { 0.0f, 1.0f, 0.0f, 0.0f };
	case WorleyChannelMask::B: return { 0.0f, 0.0f, 1.0f, 0.0f };
	case WorleyChannelMask::A: return { 0.0f, 0.0f, 0.0f, 1.0f };
	}

	return { 0.0f, 0.0f, 0.0f, 0.0f };
}
