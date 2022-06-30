#type compute
#version 450 core

#define MIN_MAX_ACCURACY 10000000

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
layout(std430, binding = 3) buffer MinMaxBuffer
{
	int MinMax[2];
};


uniform bool u_IsBaseShape;
uniform bool u_Invert;
uniform sampler2D u_PerlinTexture;
uniform vec4 u_ChannelMask;
uniform float u_Persistence;
uniform float u_Tiling;
uniform float u_PerlinWorleyMix;
uniform float u_InversionWeight;

uniform int u_CellsA;
uniform int u_CellsB;
uniform int u_CellsC;

vec3 CellOffsets[] =
{
	// centre
	vec3(0,0,0),
	// front face
	vec3(0,0,1),
	vec3(-1,1,1),
	vec3(-1,0,1),
	vec3(-1,-1,1),
	vec3(0,1,1),
	vec3(0,-1,1),
	vec3(1,1,1),
	vec3(1,0,1),
	vec3(1,-1,1),
	// back face
	vec3(0,0,-1),
	vec3(-1,1,-1),
	vec3(-1,0,-1),
	vec3(-1,-1,-1),
	vec3(0,1,-1),
	vec3(0,-1,-1),
	vec3(1,1,-1),
	vec3(1,0,-1),
	vec3(1,-1,-1),
	// ring around centre
	vec3(-1,1,0),
	vec3(-1,0,0),
	vec3(-1,-1,0),
	vec3(0,1,0),
	vec3(0,-1,0),
	vec3(1,1,0),
	vec3(1,0,0),
	vec3(1,-1,0)
};


int MinComponent(ivec3 v)
{
	return min(v.x, min(v.y, v.z));
}

int MaxComponent(ivec3 v)
{
	return max(v.x, max(v.y, v.z));
}

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

float CalculateNoise(int pointsIndex, vec3 samplePosition)
{
	int cells = GetCellCount(pointsIndex);
	vec3 cellID = floor(samplePosition * cells);
	float shortestDistance = 1.0;

	for (int i = 0; i < 27; i++)
	{
		ivec3 adjacentCellID = ivec3(cellID + CellOffsets[i]);

		if (MinComponent(adjacentCellID) == -1 || MaxComponent(adjacentCellID) == cells)
		{
			ivec3 wrappedID = ivec3(mod(adjacentCellID + ivec3(cells), ivec3(cells)));
			int adjacentIndex = wrappedID.x + cells * (wrappedID.y + wrappedID.z * cells);
			vec3 p = GetPoint(pointsIndex, adjacentIndex).xyz;

			for (int j = 0; j < 27; j++)
			{
				vec3 offset = samplePosition - (p + CellOffsets[j]);
				shortestDistance = min(shortestDistance, dot(offset, offset));
			}
		}
		else
		{
			int adjacentIndex = adjacentCellID.x + cells * (adjacentCellID.y + adjacentCellID.z * cells);
			vec3 p = GetPoint(pointsIndex, adjacentIndex).xyz;
			vec3 offset = samplePosition - p;

			shortestDistance = min(shortestDistance, dot(offset, offset));
		}
	}

	return sqrt(shortestDistance);
}

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;
void main()
{
	vec3 imgSize = imageSize(o_Image);
	ivec3 invocID = ivec3(gl_GlobalInvocationID);

	vec3 texCoord = vec3(invocID) / imgSize;
	vec3 samplePosition = mod(texCoord * u_Tiling, vec3(1.0));

	float layerA = CalculateNoise(0, samplePosition);
	float layerB = CalculateNoise(1, samplePosition);
	float layerC = CalculateNoise(2, samplePosition);
	
	float noise = layerA + (layerB * u_Persistence) + (layerC * u_Persistence * u_Persistence);
	float maxNoise = 1.0 + (u_Persistence) + (u_Persistence * u_Persistence);

	noise /= maxNoise;
	if(u_Invert)
		noise = (1.0 - noise) * u_InversionWeight;

	int minMaxValue = int(noise * MIN_MAX_ACCURACY);
	MinMax[0] = min(MinMax[0], minMaxValue);
	MinMax[1] = max(MinMax[1], minMaxValue);
	barrier();

	if (u_ChannelMask.r == 1.0 && u_IsBaseShape)
		noise = mix(noise, texture(u_PerlinTexture, texCoord.xy).r, u_PerlinWorleyMix);

	vec4 current = imageLoad(o_Image, ivec3(gl_GlobalInvocationID));
	vec4 result = current * (1.0 - u_ChannelMask) + noise * u_ChannelMask;

	imageStore(o_Image, ivec3(gl_GlobalInvocationID), result);
}