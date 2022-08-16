#type compute
#version 460

layout(binding = 0, rgba32f) restrict writeonly uniform image2D o_Image;

// Hash functions by Dave_Hoskins
float hash12(vec2 p)
{
    uvec2 q = uvec2(ivec2(p)) * uvec2(1597334673U, 3812015801U);
    uint n = (q.x ^ q.y) * 1597334673U;
    return float(n) * (1.0 / float(0xffffffffU));
}

vec2 hash22(vec2 p)
{
    uvec2 q = uvec2(ivec2(p)) * uvec2(1597334673U, 3812015801U);
    q = (q.x ^ q.y) * uvec2(1597334673U, 3812015801U);
    return vec2(q) * (1.0 / float(0xffffffffU));
}

float remap(float x, float a, float b, float c, float d)
{
    return (((x - a) / (b - a)) * (d - c)) + c;
}

// Noise function by morgan3d
float perlinNoise(vec2 x)
{
    vec2 i = floor(x);
    vec2 f = fract(x);

    float a = hash12(i);
    float b = hash12(i + vec2(1.0, 0.0));
    float c = hash12(i + vec2(0.0, 1.0));
    float d = hash12(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

vec2 curlNoise(vec2 uv)
{
    vec2 eps = vec2(0., 1.);

    float n1, n2, a, b;
    n1 = perlinNoise(uv + eps);
    n2 = perlinNoise(uv - eps);
    a = (n1 - n2) / (2. * eps.y);

    n1 = perlinNoise(uv + eps.yx);
    n2 = perlinNoise(uv - eps.yx);
    b = (n1 - n2) / (2. * eps.y);

    return vec2(a, -b);
}

uniform float u_Strength;
uniform float u_Tiling;
uniform float u_Persistence;
uniform vec2 u_TilingOffset;
uniform vec2 u_Weights;

layout(local_size_x = 8, local_size_y = 8) in;
void main()
{
	vec2 size = imageSize(o_Image);
	vec2 texCoord = gl_GlobalInvocationID.xy / size;
	texCoord *= 2.0 - 1.0;
    texCoord *= u_Tiling;
    
	vec2 curlFieldA = curlNoise(texCoord + u_TilingOffset);
    float fieldIntensity = 0.5 + pow(length(curlFieldA), 0.2) * u_Strength;

	imageStore(o_Image, ivec2(gl_GlobalInvocationID.xy), vec4(vec3(fieldIntensity), 1.0));
}