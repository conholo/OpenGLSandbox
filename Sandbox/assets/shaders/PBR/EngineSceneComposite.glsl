#type vertex
#version 450

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Binormal;
layout(location = 4) in vec2 a_TexCoord;

out vec2 v_TexCoord;
void main()
{
	v_TexCoord = a_TexCoord;
	gl_Position = vec4(a_Position.xy, 0.0, 1.0);
}

#type fragment
#version 450

layout(location = 0) out vec4 o_Color;

uniform int u_ApplyTransmittance;

uniform sampler2D u_SceneRadianceMap;
uniform sampler2D u_TransmittanceMap;
uniform sampler2D u_SkyRadianceMap;
uniform sampler2D u_SkyRadianceGeometryMap;
uniform sampler2D u_SunRadianceMap;
uniform float u_Exposure;
in vec2 v_TexCoord;

// Based on https://64.github.io/tonemapping/
vec3 ACESTonemap(vec3 color)
{
	mat3 m1 = mat3(
		0.59719, 0.07600, 0.02840,
		0.35458, 0.90834, 0.13383,
		0.04823, 0.01566, 0.83777
	);
	mat3 m2 = mat3(
		1.60475, -0.10208, -0.00327,
		-0.53108, 1.10813, -0.07276,
		-0.07367, -0.00605, 1.07602
	);
	vec3 v = m1 * color;
	vec3 a = v * (v + 0.0245786) - 0.000090537;
	vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
	return clamp(m2 * (a / b), 0.0, 1.0);
}

vec3 GammaCorrect(vec3 color, float gamma)
{
	return pow(color, vec3(1.0f / gamma));
}

void main()
{
    vec3 SceneGeometry = texture(u_SceneRadianceMap, v_TexCoord).rgb;
    vec3 SkyRadiance = texture(u_SkyRadianceMap, v_TexCoord).rgb;
    vec3 SkyRadianceGeometry = texture(u_SkyRadianceGeometryMap, v_TexCoord).rgb;
    vec3 Transmittance = u_ApplyTransmittance == 1 ? texture(u_TransmittanceMap, v_TexCoord).rgb : vec3(1.0);
    vec3 SunRadiance = texture(u_SunRadianceMap, v_TexCoord).rgb;
    
    vec3 SunSkyRadiance = SunRadiance + SkyRadiance;
    vec3 InScatteredRadiance = SunSkyRadiance - SkyRadianceGeometry * Transmittance;
    vec3 Result = SceneGeometry * Transmittance + InScatteredRadiance;
	o_Color = vec4(Result, 1.0); 
}
