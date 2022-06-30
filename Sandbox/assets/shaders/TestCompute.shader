#type compute
#version 450

layout(local_size_x = 1, local_size_y = 1) in;
layout(rgba32f, binding = 0) uniform image2D img_output;

uniform float u_Lacunarity;
uniform float u_Persistence;

// 2D Random
float random(in vec2 st)
{
    return fract(sin(dot(st.xy,
        vec2(12.9898, 78.233)))
        * 43758.5453123);
}

// 2D Noise based on Morgan McGuire @morgan3d
// https://www.shadertoy.com/view/4dS3Wd
float noise(in vec2 st)
{
    vec2 i = floor(st);
    vec2 f = fract(st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    // Smooth Interpolation

    // Cubic Hermine Curve.  Same as SmoothStep()
    vec2 u = f * f * (3.0 - 2.0 * f);
    // u = smoothstep(0.,1.,f);

    // Mix 4 coorners percentages
    return mix(a, b, u.x) +
        (c - a) * u.y * (1.0 - u.x) +
        (d - b) * u.x * u.y;
}

// x: lacunarity, y: persistence, z: octaves, w: scale
uniform vec4 u_Params;
uniform vec2 u_Offset;
uniform vec4 u_NoiseColor;
uniform float u_Time;

mat2 rotate2d(float _angle)
{
    return mat2(cos(_angle), -sin(_angle),
        sin(_angle), cos(_angle));
}


vec2 tile(vec2 _st, float _zoom)
{
    _st *= _zoom;
    return fract(_st);
}


void main()
{
	ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
	ivec2 dims = imageSize(img_output);

    float amplitude = 1.0;
    float frequency = 1.0;

    float noiseHeight = 0;

    float halfWidth = float(dims.x) / 2;
    float halfHeight = float(dims.y) / 2;

    for (int i = 0; i < u_Params.z; i++)
    {
        float sampleX = (pixelCoords.x - halfWidth) / u_Params.w * frequency + u_Offset.x;
        float sampleY = (pixelCoords.y - halfHeight) / u_Params.w * frequency + u_Offset.y;

        vec2 samplePosition = vec2(sampleX, sampleY);

        samplePosition = rotate2d(noise(samplePosition + u_Offset) * 2 - 1 * u_Time * 0.1) * samplePosition;
        samplePosition = tile(samplePosition, u_Params.x);

        float perlinValue = smoothstep(0.15, .20, noise(samplePosition) * 2 - 1);
        perlinValue -= smoothstep(0.35, .4, noise(samplePosition) * 2 - 1);

        noiseHeight += perlinValue * amplitude;

        frequency *= u_Params.x;
        amplitude *= u_Params.y;
    }

	vec4 pixel = vec4(1 - noiseHeight) * u_NoiseColor;

	imageStore(img_output, pixelCoords, pixel);
}