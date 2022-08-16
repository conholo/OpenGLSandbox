#type compute
#version 450 core

layout(binding = 1, rgba32f) restrict writeonly uniform image2D o_Image;


layout(std140, binding = 3) buffer RandomOffsets
{
	vec4 Offsets[];
};

struct PerlinSettings
{
	int Octaves;
	float NoiseScale;
	float Lacunarity;
	float Persistence;
	vec2 TextureOffset;
};

uniform PerlinSettings u_Settings;

vec2 hash(vec2 p)
{
	p = vec2(dot(p, vec2(189.0, 75.0)), dot(p, vec2(122.0, 220.0)));
	return fract(sin(p) * 4328395.432885) * 2.0 - 1.0;
}

vec2 grad(ivec2 z)  // replace this anything that returns a random vector
{
	// 2D to 1D  (feel free to replace by some other)
	int n = z.x + z.y * 11111;

	// Hugo Elias hash (feel free to replace by another one)
	n = (n << 13) ^ n;
	n = (n * (n * n * 15731 + 789221) + 1376312589) >> 16;

#if 0

	// simple random vectors
	return vec2(cos(float(n)), sin(float(n)));

#else

	// Perlin style vectors
	n &= 7;
	vec2 gr = vec2(n & 1, n >> 1) * 2.0 - 1.0;
	return (n >= 6) ? vec2(0.0, gr.x) :
		(n >= 4) ? vec2(gr.x, 0.0) :
		gr;
#endif                              
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

float PerlinNoise2D(vec2 coord)
{
	float x0 = floor(coord.x);	// Left of unit square.
	float x1 = x0 + 1.0;		// Right of unit square.

	float y0 = floor(coord.y);	// Bottom of unit square.
	float y1 = y0 + 1.0;		// Top of unit square.

	vec2 bottomLeft = vec2(x0, y0);
	vec2 topRight = vec2(x1, y1);

	// Pseudo Random vectors for each corner
	vec2 prBottomLeft = hash(bottomLeft);
	vec2 prTopLeft = hash(vec2(bottomLeft.x, topRight.y));
	vec2 prTopRight = hash(topRight);
	vec2 prBottomRight = hash(vec2(topRight.x, bottomLeft.y));

	// Distance vectors from percentage points to corner points.
	vec2 offsetBottomLeft = coord - bottomLeft;
	vec2 offsetTopLeft = coord - vec2(bottomLeft.x, topRight.y);
	vec2 offsetTopRight = coord - topRight;
	vec2 offsetBottomRight = coord - vec2(topRight.x, bottomLeft.y);

	// Dot product between offset corner vectors and random vectors.
	float d1 = dot(prBottomLeft, offsetBottomLeft);
	float d2 = dot(prBottomRight, offsetBottomRight);
	float d3 = dot(prTopLeft, offsetTopLeft);
	float d4 = dot(prTopRight, offsetTopRight);

	coord = mod(coord, 1.0);
	// Smoothing
	coord = coord * coord * (3.0 - 2.0 * coord);

	float x2 = mix(d1, d2, coord.x);
	float x3 = mix(d3, d4, coord.x);

	float e = mix(x2, x3, coord.y) * 0.5 + 0.5;

	return e;
}


layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
void main()
{
	vec2 size = imageSize(o_Image);
	vec2 texCoord = ivec2(gl_GlobalInvocationID.x - size.x / 2.0, gl_GlobalInvocationID.y - size.y / 2.0) / size * u_Settings.NoiseScale;

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

	imageStore(o_Image, ivec2(gl_GlobalInvocationID.xy), vec4(vec3(value), 1.0));
}