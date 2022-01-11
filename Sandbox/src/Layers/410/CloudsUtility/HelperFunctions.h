#pragma once

#include "Layers/410/CloudsUtility/CloudEnums.h"


std::vector<glm::vec4> CreateWorleyPoints(uint32_t cells, int seed);
std::vector<glm::vec4> GeneratePerlinOffsets(int octaves);

std::string NameFromUIType(CloudUIType type);
std::string NameFromWorleyChannel(WorleyChannelMask type);
glm::vec4 ColorFromMask(WorleyChannelMask mask);
