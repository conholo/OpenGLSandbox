#type compute
#version 450 core

layout(binding = 0, rgba32f) uniform image3D o_Image;

layout(std140, binding = 0) buffer PointsBufferA
{
	vec4 PointsA[];
};
layout(std140, binding = 1) buffer PointsBufferB
{
	vec4 PointsB[];
};
layout(std140, binding = 2) buffer PointsBufferC
{
	vec4 PointsC[];
};

uniform int u_CellsA;
uniform int u_CellsB;
uniform int u_CellsC;
uniform float u_Tiling;
uniform float u_Persistence;
uniform vec4 u_ChannelMask;

int GetCellCount(int pointsIndex)
{
	switch (pointsIndex)
	{
	case 0: return u_CellsA;
	case 1: return u_CellsB;
	case 2: return u_CellsC;
	default: return -1;
	}
}

vec4 GetPoint(int pointsIndex, int pointIndex)
{
	switch (pointsIndex)
	{
	case 0: return PointsA[pointIndex];
	case 1: return PointsB[pointIndex];
	case 2: return PointsC[pointIndex];
	default: return vec4(-1.0);
	}
}

float CalculateWorley(int pointsIndex, vec3 samplePosition)
{
	int cellsPerAxis = GetCellCount(pointsIndex);
	vec3 cellID = floor(samplePosition);
	vec3 gridID = fract(samplePosition);

	float minDistance = 1.0;

	for (int z = -1, zIndex = 0; z <= 1; z++, zIndex++)
	{
		for (int y = -1, yIndex = 0; y <= 1; y++, yIndex++)
		{
			for (int x = -1, xIndex = 0; x <= 1; x++, xIndex++)
			{
				vec3 neighbor = vec3(x, y, z);
				int index = xIndex + cellsPerAxis * (yIndex + cellsPerAxis * zIndex);
				vec3 p = GetPoint(pointsIndex, index).xyz;

				vec3 offset = neighbor + p - gridID;

				minDistance = min(minDistance, dot(offset, offset));
			}
		}
	}

	return sqrt(minDistance);
}

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;
void main()
{
	ivec3 id = ivec3(gl_GlobalInvocationID);
	vec3 size = imageSize(o_Image);
	vec3 samplePosition = vec3(id) / size * u_Tiling;

	float worleyA = CalculateWorley(0, samplePosition);
	float worleyB = CalculateWorley(1, samplePosition);
	float worleyC = CalculateWorley(2, samplePosition);

	float noise = worleyA + (worleyB * u_Persistence) + (worleyC * u_Persistence * u_Persistence);
	float maxNoise = 1.0 + (u_Persistence) + (u_Persistence * u_Persistence);

	noise /= maxNoise;

	vec4 current = imageLoad(o_Image, ivec3(gl_GlobalInvocationID));
	vec4 result = current * (1.0 - u_ChannelMask) + noise * u_ChannelMask;
	imageStore(o_Image, id, result);
}