﻿#type compute
#version 450

const float PI = 3.141592;
const float TwoPI = 2 * PI;
const float Epsilon = 0.00001;

layout(binding = 0, rgba32f) restrict writeonly uniform imageCube o_IrradianceMap;
uniform samplerCube sampler_RadianceMap;
uniform int EnvironmentSamples;

float RadicalInverse_VdC(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

// Sample i-th point from Hammersley point set of NumSamples points total.
vec2 SampleHammersley(uint i, uint samples)
{
	float invSamples = 1.0 / float(samples);
	return vec2(i * invSamples, RadicalInverse_VdC(i));
}

vec3 GetCubeMapTexCoord(vec2 imageSize)
{
    vec2 st = gl_GlobalInvocationID.xy / imageSize;
    vec2 uv = 2.0 * vec2(st.x, 1.0 - st.y) - vec2(1.0);

    vec3 ret;
    if      (gl_GlobalInvocationID.z == 0) ret = vec3(  1.0, uv.y, -uv.x);
    else if (gl_GlobalInvocationID.z == 1) ret = vec3( -1.0, uv.y,  uv.x);
    else if (gl_GlobalInvocationID.z == 2) ret = vec3( uv.x,  1.0, -uv.y);
    else if (gl_GlobalInvocationID.z == 3) ret = vec3( uv.x, -1.0,  uv.y);
    else if (gl_GlobalInvocationID.z == 4) ret = vec3( uv.x, uv.y,   1.0);
    else if (gl_GlobalInvocationID.z == 5) ret = vec3(-uv.x, uv.y,  -1.0);
    return normalize(ret);
}

// Compute orthonormal basis for converting from tanget/shading space to world space.
void ComputeBasisVectors(const vec3 N, out vec3 S, out vec3 T)
{
	// Branchless select non-degenerate T.
	T = cross(N, vec3(0.0, 1.0, 0.0));
	T = mix(cross(N, vec3(1.0, 0.0, 0.0)), T, step(Epsilon, dot(T, T)));

	T = normalize(T);
	S = normalize(cross(N, T));
}

// Convert point from tangent/shading space to world space.
vec3 TangentToWorld(const vec3 v, const vec3 N, const vec3 S, const vec3 T)
{
	return S * v.x + T * v.y + N * v.z;
}

// Uniformly sample point on a hemisphere.
// Cosine-weighted sampling would be a better fit for Lambertian BRDF but since this
// compute shader runs only once as a pre-processing step performance is not *that* important.
// See: "Physically Based Rendering" 2nd ed., section 13.6.1.
vec3 SampleHemisphere(float u1, float u2)
{
	const float u1p = sqrt(max(0.0, 1.0 - u1*u1));
	return vec3(cos(TwoPI*u2) * u1p, sin(TwoPI*u2) * u1p, u1);
}

layout(local_size_x=32, local_size_y=32, local_size_z=1) in;
void main()
{
    vec3 N = GetCubeMapTexCoord(vec2(imageSize(o_IrradianceMap)));
    
    vec3 S, T;
	ComputeBasisVectors(N, S, T);
	
    int Samples = 64 * EnvironmentSamples;
    
    // Monte Carlo integration of hemispherical irradiance.
	// As a small optimization this also includes Lambertian BRDF assuming perfectly white surface (albedo of 1.0)
	// so we don't need to normalize in PBR fragment shader (so technically it encodes exitant radiance rather than irradiance).
    vec3 Irradiance = vec3(0.0);
    for(int i = 0; i < Samples; i++)
    {
        vec2 U = SampleHammersley(i, Samples);
        vec3 Li = TangentToWorld(SampleHemisphere(U.x, U.y), N, S, T);
        float CosTheta = max(0.0, dot(Li, N));
        
        Irradiance += 2.0 * textureLod(sampler_RadianceMap, Li, 0).rgb * CosTheta;
    }
    
    Irradiance /= vec3(Samples);
    imageStore(o_IrradianceMap, ivec3(gl_GlobalInvocationID), vec4(Irradiance, 1.0));
}
