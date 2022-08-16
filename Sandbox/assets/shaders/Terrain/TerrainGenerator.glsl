#type compute
#version 460 core

const float Epsilon = 10e-4;

struct Vertex
{
	vec4 Position_UV_x;
	vec4 Normal_UV_y;
};

layout(std140, binding = 0) restrict coherent buffer TerrainVertices
{
	Vertex Vertices[];
};

layout(std430, binding = 1) restrict coherent buffer TerrainIndices
{
	uint Indices[];
};

layout(std140, binding = 2) buffer RandomOffsets
{
	vec4 Offsets[];
};

layout(std430, binding = 3) restrict buffer MinMaxBuffer
{
	float MinMax[2];
};

struct PerlinSettings
{
	int Octaves;
	float NoiseScale;
	float Lacunarity;
	float Persistence;
	vec2 TextureOffset;
};

float Remap(float v, float minOld, float maxOld, float minNew, float maxNew)
{
	return minNew + (v - minOld) * (maxNew - minNew) / (maxOld - minOld);
}

uniform PerlinSettings u_Settings;

vec2 grad(ivec2 z) 
{
	// 2D to 1D  (feel free to replace by some other)
	int n = z.x + z.y * 11111;

	// Hugo Elias hash (feel free to replace by another one)
	n = (n << 13) ^ n;
	n = (n * (n * n * 15731 + 789221) + 1376312589) >> 16;

	// Perlin style vectors
	n &= 7;
	vec2 gr = vec2(n & 1, n >> 1) * 2.0 - 1.0;
	return 
		(n >= 6) ? vec2(0.0, gr.x) :
		(n >= 4) ? vec2(gr.x, 0.0) : gr;
}

float noise(in vec2 p)
{
	ivec2 i = ivec2(floor(p));
	vec2 f = fract(p);

	vec2 u = f * f * (3.0 - 2.0 * f); // feel free to replace by a quintic smoothstep instead

	return mix(
		mix(
			dot(grad(i + ivec2(0, 0)), f - vec2(0.0, 0.0)),
			dot(grad(i + ivec2(1, 0)), f - vec2(1.0, 0.0)),
			u.x
		),
		mix(
			dot(grad(i + ivec2(0, 1)), f - vec2(0.0, 1.0)),
			dot(grad(i + ivec2(1, 1)), f - vec2(1.0, 1.0)),
			u.x
		),
		u.y);
}

float SimplexNoise(vec2 texCoord)
{
	float amplitude = 1.0;
	float frequency = u_Settings.NoiseScale;
	float value = 0.0;

	for (int i = 0; i < u_Settings.Octaves; i++)
	{
		float noise = noise((texCoord + u_Settings.TextureOffset / 100.0) * frequency + Offsets[i].xy) * 0.5 + 0.5;
		value += noise * amplitude;
		amplitude *= u_Settings.Persistence;
		frequency *= u_Settings.Lacunarity;
	}

	value /= u_Settings.Octaves;

	return value;
}


uniform float u_HeightThreshold;
uniform float u_HeightScaleFactor;
uniform vec2 u_BoundsMin;
uniform vec2 u_BoundsMax;
uniform vec2 u_Resolution;

vec3 CalculatePosition(vec2 texCoord)
{
	vec2 position = mix(u_BoundsMin, u_BoundsMax, texCoord);

	float inverseDistance = 1.0 - length(texCoord * 2.0 - 1.0);
	float distanceStrength = smoothstep(u_HeightThreshold - 0.1, u_HeightThreshold + 0.1, inverseDistance);
	float fallOffStrength = sqrt(inverseDistance) * distanceStrength;

	float heightDistanceFactor = smoothstep(u_HeightThreshold - 0.4, u_HeightThreshold + 0.4, inverseDistance) * u_HeightScaleFactor;

	float heightValue = SimplexNoise(texCoord) * heightDistanceFactor * fallOffStrength;

	return vec3(position.x, heightValue, position.y);
}

vec3 CalculateNeighborPosition(ivec2 id, ivec2 maxSize)
{
	if (id.x > maxSize.x || id.y > maxSize.y || id.x < 0 || id.y < 0) 
		return vec3(0.0001);

	vec2 texCoord = vec2(id) / vec2(maxSize);

	return CalculatePosition(texCoord);
}

vec3 NormalizeCheck(vec3 n)
{
	float len = length(n);
	vec3 normal = len == 0.0 || isnan(len) ? vec3(1.0, 1.0, 1.0) : n / len;
	return normal;
}

vec3 CalculateNormals(ivec2 id, ivec2 maxSize)
{
	ivec2 up = id + ivec2(0, 1);
	ivec2 upRight = id + ivec2(1, 1);
	ivec2 right = id + ivec2(1, 0);
	ivec2 down = id + ivec2(0, -1);
	ivec2 downLeft = id + ivec2(-1, -1);
	ivec2 left = id + ivec2(-1, 0);

	vec3 upNeighbor = CalculateNeighborPosition(up, maxSize);
	vec3 upRightNeighbor = CalculateNeighborPosition(upRight, maxSize);
	vec3 rightNeighbor = CalculateNeighborPosition(right, maxSize);
	vec3 downNeighbor = CalculateNeighborPosition(down, maxSize);
	vec3 downLeftNeighbor = CalculateNeighborPosition(downLeft, maxSize);
	vec3 leftNeighbor = CalculateNeighborPosition(left, maxSize);

	vec3 n1 = cross(leftNeighbor, upNeighbor);
	vec3 n2 = cross(upNeighbor, upRightNeighbor);
	vec3 n3 = cross(upRightNeighbor, rightNeighbor);
	vec3 n4 = cross(rightNeighbor, downNeighbor);
	vec3 n5 = cross(downNeighbor, downLeftNeighbor);
	vec3 n6 = cross(downLeftNeighbor, leftNeighbor);

	vec3 n = n1 + n2 + n3 + n4 + n5 + n6;
	return NormalizeCheck(n);
}

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
void main()
{
	vec2 size = u_Resolution;
	vec2 imageSizeMinusOne = size - 1.0;
	ivec2 id = ivec2(gl_GlobalInvocationID.xy);
	int vertexIndex = id.y + id.x * int(size.x);
	vec2 texCoord = vec2(id) / imageSizeMinusOne;

	Vertices[vertexIndex].Position_UV_x.xyz = CalculatePosition(texCoord);

	MinMax[0] = min(MinMax[0], Vertices[vertexIndex].Position_UV_x.y);
	MinMax[1] = max(MinMax[1], Vertices[vertexIndex].Position_UV_x.y);

	Vertices[vertexIndex].Position_UV_x.w = texCoord.x;
	Vertices[vertexIndex].Normal_UV_y.x = texCoord.y;
	Vertices[vertexIndex].Normal_UV_y.yzw = CalculateNormals(id, ivec2(imageSizeMinusOne));

	if (id.x < size.x - 1 && id.y < size.y - 1)
	{
		// Index * 6 gets us the start of the quad.  Subtracting by the row * 6 accounts for the number of vertices we've skipped on the extents.
		int startTriangleIndex = vertexIndex * 6 - id.x * 6;

		Indices[startTriangleIndex] = vertexIndex;
		Indices[startTriangleIndex + 1] = vertexIndex + int(size.x) + 1;
		Indices[startTriangleIndex + 2] = vertexIndex + int(size.x);

		Indices[startTriangleIndex + 3] = vertexIndex + int(size.x) + 1;
		Indices[startTriangleIndex + 4] = vertexIndex;
		Indices[startTriangleIndex + 5] = vertexIndex + 1;
	}
}
